#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#ifndef REDIRECT_HEADER
#include <linux/ip.h>
#endif

#ifndef __BPF__
#include <linux/types.h>

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

struct pseudo_hdr
{
    unsigned long saddr;    // 4 bytes
    unsigned long daddr;    // 4 bytes
    unsigned char reserved; // 1 byte
    unsigned char proto;    // 1 byte
    unsigned short len;     // 2 bytes
};

struct ipv4_ip{
    union {
        unsigned int addr32;
        unsigned char addr8[4];
    };
};

struct ipv4_segment{
    struct ipv4_ip sip, eip;
    char *str;
};

/*
 * Checksums for x86-64
 * Copyright 2002 by Andi Kleen, SuSE Labs
 * with some code from asm-x86/checksum.h
 */

static inline unsigned add32_with_carry(unsigned a, unsigned b)
{
    asm("addl %2,%0\n\t"
        "adcl $0,%0"
        : "=r"(a)
        : "0"(a), "rm"(b));
    return a;
}

static inline unsigned short from32to16(unsigned a)
{
    unsigned short b = a >> 16;
    asm("addw %w2,%w0\n\t"
        "adcw $0,%w0\n"
        : "=r"(b)
        : "0"(b), "r"(a));
    return b;
}

/*
 * Do a 64-bit checksum on an arbitrary memory area.
 * Returns a 32bit checksum.
 *
 * This isn't as time critical as it used to be because many NICs
 * do hardware checksumming these days.
 *
 * Things tried and found to not make it faster:
 * Manual Prefetching
 * Unrolling to an 128 bytes inner loop.
 * Using interleaving with more registers to break the carry chains.
 */
static unsigned do_csum(const unsigned char *buff, unsigned len)
{
    unsigned odd, count;
    unsigned long result = 0;

    if (unlikely(len == 0))
        return result;
    odd = 1 & (unsigned long)buff;
    if (unlikely(odd))
    {
        result = *buff << 8;
        len--;
        buff++;
    }
    count = len >> 1; /* nr of 16-bit words.. */
    if (count)
    {
        if (2 & (unsigned long)buff)
        {
            result += *(unsigned short *)buff;
            count--;
            len -= 2;
            buff += 2;
        }
        count >>= 1; /* nr of 32-bit words.. */
        if (count)
        {
            unsigned long zero;
            unsigned count64;
            if (4 & (unsigned long)buff)
            {
                result += *(unsigned int *)buff;
                count--;
                len -= 4;
                buff += 4;
            }
            count >>= 1; /* nr of 64-bit words.. */

            /* main loop using 64byte blocks */
            zero = 0;
            count64 = count >> 3;
            while (count64)
            {
                asm("addq 0*8(%[src]),%[res]\n\t"
                    "adcq 1*8(%[src]),%[res]\n\t"
                    "adcq 2*8(%[src]),%[res]\n\t"
                    "adcq 3*8(%[src]),%[res]\n\t"
                    "adcq 4*8(%[src]),%[res]\n\t"
                    "adcq 5*8(%[src]),%[res]\n\t"
                    "adcq 6*8(%[src]),%[res]\n\t"
                    "adcq 7*8(%[src]),%[res]\n\t"
                    "adcq %[zero],%[res]"
                    : [res] "=r"(result)
                    : [src] "r"(buff), [zero] "r"(zero),
                      "[res]"(result));
                buff += 64;
                count64--;
            }

            /* last up to 7 8byte blocks */
            count %= 8;
            while (count)
            {
                asm("addq %1,%0\n\t"
                    "adcq %2,%0\n"
                    : "=r"(result)
                    : "m"(*(unsigned long *)buff),
                      "r"(zero), "0"(result));
                --count;
                buff += 8;
            }
            result = add32_with_carry(result >> 32,
                                      result & 0xffffffff);

            if (len & 4)
            {
                result += *(unsigned int *)buff;
                buff += 4;
            }
        }
        if (len & 2)
        {
            result += *(unsigned short *)buff;
            buff += 2;
        }
    }
    if (len & 1)
        result += *buff;
    result = add32_with_carry(result >> 32, result & 0xffffffff);
    if (unlikely(odd))
    {
        result = from32to16(result);
        result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
    }
    return result;
}

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 64-bit boundary
 */
static inline __wsum csum_partial(const void *buff, int len, __wsum sum)
{
    return (__wsum)add32_with_carry(do_csum((const unsigned char *)buff, len),
                                    (__u32)sum);
}

/**
 * csum_fold - Fold and invert a 32bit checksum.
 * sum: 32bit unfolded sum
 *
 * Fold a 32bit running checksum to 16bit and invert it. This is usually
 * the last step before putting a checksum into a packet.
 * Make sure not to mix with 64bit checksums.
 */
