#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/*
void signal_handler(int signo, siginfo_t info, void *extra)
{
    printf("Signal %d received\n", signo);
    abort();
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_sigaction = SIG_DFL;
    sigaction(SIGSEGV, &action, NULL);
}

void f()
{
    printf("%p,%p\n", __builtin_return_address(0), __builtin_return_address(1));
}

void g()
{
    f();
}

void xqc_sig(int signum)
{
    printf("xqc:%d\n", signum);
    // abort();
    // exit(signum);

    // struct sigaction action;
    // action.sa_flags = 0;
    // action.sa_handler = SIG_DFL;
    // sigaction(signum, &action, NULL);

    g();

    // char cmd[50];
    // sprintf(cmd, "gcore -o /root/core/001_7_id.core %u", getpid());
    // system(cmd);

    signal(signum, SIG_DFL);
    // kill(0, signum);
}

void sigbus_7_error(void)
{
#if defined(__GNUC__)
    #if defined(__i386__)
        __asm__("pushf\norl $0x40000,(%esp)\npopf");
    #elif defined(__x86_64__)
        __asm__("pushf\norl $0x40000,(%rsp)\npopf");
    #endif
#endif

    char *pr = malloc(sizeof(int) + 1);
    int *p = (int*) (pr + 1);
    *p = 42;
    free(pr);

    // char intarray[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    // printf("int1 = 0x%08x, int2 = 0x%08x, int3 = 0x%08x, int4 = 0x%08x\n",
    //         *((int *)(intarray + 1)), *((int *)(intarray + 2)),
    //         *((int *)(intarray + 3)), *((int *)(intarray + 4)));

    printf("error 7: YES\n");
    return;
}

int getcpunum()
{
    char buf[16] = {0};
    int num;
    FILE* fp = popen("cat /proc/cpuinfo |grep processor|wc -l", "r");
    if(!fp) {
        return 1;
    }
    
    fread(buf, 1, sizeof(buf) - 1, fp);
    pclose(fp);
    num = atoi(buf);
    if(num <= 0) {
        num = 1;
    }

    return num;
}

int perf_cpu()
{
    int cpu_size = getcpunum();
    printf("CPU size:%d\n", cpu_size);
    for (int i = 0; i < cpu_size; i++) {
        char cmd[1024] = {0};

        time_t stamp = 0;
        char timeformat[1024] = {0};
        stamp = time(NULL);
        strftime(timeformat, 1024, "%Y_%m_%d_%H_%M_%S", localtime(&stamp));

        sprintf(cmd, "perf record -F 99 -C %d -g -o /sfdata/log/core/cpu%d_%ds_%s.date sleep 10 >> /dev/null", i, i, 10, timeformat);
        system(cmd);
        printf("cmd:%s\n", cmd);
    }
    
    return 0;
}

int main()
{
    struct sigaction action;
    action.sa_flags = 0;
    //action.sa_sigaction = signal_handler;
    action.sa_handler = xqc_sig;
    char *c = "hello, world";

    printf("SA_INTERRUPT:%d\n", SA_INTERRUPT);
    printf("SA_RESTART:%d\n", SA_RESTART);
    printf("SA_SIGINFO:%d\n", SA_SIGINFO);
    printf("SIG_DFL:%p\n", SIG_DFL);

    if (sigaction(SIGBUS, &action, NULL) == -1) {
        perror("sigsegv: sigaction");
        _exit(1);
    }

    struct sigaction cur_handler;
    sigaction(SIGBUS, NULL, &cur_handler);

    // char *argv[]={"ls","-al","/root/", NULL};
    // char *envp[]={0,NULL}; //传递给执行文件新的环境变量数组
    // execve("/bin/ls",argv,envp);

    perf_cpu();

    // sigbus_7_error();
    // int xqc_int;
    // scanf("%d", &xqc_int);
    // //*c = 'H';
    // printf("####################### %s,%d\n", c, xqc_int);
    return 0;
}


*/

