#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <time.h>

#ifndef offsetof
#define offsetof(type, field)  ((size_t) &( ((type *)0)->field) )
#endif

#define INIT_LIST_HEAD(p) \
do { \
	struct list_head *__p298 = (p); \
	__p298->next = __p298; \
	__p298->prev = __p298->next; \
} while (0)

#define list_del(i) \
do { \
	(i)->next->prev = (i)->prev; \
	(i)->prev->next = (i)->next; \
} while (0)

#define list_add_head(p, l) \
do { \
	struct list_head *__p298 = (p); \
	struct list_head *__l298 = (l); \
	__p298->next = __l298->next; \
	__p298->prev = __l298; \
	__l298->next->prev = __p298; \
	__l298->next = __p298; \
} while (0)

#define list_add_tail(p, l) \
do { \
	struct list_head *__p298 = (p); \
	struct list_head *__l298 = (l); \
	__p298->prev = __l298->prev; \
	__p298->next = __l298; \
	__l298->prev->next = __p298; \
	__l298->prev = __p298; \
} while (0)

#define list_empty(p)  (((p)->next == (p)) && ((p)->prev == (p)))
#define list_entry(node, type, member)  (type *)((void *)node - offsetof(type, member))
#define list_first_entry(ptr, type, member) (list_entry((ptr)->next, type, member))
#define list_for_each_entry(i, l, name)  for (i = list_entry((l)->next, typeof(*i), name); &i->name != (l); i = list_entry(i->name.next, typeof(*i), name))
#define list_for_each_entry_safe(pos, n, head, member)	 for (pos = list_entry((head)->next, typeof(*pos), member),	n = list_entry(pos->member.next, typeof(*pos), member);	&pos->member != (head);  pos = n, n = list_entry(n->member.next, typeof(*n), member))

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

typedef struct list_head {
	struct list_head *prev;
	struct list_head *next;
}list_head_t;

typedef struct schedule {
    struct schedule *head, *tail;
    char schedule_name[24];
    u8 type;
    u64 speed_time;
}schedule_t;

typedef struct probe_object {
    list_head_t notify_list;
    char probe_object_name[24];
    u64 current_time;
}probe_object_t;

typedef struct probe_strategy {
    u64 uuid;
    struct probe_strategy *head, *tail;
    list_head_t probe_object_table;
    schedule_t* schedule_table;
    u64 speed_time;
    u64 current_time;
    u8 type;
}probe_strategy_t;

probe_strategy_t *probe_strategy_table = NULL;
u64 all_time = 0;

void printf_schedule(schedule_t *p)
{
    printf("### name:%s  type:%u  sleep:%llu\n", p->schedule_name, p->type, p->speed_time);
}

void printf_probe_object(probe_object_t *p)
{
    printf("*** name:%s  update_time:%llu\n", p->probe_object_name, p->current_time);
}

void printf_probe_strategy(probe_strategy_t *p)
{
    
    schedule_t *p_schedule = NULL;
    probe_object_t *p_object = NULL;
    printf("uuid:%llu  ", p->uuid);
    printf("current_time:%llu  ", p->current_time);
    printf("speed_time:%llu  ", p->speed_time);
    printf("type:%u\n", p->type);
    for (p_schedule =  p->schedule_table; p_schedule; p_schedule = p_schedule->tail) {
        printf_schedule(p_schedule);
    }

    list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
        printf_probe_object(p_object);
    }
    
}

int probe_object_update(probe_strategy_t *p)
{
    if (list_empty(&p->probe_object_table) || !p->type) {
        p->current_time = 0;
        return 0;
    }

    p->current_time = list_first_entry(&p->probe_object_table,  probe_object_t, notify_list)->current_time;
    return 0;
}