static inline __sum16 csum_fold(__wsum sum)
{
    asm("  addl %1,%0\n"
        "  adcl $0xffff,%0"
        : "=r"(sum)
        : "r"((__u32)sum << 16),
          "0"((__u32)sum & 0xffff0000));
    return (__sum16)(~(__u32)sum >> 16);
}

/*
 *	This is a version of ip_compute_csum() optimized for IP headers,
 *	which always checksum on 4 octet boundaries.
 *
 *	By Jorge Cwik <jorge@laser.satlink.net>, adapted for linux by
 *	Arnt Gulbrandsen.
 */

/**
 * ip_fast_csum - Compute the IPv4 header checksum efficiently.
 * iph: ipv4 header
 * ihl: length of header / 4
 */
static inline __sum16 ip_fast_csum(const void *iph, unsigned int ihl)
{
    unsigned int sum;

    asm("  movl (%1), %0\n"
        "  subl $4, %2\n"
        "  jbe 2f\n"
        "  addl 4(%1), %0\n"
        "  adcl 8(%1), %0\n"
        "  adcl 12(%1), %0\n"
        "1: adcl 16(%1), %0\n"
        "  lea 4(%1), %1\n"
        "  decl %2\n"
        "  jne	1b\n"
        "  adcl $0, %0\n"
        "  movl %0, %2\n"
        "  shrl $16, %0\n"
        "  addw %w2, %w0\n"
        "  adcl $0, %0\n"
        "  notl %0\n"
        "2:"
        /* Since the input registers which are loaded with iph and ihl
           are modified, we must also specify them as outputs, or gcc
           will assume they contain their original values. */
        : "=r"(sum), "=r"(iph), "=r"(ihl)
        : "1"(iph), "2"(ihl)
        : "memory");
    return (__sum16)sum;
}

/**
 * csum_tcpup_nofold - Compute an IPv4 pseudo header checksum.
 * @saddr: source address
 * @daddr: destination address
 * @len: length of packet
 * @proto: ip protocol of packet
 * @sum: initial sum to be added in (32bit unfolded)
 *
 * Returns the pseudo header checksum the input data. Result is
 * 32bit unfolded.
 */
static inline __wsum
csum_tcpudp_nofold(__be32 saddr, __be32 daddr, __u32 len,
                   __u8 proto, __wsum sum)
{
    asm("  addl %1, %0\n"
        "  adcl %2, %0\n"
        "  adcl %3, %0\n"
        "  adcl $0, %0\n"
        : "=r"(sum)
        : "g"(daddr), "g"(saddr),
          "g"((len + proto) << 8), "0"(sum));
    return sum;
}

/**
 * csum_tcpup_magic - Compute an IPv4 pseudo header checksum.
 * @saddr: source address
 * @daddr: destination address
 * @len: length of packet
 * @proto: ip protocol of packet
 * @sum: initial sum to be added in (32bit unfolded)
 *
 * Returns the 16bit pseudo header checksum the input data already
 * complemented and ready to be filled in.
 */
static inline __sum16 csum_tcpudp_magic(__be32 saddr, __be32 daddr,
                                        __u32 len, __u8 proto,
                                        __wsum sum)
{
    return csum_fold(csum_tcpudp_nofold(saddr, daddr, len, proto, sum));
}

#endif

static __always_inline uint16_t csum_fold_helper(uint32_t csum)
{
    uint32_t r = csum << 16 | csum >> 16;
    csum = ~csum;
    csum -= r;
    return (uint16_t)(csum >> 16);
}

static __always_inline uint32_t csum_add(uint32_t addend, uint32_t csum)
{
    uint32_t res = csum;
    res += addend;
    return (res + (res < addend));
}

static __always_inline uint32_t csum_sub(uint32_t addend, uint32_t csum)
{
    return csum_add(csum, ~addend);
}

static __always_inline void update_iph_checksum(struct iphdr *iph)
{
#ifndef __BPF__
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
#else
    uint16_t *next_iph_u16 = (uint16_t *)iph;
    uint32_t csum = 0;
    iph->check = 0;
#pragma clang loop unroll(full)
    for (uint32_t i = 0; i < sizeof(*iph) >> 1; i++)
    {
        csum += *next_iph_u16++;
    }

    iph->check = ~((csum & 0xffff) + (csum >> 16));
#endif
}

static __always_inline uint16_t csum_diff4(uint32_t from, uint32_t to, uint16_t csum)
{
    uint32_t tmp = csum_sub(from, ~((uint32_t)csum));
    return csum_fold_helper(csum_add(to, tmp));
}

