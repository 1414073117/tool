#ifndef _XQC_HASH_H
#define _XQC_HASH_H

typedef long long int           int64;
typedef unsigned long long int  uint64;
typedef int                     int32;
typedef unsigned int            uint32;
typedef short                   int16;
typedef unsigned short          uint16;
typedef char                    int8;
typedef unsigned char           uint8;

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

extern exhash_head_t* expand_hash_init(uint8 max, uint8 supe, int (* fun)(void* data));
extern int expand_hash_exit(exhash_head_t* table);
extern int expand_hash_add(exhash_head_t* table, uint64 key, void* user_data);
extern void* expand_hash_search(exhash_head_t* table, uint64 key);
extern void expand_hash_del(exhash_head_t* table, uint64 key);
extern void expand_hash_traverse(exhash_head_t* table, int (* fun)(void* data));

#endif