#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define PCAP_ERRBUF_SIZE 1024
#define pcap_t void

typedef unsigned int bpf_u_int32;
typedef unsigned char u_char;

struct bpf_program
{
        unsigned int bf_len;
        void *bf_insns;
};

struct pcap_pkthdr
{
        struct timeval ts;
        bpf_u_int32 caplen;             //表示抓到的数据长度
        bpf_u_int32 len;                //表示数据包的实际长度
};

void get_packet(const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
        int i;

        printf("\n");

        printf("Packet length: %d\n", pkthdr->len);
        printf("Number of bytes: %d\n", pkthdr->caplen);
        printf("Recieved time: %s\n", ctime((const time_t *)&pkthdr->ts.tv_sec));

        for(i=0; i<pkthdr->len; ++i)
        {
                printf("%02x ", packet[i]);

                if((i + 1) % 16 == 0)
                {
                        printf("\n");
                }
        }

        printf("\n\n");
}

int main(int argc, char **argv)
{
        struct in_addr addr;
        struct bpf_program filter;
        struct pcap_pkthdr header;      //由pcap.h定义
        pcap_t *handle;
        char errbuf[PCAP_ERRBUF_SIZE];
        char filter_app[] = "port 23";  //过滤表达式
        char *dev = NULL;
        bpf_u_int32 mask;               //执行嗅探的设备的网络掩码
        bpf_u_int32 net;                //执行嗅探的设备的IP地址
        u_char *packet;                 //实际的包

        dev = (char *)pcap_lookupdev(errbuf);

        memset(&header, 0x00, sizeof(header));

        printf("\nDevice = [%s]\n", dev);

        //printf("errbuf = [%s]\n", errbuf);

        /* 探查设备属性 */
        pcap_lookupnet(dev, &net, &mask, errbuf);

        /* 以混杂模式打开会话 */
        handle = (pcap_t *)pcap_open_live(dev, 1000, 1, 0, errbuf);

        /* 编译过滤器 */
        //pcap_compile(handle, &filter, filter_app, 0, net);

        /* 应用过滤器 */
        //pcap_setfilter(handle, &filter);

        addr.s_addr = net;

        printf("net=[%s]\n", inet_ntoa(addr));

        addr.s_addr = mask;

        printf("mask=[%s]\n", inet_ntoa(addr));

        /* 截获一个包 */
        packet = (u_char *)pcap_next(handle, &header);

        if(!packet)
        {
                printf("did not capture a packet!\n");
                exit(1);
        }

        /* 打印包数据 */
        get_packet(&header, packet);

        /* 关闭会话 */
        pcap_close(handle);

        return 0;
}

// typedef struct sockaddr* saddrp;

// int main(int argc, char const *argv[])
// {
//     int sockfd = socket(AF_INET,SOCK_STREAM,0);
//     if (0 > sockfd)
//     {
//         perror("socket");
//         return -1;
//     }

//     struct sockaddr_in addr = {};
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(atoi(argv[1]));
//     addr.sin_addr.s_addr = inet_addr(argv[2]);

//     int ret =connect(sockfd,(saddrp)&addr,sizeof(addr));
//     if (0 > ret)
//     {
//         perror("connect");
//         return -1;
//     }

//     while(1)
//     {
//         char buf[255] = {};
//         printf(">");
//         gets(buf);
//         send(sockfd,buf,strlen(buf)+1,0);
//         if(0 == strcmp(buf,"q")) break;
//         ret = recv(sockfd,buf,sizeof(buf),0);
//         if (0 > ret)
//         {
//             perror("read");
//             return -1;
//         }
//         printf("Recv:%d Bytes.\nShow:%s\n",ret,buf);
//         if(0 == strcmp(buf,"q")) break;
//     }
//     close(sockfd);
//     return 0;
// }