#include <stdio.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <time.h>

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

struct ipv4_address{
    union {
        u32 ip4_32;
        u16 ip4_16[2];
        u8  ip4_8[4];
    };
};

struct ipv6_address{
    union {
        u64 ip6_64[2];
        u32 ip6_32[4];
        u16 ip6_16[8];
        u8  ip6_8[16];
    };
};

struct ip_address{
    union {
        struct ipv4_address ipv4;
        struct ipv6_address ipv6;
    };
};

struct ip_segment {
    struct ip_address head;
    struct ip_address end;
};

struct rule {
    u64 uuid;
    struct ip_segment sip;
    struct ip_segment dip;
    struct ip_segment spo;
    struct ip_segment dpo;
};

#define RULE_MAX_SIZE     100000
struct rule* rule_header[RULE_MAX_SIZE];
u64 uuid_situation = 0;

int ip_segment_rand(struct ip_segment *ip)
{
    if (ip) {
        ip->head.ipv4.ip4_8[0] = rand() % 256;
        ip->head.ipv4.ip4_8[1] = rand() % 256;
        ip->head.ipv4.ip4_8[2] = rand() % 256;
        ip->head.ipv4.ip4_8[3] = rand() % 128;
        ip->end.ipv4.ip4_32 = ip->head.ipv4.ip4_32 +  rand() % (256 * 256);
        return 1;
    }

    return 0;
}

void rule_table_printf(struct rule *p)
{
    if (!p) {
        printf("当前为NULL\n");
    }

#define IPV4_PRINTF_0(p, w, s)   p.w.ipv4.ip4_8[s]
#define IPV4_PRINTF_1(p, w)      IPV4_PRINTF_0(p, w, 3), IPV4_PRINTF_0(p, w, 2), IPV4_PRINTF_0(p, w, 1), IPV4_PRINTF_0(p, w, 0)
#define IPV4_PRINTF_2(p)         IPV4_PRINTF_1(p, head), IPV4_PRINTF_1(p, end)
    printf("#########################\n");
    printf("uuid : %llu\n", p->uuid);
    printf("sip  : %u.%u.%u.%u - %u.%u.%u.%u\n", IPV4_PRINTF_2(p->sip));
    printf("dip  : %u.%u.%u.%u - %u.%u.%u.%u\n", IPV4_PRINTF_2(p->dip));
    printf("spo  : %u.%u.%u.%u - %u.%u.%u.%u\n", IPV4_PRINTF_2(p->spo));
    printf("dpo  : %u.%u.%u.%u - %u.%u.%u.%u\n", IPV4_PRINTF_2(p->dpo));
    printf("$$$$$$$$$$$$$$$$$$$$$$$$$\n");

}

int rule_table_add_rand(void)
{
    struct rule *p = malloc(sizeof(struct rule));
    if (p) {
        if (!ip_segment_rand(&p->sip)) {
            return 0;
        }

        if (!ip_segment_rand(&p->dip)) {
            return 0;
        }

        if (!ip_segment_rand(&p->spo)) {
            return 0;
        }

        if (!ip_segment_rand(&p->dpo)) {
            return 0;
        }

        p->uuid = uuid_situation;
        uuid_situation ++;
        for (int i = 0; i < RULE_MAX_SIZE; i++) {
            if (!rule_header[i]) {
                rule_header[i] = p;
                return 1;
            }
        }

        free(p);
    }

    return 0;
}

void rule_table_del_rand(void)
{
    for (int i = 0; i < RULE_MAX_SIZE; i++) {
        if (rule_header[i] && (rand() % 10 == 0)) {
            free(rule_header[i]);
            rule_header[i] = NULL;
        }
    }
        
}

void rule_table_del_all(void)
{
    for (int i = 0; i < RULE_MAX_SIZE; i++) {
        if (rule_header[i]) {
            free(rule_header[i]);
            rule_header[i] = NULL;
        }
    }
}

void rule_table_del_uuid(u64 uuid)
{
    for (int i = 0; i < RULE_MAX_SIZE; i++) {
        if (rule_header[i] && (rule_header[i]->uuid == uuid)) {
            free(rule_header[i]);
            rule_header[i] = NULL;
        }
    }
}

void rule_table_que_uuid(u64 uuid)
{
    for (int i = 0; i < RULE_MAX_SIZE; i++) {
        if (rule_header[i] && (rule_header[i]->uuid == uuid)) {
            rule_table_printf(rule_header[i]);
        }
    }
}

int rule_table_que_quantity(void)
{
    int size_i = 0;
    for (int i = 0; i < RULE_MAX_SIZE; i++) {
        if (rule_header[i]) {
            size_i ++;
        }
    }

    return size_i;
}

void printf_help(void)
{
    printf("moashell:\n");
    printf("    -h help  View help information\n");
    printf("    -s shell  Login method\n");
}

int main(int argc, char **argv)
{
    int si = 0, opt, lx = 0;
    u64 uuid = 0;
    char *arg_xqc = NULL;

    while((opt=getopt(argc,argv,"hs:"))!=-1)
    {
        lx = 1;
        printf("opt %d\n", opt);
        switch(opt)
        {
            case 's':
                arg_xqc = optarg;
                printf("optarg %s\n", optarg);
                break;
            case 'h':
            default:
                printf_help();
                return 0;
        }
    }

    if (lx == 0 && argc >= 2) {
		arg_xqc = argv[1];
	}

    printf("arg_xqc %s\n", arg_xqc);

    srand((unsigned)time(NULL));
    while (1) {
        printf("0:退出\n1:添加\n2:随机删除\n3:删除\n4:查询\n5:输出全部\n");
        printf("6:查询数量\n");
        scanf("%d", &si);
        if (si == 0) {
            break;
        } else if (si == 1) {
            printf("输入随机添加数量\n");
            scanf("%d", &si);
            if (si <=  0) {
                printf("输入错误重新输入\n");
                continue;
            } else {
                for (int i = 0; i < si; i++) {
                    rule_table_add_rand();
                }
            }
        } else if (si == 2) {
            rule_table_del_rand();
        } else if (si == 3) {
            printf("输入删除uuid\n");
            scanf("%llu", &uuid);
            rule_table_del_uuid(uuid);
        } else if (si == 4) {
            printf("输入查询uuid\n");
            scanf("%llu", &uuid);
            rule_table_que_uuid(uuid);
        } else if (si == 5) {
            for (int i = 0; i < RULE_MAX_SIZE; i++) {
                if (rule_header[i]) {
                    rule_table_printf(rule_header[i]);
                }
            }
        } else if (si == 6) {
            printf("%d\n",rule_table_que_quantity());
        } else {
            printf("输入错误重新输入\n");
        }
    }
    
    rule_table_del_all();
    return 0;
}