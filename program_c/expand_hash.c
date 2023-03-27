#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

typedef long long int           int64;
typedef unsigned long long int  uint64;
typedef int                     int32;
typedef unsigned int            uint32;
typedef short                   int16;
typedef unsigned short          uint16;
typedef char                    int8;
typedef unsigned char           uint8;

typedef struct user_xqc
{
    uint64 data;
    struct user_xqc* next;
} user_xqc_t;

typedef struct exhash_data
{
    struct exhash_data* data;
    uint64 key;
    void* user_data;
} exhash_data_t;

typedef struct exhash_node
{
    uint64 node_mun;
    exhash_data_t* data;
} exhash_node_t;

typedef struct exhash_head
{
    uint64 table_max;
    uint64 table_mun;
    uint64 table_supe;
    int (* free_fun)(void* data);
    exhash_node_t* node;
    exhash_node_t* node_new;
} exhash_head_t;

uint64 x_rand()
{
    return ((unsigned long long)lrand48() << 32) | lrand48();
}

exhash_head_t* expand_hash_init(uint8 max, uint8 supe, int (* fun)(void* data))
{
    exhash_head_t* table = (exhash_head_t*)malloc(sizeof(exhash_head_t));
    if (!table) {
        printf("malloc exhash head error!\n");
        return NULL;
    }

    table->table_max = ((uint64)1) << max;
    table->table_supe = ((uint64)1) << supe;
    table->table_mun = 0;
    table->free_fun = fun;
    table->node_new = NULL;
    table->node = (exhash_node_t*)malloc(sizeof(exhash_node_t) * table->table_max);
    if (!table->node) {
        printf("malloc exhash node size %lu error!\n", sizeof(exhash_node_t) *  table->table_max);
        return NULL;
    }

    memset(table->node, 0, sizeof(exhash_node_t) * table->table_max);
    
    printf("init  max:%lu supe:%lu\n", table->table_max, table->table_supe);
    return table;
}

int expand_hash_exit(exhash_head_t* table)
{
    exhash_node_t* node = NULL;
    exhash_data_t* data = NULL;
    if (table) {
        for (uint64 i = 0; i < table->table_max; i++) {
            node = &table->node[i];
            while (node->data) {
                data = node->data;
                node->data = data->data;
                table->free_fun(data->user_data);
                free(data);
            }
        }

        free(table->node);
        free(table);
    }
}

int expand_hash_add(exhash_head_t* table, uint64 key, void* user_data)
{
    exhash_node_t* node = NULL, *kz1_node = NULL, *kz2_node = NULL;
    exhash_data_t* data = NULL, *data_us = NULL;
    if (!table || !user_data) {
        printf("add exhash table or user_data is NULL!\n");
        return -1;
    }
    
    node = &table->node[(key % table->table_max)];
    while (node->node_mun >= (((uint64)1) << 4) && table->table_max < table->table_supe) {
        table->node_new = (exhash_node_t*)malloc(sizeof(exhash_node_t) * (table->table_max * 2));
        if (table->node_new) {
            for (uint64 i = 0; i < table->table_max; i++) {
                kz1_node = &table->node[i];
                while (kz1_node->data) {
                    data_us = kz1_node->data;
                    kz1_node->data = data_us->data;
                    kz1_node->node_mun --;
                    
                    kz2_node = &table->node_new[data_us->key % (table->table_max * 2)];
                    data_us->data = kz2_node->data;
                    kz2_node->data = data_us;
                    kz2_node->node_mun ++;
                }
            }

            free(table->node);
            table->node = table->node_new;
            table->node_new = NULL;
            table->table_max *= 2;
            // printf("Extension size %lu!!\n", table->table_max);
        } else {
            // printf("Extension failed size %lu!!\n", table->table_max * 2);
        }

        node = &table->node[(key % table->table_max)];
    }

    data = (exhash_data_t*)malloc(sizeof(exhash_data_t));
    if (!data) {
        printf("malloc exhash data error!\n");
        return -1;
    }

    data->key = key;
    data->user_data =user_data;
    node->node_mun ++;
    table->table_mun ++;
    data->data = node->data;
    node->data =data;
    return 0;
}

void* expand_hash_search(exhash_head_t* table, uint64 key)
{
    exhash_node_t* node = NULL;
    exhash_data_t* data = NULL;
    if (!table) {
        printf("search exhash table is NULL!\n");
        return NULL;
    }

    node = &table->node[(key % table->table_max)];
    data = node->data;
    while (data) {
        if (data->key ==key) {
            return data->user_data;
        }

        data = data->data;
    }
    
    return NULL;
}

void* expand_hash_del(exhash_head_t* table, uint64 key)
{
    void *user_date = NULL;
    exhash_node_t* node = NULL;
    exhash_data_t* data = NULL, *data_per = NULL;
    if (!table) {
        printf("search exhash table is NULL!\n");
        return NULL;
    }

    node = &table->node[(key % table->table_max)];
    data = node->data;
    while (data) {
        if (data->key ==key) {
            if (data == node->data) {
                node->data = data->data;
            } else {
                data_per->data = data->data;
            }

            node->node_mun --;
            table->table_mun --;
            user_date = data->user_data;
            free(data);
            return user_date;
        }

        data_per = data;
        data = data->data;
    }
    
    return NULL;
}