/*

#define SF_PERF_FILENAME_MAXLEN    1024
#define SF_PERF_SLEEP              10
#define SF_PERF_TIME_S_INTERVAL    600
uint64_t sf_perf_last_time = 0;
uint64_t sf_perf_type = 0;
sf_spinlock_t sf_perf_lock;

static void* sf_performance_analysis_pthread(void *arg)
{
    sf_uint32_t core_num = sf_get_lcore_count();
    char cmd[SF_PERF_FILENAME_MAXLEN] = {0};

    for (sf_uint32_t i = 0; i < core_num; i++) {
        sprintf(cmd, "perf record -F 99 -C %d -g -o /sfdata/log/core/cpu%d_%ds_%lu.date sleep %d", i, i, SF_PERF_SLEEP, sf_cur_time, SF_PERF_SLEEP);
        // system(cmd);
        log_debug("dp packet", "perf", "core_num %u, cmd:%s\n", core_num, cmd);
    }

    sf_spinlock_lock(&sf_perf_lock);
    sf_perf_type = 0;
    sf_perf_last_time = sf_cur_time;
    sf_spinlock_unlock(&sf_perf_lock);
    return NULL;
}

static void (sf_uint64_t cur_tsc)
{
    pthread_t ntid;
    sf_spinlock_lock(&sf_perf_lock);
    //判断统计进程是否开启，如果线程已经开启退出当前函数。判断距离上次perf统计时间是否超过10分钟
    if (sf_perf_type != 0 || sf_low_latency_cal_time_interval_us(cur_tsc, sf_perf_last_time) < SF_PERF_TIME_S_INTERVAL * 1000000) {
        sf_spinlock_unlock(&sf_perf_lock);
        return;
    }

    sf_perf_type = 1;
    sf_spinlock_unlock(&sf_perf_lock);
    log_debug("dp packet", "perf", "start statistics thread\n");

    //创建一个线程并且开启统计
    if (pthread_create(&ntid, NULL, sf_performance_analysis_pthread, NULL) != 0) {
        sf_spinlock_lock(&sf_perf_lock);
        sf_perf_type = 0;
        sf_perf_last_time = 0;
        sf_spinlock_unlock(&sf_perf_lock);
        log_debug("dp packet", "perf", "thread start failed\n");
    }

    return;
}
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define DDOS_PORT_MAX_SCAN_SIZE            40
#define DOSS_STATICS_MAX                   2

typedef long long           sf_int64_t;
typedef unsigned long long  sf_uint64_t;
typedef int                 sf_int32_t;
typedef unsigned int        sf_uint32_t;
typedef short               sf_int16_t;
typedef unsigned short      sf_uint16_t;
typedef char                sf_int8_t;
typedef unsigned char       sf_uint8_t;
typedef char                sf_char_t;

typedef struct {
    volatile sf_int32_t cnt;
} sf_atomic32_t;

typedef struct {
    volatile sf_int16_t cnt;
} sf_atomic16_t;

typedef struct sf_ipaddr6_
{
    union {
        sf_uint8_t     addr8[16];
        sf_uint16_t    addr16[8];
        sf_uint32_t    addr32[4];
        sf_uint64_t    addr64[2];
    };
} sf_ipaddr6_t;

typedef union sf_ip_addr_
{
    union {
        struct {
            sf_uint32_t __pad[3];
            sf_uint32_t ipv4;
        };
        sf_ipaddr6_t ipv6;
    };
} sf_ip_addr_t;

struct rcu_head
{
    struct rcu_head *next;
    void (*func)(struct rcu_head *head);
};

typedef struct list_head {
    struct list_head *next, *prev;
}list_head_t;

typedef sf_ip_addr_t ddos_ip_t;

struct ddos_ip_entry {
    ddos_ip_t ip;
    sf_uint64_t remove_timeout;
    sf_uint64_t reset_timeout;
    sf_uint64_t host_scan_reset_timeout;
};

typedef struct ddos_dip_entry {
    struct ddos_ip_entry addr;
    sf_atomic32_t udp;
    sf_atomic32_t dns;
    sf_atomic32_t syn;
    sf_uint32_t flags;
    sf_atomic16_t cookie;
} ddos_dip_entry_t;

typedef struct ddos_ip_stat_entry {
    struct ddos_ip_entry addr;
    sf_uint64_t attack_time;
    sf_uint16_t block_time;
    sf_uint8_t source;
    sf_uint8_t flags;
} ddos_ip_stat_entry_t;

typedef struct ddos_ip_stat {
    ddos_ip_t ip;
    sf_uint64_t attack_time;
    sf_uint32_t block_from_now;
    sf_uint16_t block_time;
    sf_uint8_t source;
    sf_uint8_t action;
} ddos_ip_stat_t;

typedef struct ddos_sip_statisc {
    sf_uint64_t reset_port_scan_timeout;
    sf_atomic32_t icmp;
    sf_atomic32_t udp;
    sf_atomic32_t dns;
    sf_atomic32_t syn;
    sf_atomic32_t ack;
    sf_atomic32_t icmp_host;
    sf_atomic32_t scan_count;
    volatile sf_uint32_t   port_count;
    unsigned short port[DDOS_PORT_MAX_SCAN_SIZE];
} ddos_sip_statisc_t;

typedef struct ddos_sip_entry {
    struct ddos_ip_entry addr;
    struct ddos_sip_statisc statis[DOSS_STATICS_MAX];
} ddos_sip_entry_t;

struct inner_sip_entry {
    union {
        struct rcu_head rcu_list;
        struct list_head list;
    };
    ddos_sip_entry_t entry;
};

struct inner_dip_entry {
    union {
        struct rcu_head rcu_list;
        struct list_head list;
    };
    ddos_dip_entry_t entry;
};

struct inner_ip_stat_entry {
    union {
        struct rcu_head rcu_list;
        struct list_head list;
    };
    ddos_ip_stat_entry_t entry;
};

enum ddos_local_bypass_modular {
    DOSS_MODULAR_DDOS = 0,          //ddos本身模块需要放通接口
    DOSS_LINK_DETECTION,            //链路探测
    DOSS_MODULAR_MAX,
};

#define UUID_LEN  (32)

typedef struct sf_ddos_local_bypass_key_{
    sf_ip_addr_t ip;
    sf_uint16_t port;
    sf_uint8_t protocol;
}sf_ddos_local_bypass_key_t;

typedef struct sf_ddos_local_bypass_data_{
    struct list_head node;
    enum ddos_local_bypass_modular modular;
    sf_char_t uuid[UUID_LEN+1];
}sf_ddos_local_bypass_data_t;

int printf_byteH(unsigned long long byte, char *str)
{
    int count = 5, str_size = 0;
    char pr[100] = {0};
    char name[5][3] = {{"TB"}, {"GB"}, {"MB"}, {"KB"}, {"B"}};
    unsigned long long types[5] = {0};

    for (int i = 0; i < count; i++) {
        types[4 - (i%5)] = byte % 1024;
        byte = byte / 1024;
    }
    
    if (!str) {
        str = pr;
    }

    for (int i = 0; i < count; i++) {
        if (types[i]) {
            str_size += sprintf(str + str_size, "%lu%s ", types[i], name[i]);
        }

    }

    printf("%s \n", str);
    return str_size;
}

int main()
{
    printf("sf_ddos_local_bypass_key_t               sizeof:%u\n", sizeof(sf_ddos_local_bypass_key_t));
    printf("sf_ddos_local_bypass_data_t               sizeof:%u\n", sizeof(sf_ddos_local_bypass_data_t));
    printf("max        sizeof:%uKB\n", (sizeof(sf_ddos_local_bypass_key_t) + sizeof(sf_ddos_local_bypass_data_t)) * 256 / 1024);
    printf_byteH((sf_uint64_t)2, NULL);
    printf_byteH((sf_uint64_t)2 * 1024, NULL);
    printf_byteH((sf_uint64_t)2 * 1024 * 1024, NULL);
    printf_byteH((sf_uint64_t)2 * 1024 * 1024 * 1024, NULL);
    printf_byteH((sf_uint64_t)2 * 1024 * 1024 * 1024 * 1024, NULL);
    printf_byteH((sf_uint64_t)2 * 1024 * 1024 * 1024 * 1024 * 1024, NULL);
    printf_byteH((sf_uint64_t)2 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024, NULL);

// #define _Q(x, y) (sf_uint64_t)((&x->y) - x)
//     ddos_dip_entry_t dip = {0};
//     printf("dip");

    return 0;
}