uint16_t tcp_checksum(const void *buff, size_t len, uint32_t *src_addr, uint32_t *dest_addr)
{
    const uint16_t *buf = buff;
    uint32_t sum;
    size_t length = len;

    // Calculate the sum                                            //
    sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    if (len & 1)
        // Add the padding if the packet lenght is odd          //
        sum += *((uint8_t *)buf);

    // Add the pseudo-header                                        //
    sum += *(src_addr++);
    sum += *src_addr;
    sum += *(dest_addr++);
    sum += *dest_addr;
    sum += htons(IPPROTO_TCP);
    sum += htons(length);

    // Add the carries                                              //
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    // Return the one's complement of sum                           //
    return ((uint16_t)(~sum));
}

uint16_t icmp_csum(uint16_t *addr, int len)
{
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;

    // Sum up 2-byte values until none or only one byte left.
    while (count > 1)
    {
        sum += *(addr++);
        count -= 2;
    }

    // Add left-over byte, if any.
    if (count > 0)
    {
        sum += *(uint8_t *)addr;
    }

    // Fold 32-bit sum into 16 bits; we lose information by doing this,
    // increasing the chances of a collision.
    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // Checksum is one's compliment of sum.
    answer = ~sum;

    return (answer);
}

#define MAX_PCKT_LENGTH 0xFFFF

// Global variables.
uint8_t cont = 1;
time_t startTime;
volatile uint64_t pcktCount = 0;
volatile uint64_t totalData = 0;

// Thread structure.
struct pthread_info
{
    char *interface;
    struct ipv4_segment sIP;
    struct ipv4_segment dIP;
    uint16_t port;
    uint16_t sport;
    uint64_t interval;
    uint16_t min;
    uint16_t max;
    uint64_t pcktCountMax;
    time_t seconds;
    uint8_t payload[MAX_PCKT_LENGTH];
    char *payloadStr;
    uint16_t payloadLength;
    int tcp;
    int icmp;
    int verbose;
    int internal;
    int nostats;
    int tcp_urg;
    int tcp_ack;
    int tcp_psh;
    int tcp_rst;
    int tcp_syn;
    int tcp_fin;
    int icmp_type;
    int icmp_code;
    int ipip;
    int help;
    char *ipipsrc;
    char *ipipdst;
    uint8_t sMAC[ETH_ALEN];
    uint8_t dMAC[ETH_ALEN];
    uint16_t threads;
    int nocsum;
    int nocsum4;
    uint8_t minTTL;
    uint8_t maxTTL;
    uint8_t tos;

    time_t startingTime;
    uint16_t id;
} g_info;

void convert_to_ip_range(struct ipv4_segment* ip_seg)
{
    char ipd[40], mask[40], *ip = ip_seg->str;
    unsigned int uint_ip, uint_mask;
    if (strstr(ip, "-") != NULL) {
        sscanf(ip, "%[^-]-%s", ipd, mask);
        ip_seg->sip.addr32 = inet_addr(ipd);
        ip_seg->eip.addr32 = inet_addr(mask);
    } else if (strstr(ip, "/") != NULL) {
        sscanf(ip, "%[^/]/%s", ipd, mask);
        uint_ip = inet_addr(ipd);
        if (strstr(mask, ".") != NULL) {
            uint_mask = inet_addr(mask);
        } else {
            uint_mask = 1;
            size_t i;
            for (i = atoi(mask); i > 1; i--) {
                uint_mask  =  (uint_mask << 1) + 1;
            }
            
        }

        ip_seg->sip.addr32 = uint_ip & uint_mask;
        ip_seg->eip.addr32 = uint_ip | (~uint_mask);
    } else {
        ip_seg->sip.addr32 = inet_addr(ip);
        ip_seg->eip.addr32 = ip_seg->sip.addr32;
    }

    return ;
}

void signalHndl(int tmp)
{
    cont = 0;
}

void GetGatewayMAC(uint8_t *MAC)
{
    char cmd[] = "ip neigh | grep \"$(ip -4 route list 0/0|cut -d' ' -f3) \"|cut -d' ' -f5|tr '[a-f]' '[A-F]'";

    FILE *fp = popen(cmd, "r");

    if (fp != NULL)
    {
        char line[18];

        if (fgets(line, sizeof(line), fp) != NULL)
        {
            sscanf(line, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &MAC[0], &MAC[1], &MAC[2], &MAC[3], &MAC[4], &MAC[5]);
        }

        pclose(fp);
    }
}

uint16_t randNum(uint16_t min, uint16_t max, unsigned int seed)
{
    seed = rand();
    return (rand_r(&seed) % (max - min + 1)) + min;
}