int probe_schedule_update(probe_strategy_t *p)
{
    u64 min_speed = 0 - 1, max_current_time = 0;
    u8 type = 0;
    schedule_t *p_schedule = NULL;
    probe_object_t *p_object = NULL;
    // struct timespec start_time = { 0 };
    if (!p->schedule_table) {
        p->speed_time = 0;
        p->type = 0;
        return -1;
    }

    for (p_schedule =  p->schedule_table; p_schedule; p_schedule = p_schedule->tail) {
        type  = type | p_schedule->type;
        if (p_schedule->type == 1 && p_schedule->speed_time < min_speed) {
            min_speed = p_schedule->speed_time;
        }
    }

    p->speed_time = min_speed;
    p->type = type;
    if (type == 0 || list_empty(&p->probe_object_table)) {
        return 0;
    }

    // clock_gettime(CLOCK_MONOTONIC, &start_time);
    // max_current_time = start_time.tv_sec + p->speed_time;
    max_current_time = all_time + p->speed_time;
    list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
        if (p_object->current_time > max_current_time) {
            p_object->current_time = max_current_time;
        }
    }

    p->current_time = list_first_entry(&p->probe_object_table,  probe_object_t, notify_list)->current_time;
    return 0;
}

int add_probe_strategy(u64 uuid)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            printf("探测策略已存在uuid:%llu\n", uuid);
            return -1;
        }
    }

    probe_strategy_t *new_p = malloc(sizeof(probe_strategy_t));
    if (!new_p) {
        printf("probe_strategy_t 内存申请失败\n");
        return -1;
    }

    new_p->head = NULL;
    new_p->tail = NULL;
    INIT_LIST_HEAD(&new_p->probe_object_table);
    new_p->schedule_table = NULL;
    new_p->speed_time = 0;
    new_p->current_time = 0;
    new_p->uuid = uuid;
    new_p->type = 0;

    new_p->tail = probe_strategy_table;
    probe_strategy_table = new_p;
    
    if (new_p->tail) {
        new_p->tail->head = new_p;
    }

    return 0;
}

int del_probe_strategy(u64 uuid)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            if (!p->head) {
                probe_strategy_table = p->tail;
            } else {
                p->head->tail = p->tail;
            }

            if(p->tail) {
                p->tail->head = p->head;
            }

            if (!list_empty(&p->probe_object_table)) {
                probe_object_t *p_object = NULL, *p_object_next = NULL; 
                list_for_each_entry_safe(p_object, p_object_next, &p->probe_object_table, notify_list) {
                    list_del(&p_object->notify_list);
                    free(p_object);
                }
            }

            if (p->schedule_table) {
                schedule_t *p_schedule = p->schedule_table, *p_schedule_next = NULL;
                while (p_schedule) {
                    p_schedule_next = p_schedule->tail;
                    free(p_schedule);
                    p_schedule = p_schedule_next;
                }
            }

            free(p);
            return 0;
        }
    }

    printf("探测策略不存在uuid:%llu\n", uuid);
    return -1;
}

int add_probe_object(u64 uuid, char *name)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            probe_object_t *p_object = NULL;
            list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
                if (strcmp(p_object->probe_object_name, name) == 0) {
                    printf("探测策略已存在uuid:%llu, name:%s\n", uuid, name);
                    return -1;
                }
            }

            probe_object_t *new_object = malloc(sizeof(probe_object_t));
            if (!new_object) {
                printf("probe_object_t 内存申请失败\n");
                return -1;
            }

            strcpy(new_object->probe_object_name, name);
            // struct timespec start_time = { 0 };
            // clock_gettime(CLOCK_MONOTONIC, &start_time);
            // new_object->current_time = start_time.tv_sec + p->speed_time;
            new_object->current_time = all_time + p->speed_time;

            if (list_empty(&p->probe_object_table)) {
                list_add_head(&new_object->notify_list, &p->probe_object_table);
            } else {
                list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
                    if (p_object->current_time > new_object->current_time) {
                        list_add_tail(&new_object->notify_list, &p_object->notify_list);
                        break;
                    } else if (p_object->notify_list.next == &p->probe_object_table) {
                        list_add_head(&new_object->notify_list, &p_object->notify_list);
                        break;
                    }
                }
            }

            probe_object_update(p);
            return 0;
        }
    }

    printf("探测策略添加失败uuid:%llu, name:%s\n", uuid, name);
    return -1;
}

