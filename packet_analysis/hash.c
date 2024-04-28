#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "hash.h"
#include "main.h"

int hash_del_node(void* data)
{
    if (data) {
        free(data);
    }

    return 0;
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
    table->free_fun = (fun == NULL) ? hash_del_node : fun;
    table->node_new = NULL;
    table->node = (exhash_node_t*)malloc(sizeof(exhash_node_t) * table->table_max);
    if (!table->node) {
        printf("malloc exhash node size %lu error!\n", sizeof(exhash_node_t) *  table->table_max);
        return NULL;
    }

    memset(table->node, 0, sizeof(exhash_node_t) * table->table_max);
    // printf("init  max:%lu supe:%lu\n", table->table_max, table->table_supe);
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
            memset(table->node_new, 0, sizeof(exhash_node_t) * (table->table_max * 2));
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
        }

        node = &table->node[(key % table->table_max)];
    }

    data = (exhash_data_t*)malloc(sizeof(exhash_data_t));
    if (!data) {
        printf("malloc exhash data error!\n");
        return -1;
    }

    data->key = key;
    data->user_data = user_data;
    node->node_mun ++;
    table->table_mun ++;
    data->data = node->data;
    node->data = data;
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
        if (data->key == key) {
            return data->user_data;
        }

        data = data->data;
    }
    
    return NULL;
}

void expand_hash_del(exhash_head_t* table, uint64 key)
{
    void *user_date = NULL;
    exhash_node_t* node = NULL;
    exhash_data_t* data = NULL, *data_per = NULL;
    if (!table) {
        printf("search exhash table is NULL!\n");
        return;
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
            table->free_fun(data->user_data);
            free(data);
            return;
        }

        data_per = data;
        data = data->data;
    }
    
    return;
}

void expand_hash_traverse(exhash_head_t* table, int (* fun)(void* data))
{
    exhash_node_t* node = NULL;
    exhash_data_t* data = NULL;
    if (!table) {
        printf("search exhash table is NULL!\n");
        return;
    }

    for (uint64 i = 0; i < table->table_max; i++) {
        node = &table->node[i];
        data = node->data;
        while (data) {
            fun(data->user_data);
            data = data->data;
        }
    }
    
    return;
}