unsigned random_ip(struct ipv4_segment* ip_seg, unsigned int seed)
{
    struct ipv4_ip s_ip, e_ip, ip;
    unsigned int scope = 0;
    if (ip_seg == NULL) {
        return 0;
    }

    s_ip.addr32 = ntohl(ip_seg->sip.addr32);
    e_ip.addr32 = ntohl(ip_seg->eip.addr32);
    scope = e_ip.addr32 - s_ip.addr32;
    if (scope == 0) {
        ip.addr32 = 0;
    } else {
        size_t i;
        for (i = 0; i < 4; i++) {
            ip.addr8[i] = randNum(0, 0xff, seed);
        }

        ip.addr32 = ip.addr32 % scope;
    }

    return ntohl(s_ip.addr32 + ip.addr32);
}

void *threadHndl(void *data)
{
    // Pass info.
    struct pthread_info *info = (struct pthread_info *)data;
    srand(time(NULL) + getpid() + pthread_self());

    // Create sockaddr_ll struct.
    struct sockaddr_ll sin;

    // Fill out sockaddr_ll struct.
    sin.sll_family = PF_PACKET;
    sin.sll_ifindex = if_nametoindex(info->interface);
    sin.sll_protocol = htons(ETH_P_IP);
    sin.sll_halen = ETH_ALEN;

    // Initialize socket FD.
    int sockfd;

    // Attempt to create socket.
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("socket");

        pthread_exit(NULL);
    }

    if (info->sMAC[0] == 0 && info->sMAC[1] == 0 && info->sMAC[2] == 0 && info->sMAC[3] == 0 && info->sMAC[4] == 0 && info->sMAC[5] == 0)
    {
        // Receive the interface's MAC address (the source MAC).
        struct ifreq ifr;
        strcpy(ifr.ifr_name, info->interface);

        // Attempt to get MAC address.
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) != 0)
        {
            perror("ioctl");

            pthread_exit(NULL);
        }

        // Copy source MAC to necessary variables.
        memcpy(info->sMAC, ifr.ifr_addr.sa_data, ETH_ALEN);
    }

    memcpy(sin.sll_addr, info->sMAC, ETH_ALEN);

    // Attempt to bind socket.
    if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) != 0)
    {
        perror("bind");

        pthread_exit(NULL);
    }

    // Loop.
    while (1)
    {
        // Create rand_r() seed.
        unsigned int seed;

        uint16_t offset = 0;

        if (info->nostats)
        {
            seed = time(NULL) ^ getpid() ^ pthread_self();
        }
        else
        {
            seed = (unsigned int)(pcktCount + info->id);
        }

        // Get source port (random).
        uint16_t srcPort;

        // Check if source port is 0 (random).
        if (info->sport == 0)
        {
            srcPort = randNum(1024, 65535, seed);
        }
        else
        {
            srcPort = info->sport;
        }

        // Get destination port.
        uint16_t dstPort;

        // Check if port is 0 (random).
        if (info->port == 0)
        {
            dstPort = randNum(10, 65535, seed);
        }
        else
        {
            dstPort = info->port;
        }

        unsigned int IP;

        if (info->sIP.str == NULL)
        {
            // Spoof source IP as any IP address.
            struct ipv4_ip tmp;

            if (info->internal)
            {
                tmp.addr8[0] = randNum(10, 10, seed);
                tmp.addr8[1] = randNum(0, 254, seed + 1);
                tmp.addr8[2] = randNum(0, 254, seed + 2);
                tmp.addr8[3] = randNum(0, 254, seed + 3);
            }
            else
            {
                tmp.addr8[0] = randNum(1, 254, seed);
                tmp.addr8[1] = randNum(0, 254, seed + 1);
                tmp.addr8[2] = randNum(0, 254, seed + 2);
                tmp.addr8[3] = randNum(0, 254, seed + 3);
            }

            // sprintf(IP, "%d.%d.%d.%d", tmp[0], tmp[1], tmp[2], tmp[3]);
            IP = tmp.addr32;
        }
        else
        {
            IP = random_ip(&info->sIP, seed);
        }

        // Initialize packet buffer.
        char buffer[MAX_PCKT_LENGTH];

        // Create ethernet header.
        struct ethhdr *eth = (struct ethhdr *)(buffer);

        // Fill out ethernet header.
        eth->h_proto = htons(ETH_P_IP);
        memcpy(eth->h_source, info->sMAC, ETH_ALEN);
        memcpy(eth->h_dest, info->dMAC, ETH_ALEN);

        // Increase offset.
        offset += sizeof(struct ethhdr);

        // Create outer IP header if enabled.
        struct iphdr *oiph = NULL;

        if (info->ipip)
        {
            oiph = (struct iphdr *)(buffer + offset);

            // Fill out header.
            oiph->ihl = 5;
            oiph->version = 4;
            oiph->protocol = IPPROTO_IPIP;
            oiph->id = 0;
            oiph->frag_off = 0;
            oiph->saddr = inet_addr(info->ipipsrc);
            oiph->daddr = inet_addr(info->ipipdst);
            oiph->tos = info->tos;

            oiph->ttl = (uint8_t)randNum(info->minTTL, info->maxTTL, seed);

            // Increase offset.
            offset += sizeof(struct iphdr);
        }

        // Create IP header.
        struct iphdr *iph = (struct iphdr *)(buffer + offset);

        // Fill out IP header.
        iph->ihl = 5;
        iph->version = 4;

        // Check for TCP.
        if (info->tcp)
        {
            iph->protocol = IPPROTO_TCP;
        }
        else if (info->icmp)
        {
            iph->protocol = IPPROTO_ICMP;
        }
        else
        {
            iph->protocol = IPPROTO_UDP;
        }

        iph->id = 0;
        iph->frag_off = 0;
        iph->saddr = IP;
        iph->daddr = random_ip(&info->dIP, seed);
        iph->tos = info->tos;

        iph->ttl = (uint8_t)randNum(info->minTTL, info->maxTTL, seed);

        // Increase offset.
        offset += sizeof(struct iphdr);

        // Calculate payload length and payload.
        uint16_t dataLen;

        // Initialize payload.
        uint16_t l4header;

        switch (iph->protocol)
        {
        case IPPROTO_UDP:
            l4header = sizeof(struct udphdr);

            break;

        case IPPROTO_TCP:
            l4header = sizeof(struct tcphdr);

            break;

        case IPPROTO_ICMP:
            l4header = sizeof(struct icmphdr);

            break;
        }

        // Increase offset.
        offset += l4header;

        unsigned char *data = (unsigned char *)(buffer + offset);

        // Check for custom payload.
        if (info->payloadLength > 0)
        {
            dataLen = info->payloadLength;
            uint16_t i;
            for (i = 0; i < info->payloadLength; i++)
            {
                *data = info->payload[i];
                *data++;
            }
        }
        else
        {
            dataLen = randNum(info->min, info->max, seed);
            uint16_t i;
            // Fill out payload with random characters.
            for (i = 0; i < dataLen; i++)
            {
                *data = rand_r(&seed);
                *data++;
            }
        }

        // Decrease offset since we're going back to fill in L4 layer.
        offset -= l4header;

        // Check protocol.
        if (iph->protocol == IPPROTO_TCP)
        {
            // Create TCP header.
            struct tcphdr *tcph = (struct tcphdr *)(buffer + offset);

            // Fill out TCP header.
            tcph->doff = 5;
            tcph->source = htons(srcPort);
            tcph->dest = htons(dstPort);
            tcph->ack_seq = 0;
            tcph->seq = 0;

            // Check for each flag.
            if (info->tcp_urg)
            {
                tcph->urg = 1;
            }

            if (info->tcp_ack)
            {
                tcph->ack = 1;
            }

            if (info->tcp_psh)
            {
                tcph->psh = 1;
            }

            if (info->tcp_rst)
            {
                tcph->rst = 1;
            }

            if (info->tcp_syn)
            {
                tcph->syn = 1;
            }

            if (info->tcp_fin)
            {
                tcph->fin = 1;
            }

            // Calculate TCP header checksum.
            tcph->check = 0;

            if (!info->nocsum4)
            {
                tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct tcphdr) + dataLen, IPPROTO_TCP, csum_partial(tcph, sizeof(struct tcphdr) + dataLen, 0));
            }
        }
        else if (iph->protocol == IPPROTO_ICMP)
        {
            // Create ICMP header.
            struct icmphdr *icmph = (struct icmphdr *)(buffer + offset);

            // Fill out ICMP header.
            icmph->type = info->icmp_type;
            icmph->code = info->icmp_code;

            // Calculate ICMP header's checksum.
            icmph->checksum = 0;

            if (!info->nocsum4)
            {
                icmph->checksum = icmp_csum((uint16_t *)icmph, sizeof(struct icmphdr) + dataLen);
            }
        }
        else
        {
            // Create UDP header.
            struct udphdr *udph = (struct udphdr *)(buffer + offset);

            // Fill out UDP header.
            udph->source = htons(srcPort);
            udph->dest = htons(dstPort);
            udph->len = htons(sizeof(struct udphdr) + dataLen);

            // Calculate UDP header checksum.
            udph->check = 0;

            if (!info->nocsum4)
            {
                udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + dataLen, IPPROTO_UDP, csum_partial(udph, sizeof(struct udphdr) + dataLen, 0));
            }
        }

        uint16_t pcktlen = 0;

        // Calculate length and checksum of IP headers.
        pcktlen = sizeof(struct iphdr) + l4header + dataLen;

        iph->tot_len = htons(pcktlen);

        if (!info->nocsum)
        {
            update_iph_checksum(iph);
        }

        if (oiph != NULL)
        {
            // Add length of additional IP header (20 bytes).
            pcktlen += sizeof(struct iphdr);

            oiph->tot_len = htons(pcktlen);

            if (!info->nocsum)
            {
                update_iph_checksum(oiph);
            }
        }

        // Initialize variable that represents how much data we've sent.
        uint16_t sent;

        // Attempt to send data.
        if ((sent = sendto(sockfd, buffer, pcktlen + sizeof(struct ethhdr), 0, (struct sockaddr *)&sin, sizeof(sin))) < 0)
        {
            perror("send");

            continue;
        }

        // Add onto stats if enabled.
        if (!info->nostats)
        {
            __sync_add_and_fetch(&totalData, sent);
        }

        if (!info->nostats || info->pcktCountMax > 0)
        {
            // Check packet count.
            if (__sync_add_and_fetch(&pcktCount, 1) >= info->pcktCountMax && info->pcktCountMax > 0)
            {
                cont = 0;

                break;
            }
        }

        // Verbose mode.
        if (info->verbose)
        {
            fprintf(stdout, "Sent %d bytes to destination. (%" PRIu64 "/%" PRIu64 ")\n", sent, pcktCount, info->pcktCountMax);
        }

        // Check time elasped.
        if (info->seconds > 0)
        {
            time_t timeNow = time(NULL);

            if (timeNow >= (info->startingTime + info->seconds))
            {
                cont = 0;

                break;
            }
        }

        // Check if we should wait between packets.
        if (info->interval > 0)
        {
            usleep(info->interval);
        }
    }

    // Close socket.
    close(sockfd);

    // Free information.
    free(info);

    // Exit thread.
    pthread_exit(NULL);
}