int del_probe_object(u64 uuid, char *name)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            probe_object_t *p_object = NULL;
            list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
                if (strcmp(p_object->probe_object_name, name) == 0) {
                    list_del(&p_object->notify_list);
                    free(p_object);
                    probe_object_update(p);
                    return 0;
                }
            }

        }
    }

    printf("del_probe_object 探测策略不存在uuid:%llu, name:%s\n", uuid, name);
    return -1;
}

int add_schedule(u64 uuid, char *name, u64 time_x, u8 type)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            schedule_t *p_schedule = NULL;
            for (p_schedule = p->schedule_table; p_schedule != NULL; p_schedule = p_schedule->tail) {
                if (strcmp(p_schedule->schedule_name, name) == 0) {
                    printf("schedule_t 探测策略已存在uuid:%llu, name:%s\n", uuid, name);
                    return -1;
                }
            }

            schedule_t *new_schedule = malloc(sizeof(schedule_t));
            if (!new_schedule) {
                printf("schedule_t 内存申请失败\n");
                return -1;
            }

            new_schedule->head = NULL;
            new_schedule->tail = NULL;
            strcpy(new_schedule->schedule_name, name);
            new_schedule->speed_time = time_x;
            new_schedule->type = type;

            p_schedule = p->schedule_table;
            if (p_schedule) {
                new_schedule->tail = p_schedule;
                p_schedule->head = new_schedule;
            }
            p->schedule_table = new_schedule;
            probe_schedule_update(p);
            return 0;
        }
    }

    printf("探测策略添加失败uuid:%llu, name:%s\n", uuid, name);
    return -1;
}

int mod_schedule(u64 uuid, char *name, u64 time_x, u8 type)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            schedule_t *p_schedule = NULL;
            for (p_schedule = p->schedule_table; p_schedule != NULL; p_schedule = p_schedule->tail) {
                if (strcmp(p_schedule->schedule_name, name) == 0) {
                    p_schedule->speed_time = time_x;
                    p_schedule->type = type;
                    probe_schedule_update(p);
                    return 0;
                }
            }
        }
    }

    printf("schedule_t 探测策略不存在uuid:%llu, name:%s\n", uuid, name);
    return -1;
}

int del_schedule(u64 uuid, char *name)
{
    probe_strategy_t *p = NULL;
    for (p = probe_strategy_table; p != NULL; p = p->tail) {
        if (p->uuid == uuid) {
            schedule_t *p_schedule = NULL;
            for (p_schedule = p->schedule_table; p_schedule != NULL; p_schedule = p_schedule->tail) {
                if (strcmp(p_schedule->schedule_name, name) == 0) {
                    if (!p_schedule->head) {
                        p->schedule_table = p_schedule->tail;
                    } else {
                        p_schedule->head->tail = p_schedule->tail;
                    }

                    if(p_schedule->tail) {
                        p_schedule->tail->head = p_schedule->head;
                    }

                    free(p_schedule);
                    probe_schedule_update(p);
                    return 0;
                }
            }
        }
    }

    printf("schedule_t 探测策略不存在uuid:%llu, name:%s\n", uuid, name);
    return -1;
}

int single_probe_domain_run(u64 new_time, probe_strategy_t *p, int szie)
{
    if (new_time < p->current_time || !p->type) {
        printf("当前策略uuid:%llu未激活进行遍历  cu time %llu  new time %llu type %u\n", p->uuid, p->current_time, new_time, p->type);
        return -1;
    }

    if (list_empty(&p->probe_object_table)) {
        p->current_time = new_time + p->speed_time;
        printf("域名对象为空列表\n");
        return -1;
    }

    probe_object_t *p_object = NULL;
    while (1) {
        p_object = list_first_entry(&p->probe_object_table,  probe_object_t, notify_list);
        if (szie <= 0 || p_object->current_time > new_time) {
            p->current_time = p_object->current_time;
            break;
        }

        printf("发生当前域名 %s\n", p_object->probe_object_name);
        list_del(&p_object->notify_list);
        p_object->current_time = new_time + p->speed_time;
        list_add_tail(&p_object->notify_list, &p->probe_object_table);
        szie --;
    }

    return 0;
}

