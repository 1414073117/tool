#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include "hash.h"
#include "main.h"

typedef struct pact_statistics {
    uint64 number_packets;
    uint64 total_length;
}pact_statistics_t;

typedef struct tcp_udp_node {
    uint16 sport;
    uint16 dport;
    uint8 protocol;
    pact_statistics_t pact;
}tcp_udp_node_t;

typedef struct ip_node {
    uint8 dMAC[6];
    uint8 sMAC[6];
    uint32 sip;
    uint32 dip;
    time_val start_time;
    time_val stop_time;
    uint64 number_packets;
    uint64 total_length;
    exhash_head_t *pact_hash;
    pact_statistics_t tcp;
    pact_statistics_t udp;
    pact_statistics_t icmp;
    pact_statistics_t other;
}ip_node_t;

exhash_head_t *hash_ip;

int hash_ip_del_node(void* data)
{
    ip_node_t *p = (ip_node_t *)data;
    if (p) {
        if (p->pact_hash) {
            expand_hash_exit(p->pact_hash);
        }

        free(p);
    }
}

int printf_tcp_udp_node(void *data)
{
    tcp_udp_node_t *p = (tcp_udp_node_t *)data;
    if (!p) {
        return -1;
    }

    printf("        %s: sport:%-5d dport:%-5d\n", p->protocol == 6 ? "TCP" : "UDP", ntohs(p->sport), ntohs(p->dport));
    printf("        pact: packets %u length %u\n", p->pact.number_packets, p->pact.total_length);
    return 0;
}