// Command line options.
static struct option longoptions[] =
    {
        {"dev", required_argument, NULL, 'i'},
        {"src", required_argument, NULL, 's'},
        {"dst", required_argument, NULL, 'd'},
        {"port", required_argument, NULL, 'p'},
        {"sport", required_argument, NULL, 14},
        {"interval", required_argument, NULL, 1},
        {"threads", required_argument, NULL, 't'},
        {"min", required_argument, NULL, 2},
        {"max", required_argument, NULL, 3},
        {"count", required_argument, NULL, 'c'},
        {"time", required_argument, NULL, 6},
        {"smac", required_argument, NULL, 7},
        {"dmac", required_argument, NULL, 8},
        {"payload", required_argument, NULL, 10},
        {"verbose", no_argument, &g_info.verbose, 'v'},
        {"tcp", no_argument, &g_info.tcp, 4},
        {"icmp", no_argument, &g_info.icmp, 4},
        {"ipip", no_argument, &g_info.ipip, 4},
        {"internal", no_argument, &g_info.internal, 5},
        {"nostats", no_argument, &g_info.nostats, 9},
        {"urg", no_argument, &g_info.tcp_urg, 11},
        {"ack", no_argument, &g_info.tcp_ack, 11},
        {"psh", no_argument, &g_info.tcp_psh, 11},
        {"rst", no_argument, &g_info.tcp_rst, 11},
        {"syn", no_argument, &g_info.tcp_syn, 11},
        {"fin", no_argument, &g_info.tcp_fin, 11},
        {"icmptype", required_argument, NULL, 12},
        {"icmpcode", required_argument, NULL, 13},
        {"ipipsrc", required_argument, NULL, 15},
        {"ipipdst", required_argument, NULL, 16},
        {"nocsum", no_argument, &g_info.nocsum, 17},
        {"nocsum4", no_argument, &g_info.nocsum4, 18},
        {"minttl", required_argument, NULL, 19},
        {"maxttl", required_argument, NULL, 20},
        {"tos", required_argument, NULL, 21},
        {"help", no_argument, &g_info.help, 'h'},
        {NULL, 0, NULL, 0}};

