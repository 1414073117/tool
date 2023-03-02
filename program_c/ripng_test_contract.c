#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_IP "127.10.10.11"
#define PORT 12347
#define MAXDATASIZE 1024

#define RIPNG_V1 1
#define RIPNG_REQUEST 1
#define RIPNG_RESPONSE 2
#define RIPNG_MAX_PACKET_SIZE 1500
#define IPV6_MAX_BYTELEN    16

#define IPV6_ADDR_CMP(D, S) memcmp((D), (S), IPV6_MAX_BYTELEN)
#define IPV6_ADDR_SAME(D, S) (memcmp((D), (S), IPV6_MAX_BYTELEN) == 0)
#define IPV6_ADDR_COPY(D, S) memcpy((D), (S), IPV6_MAX_BYTELEN)

typedef unsigned int    u_int32;
typedef unsigned short  u_int16;
typedef unsigned char   u_int8;

struct ripng_info {
    struct in6_addr nexthop_out;
    struct in6_addr ipv6net_out;
    u_int8 prefixlen;
    u_int8 metric_out;
    u_int16 tag_out;
};

struct ripng_rte_data {
    struct ripng_info rinfo;
    struct ripng_rte_data *next;
};

struct stream {
    size_t size;
    u_int8 data[RIPNG_MAX_PACKET_SIZE - 48];
};


int sltime = 1000;

int stream_putc(struct stream *s, u_char c)
{
    s->data[s->size++] = c;
    return 1;
}

int stream_putw(struct stream *s, u_int16_t w)
{
    s->data[s->size++] = (u_char)(w >>  8);
    s->data[s->size++] = (u_char) w;
    return 2;
}

size_t stream_write(struct stream *s, const void *ptr, size_t size)
{
  memcpy(s->data + s->size, ptr, size);
  s->size += size;
  return size;
}

int ripng_write_rte(int num, struct stream *s, u_int16 flag, struct in6_addr *nexthop, u_int16 tag, u_char metric)
{
    if (num == 0) {
        stream_putc(s, RIPNG_RESPONSE);
        stream_putc(s, RIPNG_V1);
        stream_putw(s, 0);
    }

    stream_write(s, (u_char *)nexthop, sizeof(struct in6_addr));
    stream_putw(s, tag);
    stream_putc(s, flag);
    stream_putc(s, metric);

    return ++num;
}

int ripng_send_packet(u_int8 *buf, int bufsize, struct in6_addr *to)
{
    int ret, sock;
    struct sockaddr_in6 addr;
    struct sockaddr_in6 ripaddr;
    char *inface = "eth2";
    // struct msghdr msg;
    // struct iovec iov;
    // struct cmsghdr *cmsgptr;
    // char adata[256];

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Can't make ripng socket\n");
        return sock;
    }

    memset(&ripaddr, 0, sizeof(ripaddr));
    ripaddr.sin6_family = AF_INET6;
    ripaddr.sin6_port = htons(521);

    ret = bind(sock, (struct sockaddr *)&ripaddr, sizeof(ripaddr));
    if (ret < 0) {
        printf("Can't bind ripng socket: %d.\n", errno);
        return ret;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in6));
    addr.sin6_family = AF_INET6;
    addr.sin6_flowinfo = htonl(0);
    addr.sin6_port = htons(521);

    if (to != NULL) {
        addr.sin6_addr = *to;
    } else {
        inet_pton(AF_INET6, "ff02::9", &addr.sin6_addr);
    }

    struct ifreq nif;
    strcpy(nif.ifr_name, inface);

    /* 绑定接口 */
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) < 0) {
        close(sock);
        printf("bind interface fail, errno: %d \r\n", errno);
        return -1;		
    }

    int hop_limit = 1;
    int val = 255;

    ret = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, sizeof(val));
    if (ret < 0) {
        printf("can't setsockopt IPV6_MULTICAST_HOPS\n");
        return -1;
    }

    /* 修改默认HOP LIMIT */
#ifdef IPV6_RECVHOPLIMIT	/*2292bis-01*/
    ret = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &hop_limit, sizeof(hop_limit));
    if (ret < 0) {
        printf("can't setsockopt IPV6_RECVHOPLIMIT\n");
        return -1;
    }
#else	/*RFC2292*/
    ret = setsockopt(sock, IPPROTO_IPV6, IPV6_HOPLIMIT, &hop_limit, sizeof(hop_limit));
    if (ret < 0) {
        printf("can't setsockopt IPV6_HOPLIMIT\n");
        return -1;
    }