int xqc_main_time(void)
{
    u64 uuid = 0, time_x;
    int type_s;
    int size = 5, type = 0, q_time = 0;
    char str[24];
    while (1) {
        printf("$$$$$$$$$$new time %llu $$$$$$$$$$$\n", all_time);
        if (q_time <= 0) {
                q_time = 1;
                size = 5;
            while (1) {
                printf("<0:退出\n0:跳跃时间\n1:最大遍历数量\n2:添加策略\n3:删除策略\n4:添加域名对象\n");
                printf("5:删除域名对象\n6:添加时间计划\n7:修改时间计划\n8:删除时间计划\n");
                printf("9:查询策略\n10:查询域名\n11:查询时间计划\n12:查询全部\n");
                scanf("%d", &type);
                if (type < 0) {
                    break;
                } else if(type == 0) {
                    scanf("%d", &q_time);
                }else if (type == 1) {
                    scanf("%d", &size);
                } else if (type == 2) {
                    scanf("%llu", &uuid);
                    add_probe_strategy(uuid);
                } else if (type == 3) {
                    scanf("%llu", &uuid);
                    del_probe_strategy(uuid);;
                } else if (type == 4) {
                    scanf("%llu%s", &uuid, str);
                    add_probe_object(uuid, str);
                } else if (type == 5) {
                    scanf("%llu%s", &uuid, str);
                    del_probe_object(uuid, str);
                } else if (type == 6) {
                    scanf("%llu%s%llu%d", &uuid, str, &time_x, &type_s);
                    add_schedule(uuid, str, time_x, type_s > 0 ? 1 : 0);
                } else if (type == 7) {
                    scanf("%llu%s%llu%d", &uuid, str, &time_x, &type_s);
                    mod_schedule(uuid, str, time_x, type_s > 0 ? 1 : 0);
                } else if (type == 8) {
                    scanf("%llu%s", &uuid, str);
                    del_schedule(uuid, str);
                } else if (type == 9) {
                    scanf("%llu", &uuid);
                    for (probe_strategy_t *p = probe_strategy_table; p; p = p->tail) {
                        if (p->uuid = uuid) {
                            printf_probe_strategy(p);
                        }
                    }
                } else if (type == 10) {
                    scanf("%llu%s", &uuid, str);
                    for (probe_strategy_t *p = probe_strategy_table; p; p = p->tail) {
                        if (p->uuid = uuid) {
                            probe_object_t *p_object = NULL;
                            list_for_each_entry(p_object, &p->probe_object_table, notify_list) {
                                if (strcmp(p_object->probe_object_name, str) == 0) {
                                    printf_probe_object(p_object);
                                }
                            }
                        }
                    }
                } else if (type == 11) {
                    scanf("%llu%s", &uuid, str);
                    for (probe_strategy_t *p = probe_strategy_table; p; p = p->tail) {
                        if (p->uuid = uuid) {
                            schedule_t *p_schedule = NULL;
                            for (p_schedule = p->schedule_table; p_schedule; p_schedule = p_schedule->tail) {
                                if (strcmp(p_schedule->schedule_name, str) == 0) {
                                    printf_schedule(p_schedule);
                                }
                            }
                        }
                    }
                } else if (type == 12) {
                    for (probe_strategy_t *p = probe_strategy_table; p; p = p->tail) {
                        printf_probe_strategy(p);
                    }
                } else {
                    printf("输入错误重新输入\n");
                }
            }
            
        }

        for (probe_strategy_t *p = probe_strategy_table; p; p = p->tail) {
            if (all_time >= p->current_time && p->type) {
                single_probe_domain_run(all_time, p, size);
            }
        }
        
        q_time --;
        all_time ++;
    }

    return 0;
}

int main(int argc, char **argv)
{
    return xqc_main_time();
}