void parse_command_line(int argc, char *argv[])
{
    int c = -1;

    // Parse command line.
    while (optind < argc)
    {
        if ((c = getopt_long(argc, argv, "i:d:t:vhs:p:c:", longoptions, NULL)) != -1)
        {
            switch (c)
            {
            case 'i':
                g_info.interface = optarg;

                break;

            case 's':
                g_info.sIP.str = optarg;
                convert_to_ip_range(&g_info.sIP);

                break;

            case 'd':
                g_info.dIP.str = optarg;
                convert_to_ip_range(&g_info.dIP);

                break;

            case 'p':
                g_info.port = atoi(optarg);

                break;

            case 14:
                g_info.sport = atoi(optarg);

                break;

            case 1:
                g_info.interval = strtoll(optarg, NULL, 10);

                break;

            case 't':
                g_info.threads = atoi(optarg);

                break;

            case 2:
                g_info.min = atoi(optarg);

                break;

            case 3:
                g_info.max = atoi(optarg);

                break;

            case 'c':
                g_info.pcktCountMax = strtoll(optarg, NULL, 10);

                break;

            case 6:
                g_info.seconds = strtoll(optarg, NULL, 10);

                break;

            case 7:
                sscanf(optarg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &g_info.sMAC[0], &g_info.sMAC[1], &g_info.sMAC[2], &g_info.sMAC[3], &g_info.sMAC[4], &g_info.sMAC[5]);

                break;

            case 8:
                sscanf(optarg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &g_info.dMAC[0], &g_info.dMAC[1], &g_info.dMAC[2], &g_info.dMAC[3], &g_info.dMAC[4], &g_info.dMAC[5]);

                break;

            case 10:
                g_info.payloadStr = optarg;

                break;

            case 12:
                g_info.icmp_type = atoi(optarg);

                break;

            case 13:
                g_info.icmp_code = atoi(optarg);

                break;

            case 15:
                g_info.ipipsrc = optarg;

                break;

            case 16:
                g_info.ipipdst = optarg;

                break;

            case 17:
                g_info.nocsum = 1;

                break;

            case 18:
                g_info.nocsum4 = 1;

                break;

            case 19:
                g_info.minTTL = (uint8_t)atoi(optarg);

                break;

            case 20:
                g_info.maxTTL = (uint8_t)atoi(optarg);

                break;

            case 21:
                g_info.tos = (uint8_t)atoi(optarg);

                break;

            case 'v':
                g_info.verbose = 1;

                break;

            case 'h':
                g_info.help = 1;

                break;

            case '?':
                fprintf(stderr, "Missing argument.\n");

                break;
            }
        }
        else
        {
            optind++;
        }
    }
}

