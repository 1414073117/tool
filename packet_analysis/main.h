#ifndef _XQC_MAIN_H
#define _XQC_MAIN_H

#define TCP_FLAG_FIN    0x01
#define TCP_FLAG_SYN    0x02
#define TCP_FLAG_RST    0x04
#define TCP_FLAG_PUSH   0x08
#define TCP_FLAG_ACK    0x10
#define TCP_FLAG_URG    0x20
#define TCP_FLAG_ECE    0x40
#define TCP_FLAG_CWR    0x80
#define TCP_FLAGS_RSTFINACKSYN (TCP_FLAG_RST + TCP_FLAG_FIN + TCP_FLAG_SYN + TCP_FLAG_ACK)
#define TCP_FLAGS_ACKSYN (TCP_FLAG_SYN + TCP_FLAG_ACK) 

//pacp文件头结构体
typedef struct pcap_file_header
{
    uint32 magic;               /* 大小端标识 */
    uint16 version_major;       /* 主要版本 */
    uint16 version_minor;       /* 次要版 */
    int32 thiszone;             /* 时区相关 */
    uint32 sigfigs;             /* 时间戳精度 */
    uint32 snaplen;             /* 最大存储长度 */
    uint32 linktype;            /* 链路类型 */
} pcap_file_header;

//时间戳
typedef struct time_val
{
    uint32 tv_sec;         /* 时间戳单位s */
    uint32 tv_usec;        /* 时间戳单位us */
} time_val;

//pcap数据包头结构体
typedef struct pcap_pkthdr
{
    time_val ts;  /* 时间戳*/
    uint32 caplen; /* 数据区长度 */
    uint32 len;    /* 数据包长度 */
} pcap_pkthdr;

typedef struct Ethhdr_t
{
    uint8 dMAC[6];
    uint8 sMAC[6];
    uint16 type;
}Ethhdr_t;

typedef struct Vlan_t {
    uint16	tci;
    uint16	proto;
} Vlan_t;

typedef struct IPHeader_t {
    uint8 Ver_HLen;         //版本+报头长度
    uint8 TOS;              //服务类型
    uint16 TotalLen;        //总长度
    uint16 ID;              //标识
    uint16 Flag_Segment;    //标志+片偏移
    uint8 TTL;              //生存周期
    uint8 Protocol;         //协议类型
    uint16 Checksum;        //头部校验和
    uint32 SrcIP;           //源IP地址
    uint32 DstIP;           //目的IP地址
} IPHeader_t;

typedef struct TCPUDPHeader_t {
    uint16 SrcPort;       // 源端口号16bit
    uint16 DstPort;       // 目的端口号16bit

}TCPUDPHeader_t;

typedef struct _tcp_header {
    union {
        struct {
            uint16 src_port;
            uint16 dst_port;
        };
        struct {
            uint16 src,dst;
        };
    };
    uint32 seq_number;
    uint32 ack_number;
    uint8 data_offset_and_reserved;
    uint8 flags;
    uint16 window;
    uint16 checksum;
    uint16 urgent_pointer;
}_tcp_header_t;

typedef struct Quintet {
    uint8 dMAC[6];          //目的mac
    uint8 sMAC[6];          //源mac
    uint32 SrcIP;           //源IP地址
    uint32 DstIP;           //目的IP地址
    uint8 Protocol;         //协议类型
    uint16 SrcPort;       // 源端口号
    uint16 DstPort;       // 目的端口号
}Quintet_t;

#endif