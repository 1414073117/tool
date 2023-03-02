#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define RTE_MAX_NUMA_NODES   100

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

struct atomic_state {
    u32 numa_max_id;
    u32 numa_count;
    int numa_id[RTE_MAX_NUMA_NODES];
}; 

struct A {
    u8 x1;
    u16 x2;
    u32 x3;
    u64 x4;
}; 

void A_printf(void)
{
#define SP(x)   ((u64)&(xqc.x) - (u64)&xqc)
    struct A xqc;
    printf("A size:%d x1:%llu x2:%llu x3:%llu x4:%llu\n", sizeof(xqc), SP(x1), SP(x2), SP(x3), SP(x4));
    // printf("%llu  %llu  %llu\n", &xqc.x2, &xqc.x2 + 1, (u64)&xqc.x1 - (u64)&xqc);
}

struct B {
    u8 x1;
    u32 x2;
    u16 x3;
    u64 x4;
}; 

void B_printf(void)
{
#define SP(x)   ((u64)&(xqc.x) - (u64)&xqc)
    struct B xqc;
    printf("B size:%d x1:%llu x2:%llu x3:%llu x4:%llu\n", sizeof(xqc), SP(x1), SP(x2), SP(x3), SP(x4));
    // printf("%llu  %llu  %llu\n", &xqc.x2, &xqc.x2 + 1, (u64)&xqc.x1 - (u64)&xqc);
}


struct C {
    u8 x1;
    u64 x2;
    u16 x3;
    u32 x4;
}; 

void C_printf(void)
{
#define SP(x)   ((u64)&(xqc.x) - (u64)&xqc)
    struct C xqc;
    printf("C size:%d x1:%llu x2:%llu x3:%llu x4:%llu\n", sizeof(xqc), SP(x1), SP(x2), SP(x3), SP(x4));
    // printf("%llu  %llu  %llu\n", &xqc.x2, &xqc.x2 + 1, (u64)&xqc.x1 - (u64)&xqc);
}

struct atomic_state sf_config;

int sf_numa_id_is_binded(u16 numa_id)
{
    for(int i = 0; i < sf_config.numa_count; i++) {
    	if(sf_config.numa_id[i] == numa_id) {
    		return 1;
    	}
    }

    return 0;
}

void sf_get_and_set_numa_num(void)
{
    sf_config.numa_max_id = 0;
    sf_config.numa_count = 0;
    u16 socket_id;
    memset(sf_config.numa_id, 0 ,sizeof(sf_config.numa_id));
    for (int i = 0; i < 10; i++) {
        socket_id = rand() % RTE_MAX_NUMA_NODES;
        printf("i:%d  socket_id:%u\n", i, socket_id);
        if (socket_id > sf_config.numa_max_id) {
            sf_config.numa_max_id = socket_id;
        }
        if(!sf_numa_id_is_binded(socket_id)) {
            sf_config.numa_id[sf_config.numa_count] = socket_id;
            sf_config.numa_count++;
        }
    }
}

void position_printf(void)
{
    printf("numa_max_id   %u\n", sf_config.numa_max_id);
    printf("numa_count   %u\n", sf_config.numa_count);
    for (int i = 0; i < sf_config.numa_count; i++) {
        printf("i:%d  socket_id:%d\n", i, sf_config.numa_id[i]);
    }
}

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    // sf_get_and_set_numa_num();
    // position_printf();
    A_printf();
    B_printf();
    C_printf();
    return 0;
}