int printf_ip_node(void *data)
{
    struct timeval tv;
    struct tm *tm;
    char dmac[18], smac[18];
    char Sip[16], dip[16];
    char t_buf[64], p_buf[64];
    ip_node_t *p = (ip_node_t *)data;
    if (!p) {
        return -1;
    }

    sprintf(dmac, "%02x:%02x:%02x:%02x:%02x:%02x", p->dMAC[0], p->dMAC[1], p->dMAC[2], p->dMAC[3], p->dMAC[4], p->dMAC[5]);
    sprintf(smac, "%02x:%02x:%02x:%02x:%02x:%02x", p->sMAC[0], p->sMAC[1], p->sMAC[2], p->sMAC[3], p->sMAC[4], p->sMAC[5]);
    inet_ntop(AF_INET,&(p->sip),Sip,sizeof(Sip));
    inet_ntop(AF_INET, &(p->dip),dip,sizeof(dip));
    printf("%s %s %-16.16s \t %-16.16s \t %-5u \t %-5u\n", dmac, smac, Sip, dip, p->number_packets, p->total_length);

    tv.tv_sec = p->start_time.tv_sec;
    strftime(t_buf, sizeof(t_buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    tv.tv_sec = p->stop_time.tv_sec;
    strftime(p_buf, sizeof(p_buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    printf("    start time:%s.%06ld  stop time:%s.%06ld\n", t_buf,  p->start_time.tv_usec, p_buf,  p->stop_time.tv_usec);
    printf("    tcp  : packets %u length %u\n", p->tcp.number_packets, p->tcp.total_length);
    printf("    udp  : packets %u length %u\n", p->udp.number_packets, p->udp.total_length);
    printf("    icmp : packets %u length %u\n", p->icmp.number_packets, p->icmp.total_length);
    printf("    other: packets %u length %u\n", p->other.number_packets, p->other.total_length);

    if (p->pact_hash) {
        expand_hash_traverse(p->pact_hash, printf_tcp_udp_node);
    }

    return 0;
}

void printf_packet_hash(exhash_head_t *hash_ip)
{
    if (!hash_ip) {
        return;
    }

    expand_hash_traverse(hash_ip, printf_ip_node);
    return;
}

int packet_hash(pcap_pkthdr *pcap_pkthdr, Quintet_t *quintet, uint32 len)
{
    uint64 key = ((uint64)quintet->DstIP) << 32 + (uint64)quintet->SrcIP;
    ip_node_t *ip_node = expand_hash_search(hash_ip, key);
    if (!ip_node) {
        ip_node = malloc(sizeof(ip_node_t));
        if (!ip_node) {
            printf("malloc ip_node_t error!");
            return -1;
        }

        memset(ip_node,0,sizeof(ip_node_t));
        ip_node->start_time = pcap_pkthdr->ts;
        memcpy(ip_node->dMAC, quintet->dMAC, sizeof(ip_node->dMAC));
        memcpy(ip_node->sMAC, quintet->sMAC, sizeof(ip_node->sMAC));
        ip_node->sip = quintet->SrcIP;
        ip_node->dip = quintet->DstIP;
        ip_node->pact_hash = expand_hash_init(4, 10, NULL);
        if (!ip_node->pact_hash) {
            free(ip_node);
            printf("init hash pact_hash error!\n");
            return -1;
        }

        if (expand_hash_add(hash_ip, key, ip_node) != 0) {
            printf("add hash pact_hash error!\n");
            return -1;
        }
    }

    ip_node->stop_time = pcap_pkthdr->ts;
    ip_node->number_packets ++;
    ip_node->total_length += len;

    if (quintet->Protocol == 1) {
        ip_node->icmp.number_packets ++;
        ip_node->icmp.total_length += len;

    } else if (quintet->Protocol == 6 || quintet->Protocol == 17) {
        uint64 pact_key = ((uint64)quintet->DstPort) << 24 + ((uint64)quintet->SrcPort) << 8 + (uint64)quintet->Protocol;
        tcp_udp_node_t *tcp_udp_node = expand_hash_search(ip_node->pact_hash, pact_key);
        if (!tcp_udp_node) {
            tcp_udp_node = malloc(sizeof(tcp_udp_node_t));
            if (!tcp_udp_node) {
                printf("malloc tcp_udp_node error!");
                return -1;
            }

            memset(tcp_udp_node,0,sizeof(tcp_udp_node_t));
            tcp_udp_node->dport = quintet->DstPort;
            tcp_udp_node->sport = quintet->SrcPort;
            tcp_udp_node->protocol = quintet->Protocol;

            if (expand_hash_add(ip_node->pact_hash, pact_key, tcp_udp_node) != 0) {
                printf("add hash tcp_udp_node_t error!\n");
                return -1;
            }
        }

        tcp_udp_node->pact.number_packets ++;
        tcp_udp_node->pact.total_length += len;
        if (quintet->Protocol == 6) {
            ip_node->tcp.number_packets ++;
            ip_node->tcp.total_length += len;
        } else {
            ip_node->udp.number_packets ++;
            ip_node->udp.total_length += len;
        }
    } else {
        ip_node->other.number_packets ++;
        ip_node->other.total_length += len;
    }

    return 0;
}

int parse_packet_header(pcap_pkthdr *pcap_pkthdr, int8 *pkt, uint32 len)
{
    Quintet_t quintet;
    char dmac[18], smac[18];
    char Sip[16], dip[16];
    Ethhdr_t *ethhdr_header = NULL;
    Vlan_t *vlan_header = NULL;
    IPHeader_t *ip_header = NULL;
    TCPUDPHeader_t *tcpudp_header = NULL;
    uint32 position = 0;
    uint16 eth_type = 0;

    if ((position + sizeof(Ethhdr_t)) > len) {
        printf("len %u: can not read Quintet_t or len error\n", position);
        return -1;
    }

    memset(&quintet, 0, sizeof(Quintet_t));
    ethhdr_header = (Ethhdr_t *)(pkt + position);
    position += sizeof(Ethhdr_t);
    memcpy(quintet.dMAC, ethhdr_header->dMAC, sizeof(quintet.dMAC));
    memcpy(quintet.sMAC, ethhdr_header->sMAC, sizeof(quintet.sMAC));
    if (ethhdr_header->type == htons(0x8100)) {
        if ((position + sizeof(Vlan_t)) > len) {
            printf("Vlan_t: abnormal packet length %u\n", len);
            return -1;
        }
    
        vlan_header = (Vlan_t *)(pkt + position);
        position += sizeof(Vlan_t);
        eth_type = vlan_header->proto;
    } else {
        eth_type = ethhdr_header->type;
    }

    if (eth_type != htons(0x0800) || (position + sizeof(IPHeader_t)) > len) {
        printf("Unable to parse ip packet type:%x\n", htons(eth_type));
        return -1;
    }


    ip_header = (IPHeader_t *)(pkt + position);
    position += sizeof(IPHeader_t);
    quintet.SrcIP = ip_header->SrcIP;
    quintet.DstIP = ip_header->DstIP;
    quintet.Protocol = ip_header->Protocol;

    if(quintet.Protocol == 6 || quintet.Protocol == 17) {
        if((position + sizeof(TCPUDPHeader_t)) > len) {
            printf("TCPUDPHeader_t: abnormal packet length %u\n", len);
            return -1;
        }

        tcpudp_header = (TCPUDPHeader_t *)(pkt + position);
        position += sizeof(TCPUDPHeader_t);

        quintet.SrcPort = tcpudp_header->SrcPort;
        quintet.DstPort = tcpudp_header->DstPort;
    }

    sprintf(dmac, "%02x:%02x:%02x:%02x:%02x:%02x", quintet.dMAC[0], quintet.dMAC[1], quintet.dMAC[2], quintet.dMAC[3], quintet.dMAC[4], quintet.dMAC[5]);
    sprintf(smac, "%02x:%02x:%02x:%02x:%02x:%02x", quintet.sMAC[0], quintet.sMAC[1], quintet.sMAC[2], quintet.sMAC[3], quintet.sMAC[4], quintet.sMAC[5]);
    inet_ntop(AF_INET,&(quintet.SrcIP),Sip,sizeof(Sip));
    inet_ntop(AF_INET, &(quintet.DstIP),dip,sizeof(dip));

    // printf("%s %s %-16.16s \t %-16.16s \t %-5d \t %-5d \t %-8d\n", dmac, smac, Sip, dip, ntohs(quintet.SrcPort), ntohs(quintet.DstPort), quintet.Protocol);
    packet_hash(pcap_pkthdr, &quintet, len);
    return 0;
}

int main(int argc,char *argv[])
{
    pcap_file_header pcap_file_header;
    pcap_pkthdr *ptk_header = NULL;//设置将要读取的pcap包的包头
    FILE* pFile = NULL;
    uint8 *pkt = NULL;
    int i = 0;

    ptk_header  = (pcap_pkthdr *)malloc(sizeof(pcap_pkthdr));
    pFile = fopen( argv[1], "r");
    if( pFile == 0) {
        printf( "打开pcap文件失败");
        return 0;
    }

    if(fread(&pcap_file_header, sizeof(pcap_file_header), 1, pFile) != 1) {
        printf("%d: can not read pcap_file_header\n");
        return 2;
    }

    hash_ip = expand_hash_init(10, 16, hash_ip_del_node);
    if (!hash_ip) {
        printf("init ip hash error\n");
        return 3;
    }

    while(1) {
        memset(ptk_header, 0, sizeof(pcap_pkthdr));
        if(fread(ptk_header, sizeof(pcap_pkthdr), 1, pFile) != 1) {
            printf("%d: can not read ptk_header\n", i);
            break;
        }

        // printf("caplen:%d\n",ptk_header->caplen);
        pkt = (uint8 *)malloc(sizeof(uint8) * ptk_header->caplen);
        if (!pkt) {
            printf("%d: can not read pkt\n", i);
        }

        memset(pkt, 0, sizeof(uint8) * ptk_header->caplen);
        if(fread(pkt, sizeof(uint8) * ptk_header->caplen, 1, pFile) != 1) {
            printf("%d: can not read ip_header\n", i);
            free(pkt);
            break;
        }

        parse_packet_header(ptk_header, pkt, ptk_header->len);
        free(pkt);
        i++;
    }

    printf_packet_hash(hash_ip);
    fclose(pFile);
    free(ptk_header);
    expand_hash_exit(hash_ip);
    return 0;
}