#endif


    ret = sendto(sock, buf, bufsize, 0, ((struct sockaddr *)&addr),sizeof(addr));
    usleep(sltime);

    // msg.msg_name = &addr;
    // msg.msg_namelen = sizeof(addr);
    // msg.msg_iov = &iov;
    // msg.msg_iovlen = 1;
    // msg.msg_iov->iov_base = buf;
    // msg.msg_iov->iov_len = bufsize;
    // msg.msg_control = 0;
    // msg.msg_controllen = 0;
    // msg.msg_flags = 0;

    // sendmsg(sock,&msg,0);

    if (ret < 0) {
        printf("RIPng send fail on %s: %d\n", inface, errno);
    }
    close(sock);

    return ret;
}

void ripng_rte_send(struct ripng_rte_data *data, struct in6_addr *to)
{
    struct in6_addr last_nexthop;
    struct stream s;
    int num = 0, rtemax, ret;

    memset(&last_nexthop, 0, sizeof(last_nexthop));
    s.size = 0;

    rtemax = (RIPNG_MAX_PACKET_SIZE - 52) / 20;

    for (; data; data = data->next) {
        if (!IPV6_ADDR_SAME(&last_nexthop, &data->rinfo.nexthop_out)) {
            if (num == (rtemax - 1)) {
                ret = ripng_send_packet(s.data, s.size, to);
                num = 0;
                s.size = 0;
            }
            last_nexthop = data->rinfo.nexthop_out;
            num = ripng_write_rte(num, &s, 0, &data->rinfo.nexthop_out, 0, 0xff);
        }

        num = ripng_write_rte(num, &s, data->rinfo.prefixlen, &data->rinfo.ipv6net_out, data->rinfo.tag_out, data->rinfo.metric_out);
        if (num == rtemax) {
            ret = ripng_send_packet(s.data, s.size, to);
            num = 0;
            s.size = 0;
        }
    }

    if (num != 0) {
        ret = ripng_send_packet((caddr_t)s.data, s.size, to);
        s.size = 0;
    }
}

void ripng_new_nodes(int max)
{
    struct ripng_rte_data *head = NULL, *tail = NULL, *p = NULL;

    if (max <= 0 || max >= 0xffffffff) {
        printf("maximum number error!!!\n");
        return;
    }

    for (int i = 0; i < max; i++) {
        p = (struct ripng_rte_data *)malloc(sizeof(struct ripng_rte_data));
        if (p == NULL) {
            printf("(ripng_rte_data) memory request failed!!\n");
            return;
        }

        p->next = NULL;
        p->rinfo.prefixlen = 112;
        p->rinfo.metric_out = 2;
        p->rinfo.tag_out = htons(1);
        p->rinfo.nexthop_out.s6_addr32[0] = 0x80FE;
        p->rinfo.nexthop_out.s6_addr32[1] = 0x0;
        p->rinfo.nexthop_out.s6_addr32[2] = 0xff0f6800;
        p->rinfo.nexthop_out.s6_addr32[3] = 0xab0adbfe;
        p->rinfo.ipv6net_out.s6_addr32[0] = 0x220;
        p->rinfo.ipv6net_out.s6_addr32[1] = htonl(0x0 + i);
        p->rinfo.ipv6net_out.s6_addr32[2] = 0x0;
        p->rinfo.ipv6net_out.s6_addr32[3] = 0x0;

        if (head == NULL) {
            head = p;
        } else {
            tail->next = p;
        }
        tail = p;
    }

    ripng_rte_send(head, NULL);
}

#if 0
static int ripng_make_socket(void)
{
    int ret, sock;
    struct sockaddr_in6 ripaddr;

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Can't make ripng socket");
        return sock;
    }

    memset(&ripaddr, 0, sizeof(ripaddr));
    ripaddr.sin6_family = AF_INET6;
    ripaddr.sin6_len = sizeof(struct sockaddr_in6);
    ripaddr.sin6_port = htons(521);

    ret = bind(sock, (struct sockaddr *)&ripaddr, sizeof(ripaddr));
    if (ret < 0) {
        printf("Can't bind ripng socket: %s.", safe_strerror(errno));
        return ret;
    }

    return sock;
}
#endif

int main(int argc, char **argv)
{
    int max = 10;

    if (argc >= 2) {
        max = atoi(argv[1]);
        if (argc >= 3) {
            sltime = atoi(argv[2]);
        }
    }

    printf("ripng number of routing entries %d, Intervals %dus\n", max, sltime);
    ripng_new_nodes(max);
    return 0;
}