int main(int argc, char *argv[])
{
    // Set defaults.
    g_info.threads = get_nprocs();
    memset(g_info.sMAC, 0, ETH_ALEN);
    memset(g_info.dMAC, 0, ETH_ALEN);
    g_info.minTTL = 64;
    g_info.maxTTL = 64;
    g_info.interval = 1000000;
    g_info.tos = 0;

    g_info.startingTime = time(NULL);

    // Parse the command line.
    parse_command_line(argc, argv);

    // Check if help flag is set. If so, print help information.
    if (g_info.help)
    {
        fprintf(stdout, "Usage for: %s:\n"
                        "--dev -i => Interface name to bind to.\n"
                        "--src -s => Source address (0/unset = random/spoof).\n"
                        "--dst -d => Destination IP to send packets to.\n"
                        "--port -p => Destination port (0/unset = random port).\n"
                        "--sport => Source port (0/unset = random port).\n"
                        "--internal => When set, if no source IP is specified, it will randomize the source IP from the 10.0.0.0/8 range.\n"
                        "--interval => Interval between sending packets in micro seconds.\n"
                        "--threads -t => Amount of threads to spawn (default is host's CPU count).\n"
                        "--count -c => The maximum packet count allowed sent.\n"
                        "--time => Amount of time in seconds to run tool for.\n"
                        "--smac => Source MAC address in xx:xx:xx:xx:xx:xx format.\n"
                        "--dmac => Destination MAC address in xx:xx:xx:xx:xx:xx format.\n"
                        "--payload => The payload to send. Format is in hexadecimal. Example: FF FF FF FF 49.\n"
                        "--verbose -v => Print how much data we've sent each time.\n"
                        "--nostats => Do not track PPS and bandwidth. This may increase performance.\n"
                        "--urg => Set the URG flag for TCP packets.\n"
                        "--ack => Set the ACK flag for TCP packets.\n"
                        "--psh => Set the PSH flag for TCP packets.\n"
                        "--rst => Set the RST flag for TCP packets.\n"
                        "--syn => Set the SYN flag for TCP packets.\n"
                        "--fin => Set the FIN flag for TCP packets.\n"
                        "--min => Minimum payload length.\n"
                        "--max => Maximum payload length.\n"
                        "--tcp => Send TCP packets.\n"
                        "--icmp => Send ICMP packets.\n"
                        "--icmptype => The ICMP type to send when --icmp is specified.\n"
                        "--icmpcode => The ICMP code to send when --icmp is specified.\n"
                        "--ipip => Add outer IP header in IPIP format.\n"
                        "--ipipsrc => When IPIP is specified, use this as outer IP header's source address.\n"
                        "--ipipdst => When IPIP is specified, use this as outer IP header's destination address.\n"
                        "--nocsum => Do not calculate the IP header's checksum. Useful for checksum offloading on the hardware which'll result in better performance.\n"
                        "--nocsum4 => Do not calculate the layer 4's checksum (e.g. TCP/UDP). It will leave the checksum field as 0 in the headers.\n"
                        "--minttl => The minimum TTL (Time-To-Live) range for a packet.\n"
                        "--maxttl => The maximum TTL (Time-To-Live) range for a packet.\n"
                        "--tos => The TOS (Type Of Service) to set on each packet.\n"
                        "--help -h => Show help menu information.\n",
                argv[0]);

        exit(0);
    }

    // Check if interface argument was set.
    if (g_info.interface == NULL)
    {
        fprintf(stderr, "Missing --dev option.\n");

        exit(1);
    }

    // Check if destination IP argument was set.
    if (g_info.dIP.str == NULL)
    {
        fprintf(stderr, "Missing --dst option\n");

        exit(1);
    }

    // Create pthreads.
    pthread_t pid[g_info.threads];

    // Print information.
    fprintf(stdout, "Launching against %s:%d (0 = random) from interface %s. Thread count => %d and Interval => %" PRIu64 " micro seconds.\n", g_info.dIP.str, g_info.port, g_info.interface, g_info.threads, g_info.interval);

    // Start time.
    startTime = time(NULL);
    uint16_t i;
    // Loop thread each thread.
    for (i = 0; i < g_info.threads; i++)
    {
        // Create new pthread_info struct to pass to thread and copy g_info to it.
        struct pthread_info *info = malloc(sizeof(struct pthread_info));
        memcpy(info, &g_info, sizeof(struct pthread_info));

        // Check for inputted destination MAC.
        if (info->dMAC[0] == 0 && info->dMAC[1] == 0 && info->dMAC[2] == 0 && info->dMAC[3] == 0 && info->dMAC[4] == 0 && info->dMAC[5] == 0)
        {
            // Get destination MAC address (gateway MAC).
            GetGatewayMAC(info->dMAC);
        }
        else
        {
            memcpy(info->dMAC, info->dMAC, ETH_ALEN);
        }

        memcpy(info->sMAC, info->sMAC, ETH_ALEN);

        // Do custom payload if set.
        if (info->payloadStr != NULL)
        {
            // Split argument by space.
            char *split;

            // Create temporary string.
            char *str = malloc((strlen(info->payloadStr) + 1) * sizeof(char));
            strcpy(str, info->payloadStr);

            split = strtok(str, " ");

            while (split != NULL)
            {
                sscanf(split, "%2hhx", &info->payload[info->payloadLength]);

                info->payloadLength++;
                split = strtok(NULL, " ");
            }

            // Free temporary string.
            free(str);
        }

        // Create thread.
        if (pthread_create(&pid[i], NULL, threadHndl, (void *)info) != 0)
        {
            fprintf(stderr, "Error spawning thread %" PRIu16 "...\n", i);
        }
    }

    // Signal.
    signal(SIGINT, signalHndl);

    // Loop!
    while (cont)
    {
        sleep(1);
    }

    // End time.
    time_t endTime = time(NULL);

    // Wait a second for cleanup.
    sleep(1);

    // Statistics
    time_t totalTime = endTime - startTime;

    fprintf(stdout, "Finished in %lu seconds.\n\n", totalTime);

    if (!g_info.nostats)
    {
        uint64_t pps = pcktCount / (uint64_t)totalTime;
        uint64_t MBTotal = totalData / 1000000;
        uint64_t MBsp = (totalData / (uint64_t)totalTime) / 1000000;
        uint64_t mbTotal = totalData / 125000;
        uint64_t mbps = (totalData / (uint64_t)totalTime) / 125000;

        // Print statistics.
        fprintf(stdout, "Packets Total => %" PRIu64 ".\nPackets Per Second => %" PRIu64 ".\n\n", pcktCount, pps);
        fprintf(stdout, "Megabytes Total => %" PRIu64 ".\nMegabytes Per Second => %" PRIu64 ".\n\n", MBTotal, MBsp);
        fprintf(stdout, "Megabits Total => %" PRIu64 ".\nMegabits Per Second => %" PRIu64 ".\n\n", mbTotal, mbps);
    }

    // Exit program successfully.
    exit(0);
}