uint64 hash_key(user_xqc_t* data)
{
    return data->data;
}

user_xqc_t* new_xqc()
{
    user_xqc_t* p = (user_xqc_t*)malloc(sizeof(user_xqc_t));
    if (!p) {
        printf("new user xqc malloc error\n");
        return NULL;
    }

    p->data = x_rand();
    p->next = NULL;
    return p;
}

int exit_xqc(void* data)
{
    if (data) {
        free(data);
    }

    return 0;
}

void hash_add_xqc(uint64 size_x, exhash_head_t* table, user_xqc_t** xqc)
{
    user_xqc_t* data = NULL;
    if (size_x <= 0 || !table || !xqc) {
        return ;
    }

    for (uint64 i = 0; i < size_x; i ++) {
        data = new_xqc();
        if (data) {
            while (expand_hash_search(table, hash_key(data))) {
                exit_xqc(data);
                data = NULL;
                data = new_xqc();
            }

            expand_hash_add(table, hash_key(data), (void*)data);
            data->next = *xqc;
            *xqc =  data;
        }
    }
}

void hash_search_xqc(exhash_head_t* table, user_xqc_t* xqc)
{
    uint64 size_x = 0;
    user_xqc_t* data = xqc;
    if (!table || !xqc) {
        return ;
    }

    while (data) {
        if (expand_hash_search(table, hash_key(data)) != data) {
            printf("search %lu error xqc!!\n", data->data);
        } else {
            size_x ++;
        }

        data = data->next;
    }

    // printf("search size %lu\n", size_x);
}

void hash_del_xqc(exhash_head_t* table, user_xqc_t** xqc)
{
    uint64 size_x = 0;
    user_xqc_t* data = *xqc, *data_per = NULL;
    if (!table || !xqc) {
        return ;
    }

    while (data) {
        if (!expand_hash_search(table, hash_key(data))) {
            printf("search %lu error xqc!!\n", data->data);
            data_per = data;
            data = data_per->next;
        } else {
            if (expand_hash_del(table, hash_key(data)) != data) {
                printf("search %lu exist no xqc!!\n", data->data);
                data_per = data;
                data = data_per->next;
            } else {
                size_x ++;
                if (data == *xqc) {
                    *xqc = data->next;
                    data_per = NULL;
                    free(data);
                    data = *xqc;
                } else {
                    data_per->next = data->next;
                    free(data);
                    data = data_per->next;
                }
            }
        }
    }

    // printf("del size %lu\n", size_x);
}

#define TIME_NS(time_duan) ((((uint64)(time_duan) * 1000) / (CLOCKS_PER_SEC / 1000000)) % 1000)
#define TIME_US(time_duan) ((((uint64)(time_duan) * 1000) / (CLOCKS_PER_SEC / 1000)) % 1000)
#define TIME_MS(time_duan) (((uint64)(time_duan) / (CLOCKS_PER_SEC / 1000)) % 1000)
#define TIME_S(time_duan) ((uint64)(time_duan) / CLOCKS_PER_SEC)
#define PRINTF_TIME_AEC(fun_name, time_duan) printf("%-9s: %lu.%03lu%03lu%03lus\n", fun_name, TIME_S(time_duan), TIME_MS(time_duan), TIME_US(time_duan))

void printf_time_interval(char *str_x, uint64 interval)
{
    printf("%-9s: %lu.%03lu%03lu%03lus\n", str_x, TIME_S(interval), TIME_MS(interval), TIME_US(interval), TIME_NS(interval));
}

void test_xqc(uint64 size_x, exhash_head_t* table, user_xqc_t** xqc)
{
    clock_t start_time, end_time;
    printf("####################START####################\n");
    printf("%lu\n", size_x);
    start_time = clock();
    hash_add_xqc(size_x, table, xqc);
    end_time = clock();
    PRINTF_TIME_AEC("add", end_time - start_time);
    PRINTF_TIME_AEC("one add", (end_time - start_time) / size_x);
    start_time = clock();
    hash_search_xqc(table, *xqc);
    end_time = clock();
    PRINTF_TIME_AEC("search", end_time - start_time);
    PRINTF_TIME_AEC("one search", (end_time - start_time) / size_x);
    start_time = clock();
    hash_del_xqc(table, xqc);
    end_time = clock();
    PRINTF_TIME_AEC("del", end_time - start_time);
    PRINTF_TIME_AEC("one del", (end_time - start_time) / size_x);
    printf("####################STOP#####################\n");
}

int main(int argc, char *argv[])
{
    srand48(time(NULL));
    exhash_head_t* table = expand_hash_init(10, 32, exit_xqc);
    user_xqc_t* xqc = NULL;
    test_xqc(100, table, &xqc);
    test_xqc(1000, table, &xqc);
    test_xqc(10000, table, &xqc);
    test_xqc(100000, table, &xqc);
    test_xqc(1000000, table, &xqc);
    test_xqc(10000000, table, &xqc);
    // test_xqc(100000000, table, &xqc);
    expand_hash_exit(table);
    return 0;
}