#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define DNS_MAX   5
#define DNS_INIT(x) memset(x, '\0', sizeof(x)), x##_size = 0

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8; 

int s_dns[DNS_MAX + 1], d_dns[(DNS_MAX + 1)*3];
int s_dns_size = 0, d_dns_size = 0;
int msg_dns[DNS_MAX], del_dns[DNS_MAX], add_dns[DNS_MAX];
int msg_dns_size = 0, del_dns_size = 0, add_dns_size = 0;

void printf_sz(int *p, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%d ", p[i]);
    }

    printf("\n");
    return;
}

int alike(int *sp, int s_size, int *dp, int d_size)
{
    int type;
    if (s_size != d_size) {
        return -1;
    }

    for (int i = 0; i < s_size; i++) {
        type = 0;
        for (size_t j = 0; j < d_size; j++) {
            if (sp[i] == dp[j]) {
                type = 1;
                break;
            }
        }

        if (type != 1) {
            return -1;
        }
    }

    for (int i = 0; i < d_size; i++) {
        type = 0;
        for (size_t j = 0; j < s_size; j++) {
            if (sp[j] == dp[i]) {
                type = 1;
                break;
            }
        }

        if (type != 1) {
            return -1;
        }
    }
    
}

void harmonious(void)
{
    while (1) {
        DNS_INIT(msg_dns);
        DNS_INIT(del_dns);
        DNS_INIT(add_dns);

        for (int i = 0; i < DNS_MAX; i++) {
            int type_m = 0;
            msg_dns[msg_dns_size] = rand()%10 + 1;

            for (int j = 0; j < s_dns_size; j++) {
                if (s_dns[j] == msg_dns[msg_dns_size]) {
                    type_m = 1;
                    break;
                }
            }

            if (type_m != 1) {
                s_dns[s_dns_size] = msg_dns[msg_dns_size];
                s_dns_size++;
            }

            if (s_dns_size > DNS_MAX) {
                int del = rand()%s_dns_size;
                del_dns[del_dns_size] = s_dns[del];
                del_dns_size ++;
                for (int j = del; j < s_dns_size; j++) {
                    if (j == (s_dns_size - 1)) {
                        s_dns[j] = 0;
                    } else {
                        s_dns[j] = s_dns[j+1];
                    }
                }

                s_dns_size --;
            }
            
            for (int j = 0; j < del_dns_size; j++) {
                if (del_dns[j] == msg_dns[msg_dns_size]) {
                add_dns[add_dns_size] = msg_dns[msg_dns_size];
                add_dns_size ++;
                    break;
                }
            }

            msg_dns_size ++;
        }

        for (int i = 0; i < msg_dns_size; i++) {
            int type_q = 0;
            for (int j = 0; j < d_dns_size; j++) {
                if (d_dns[j] == msg_dns[i]) {
                    type_q = 1;
                    break;
                }
            }

            if (type_q == 0) {
                d_dns[d_dns_size] = msg_dns[i];
                d_dns_size ++;
            }
        }

        for (int i = 0; i < del_dns_size; i++) {
            int type_q = -1;
            for (int j = 0; j < d_dns_size; j++) {
                if (d_dns[j] == del_dns[i]) {
                    type_q = j;
                    break;
                }
            }

            if (type_q != -1) {
                for (int j = type_q; j < del_dns_size; j++) {
                    if (j == (del_dns_size - 1)) {
                        d_dns[j] = 0;
                    } else {
                        d_dns[j] = d_dns[j+1];
                    }
                }

                del_dns_size --;
            }
        }

        for (int i = 0; i < add_dns_size; i++) {
            int type_q = 0;
            for (int j = 0; j < d_dns_size; j++) {
                if (d_dns[j] == add_dns[i]) {
                    type_q = 1;
                    break;
                }
            }

            if (type_q == 0) {
                d_dns[d_dns_size] = add_dns[i];
                d_dns_size ++;
            }
        }
    }

    return;
}

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    DNS_INIT(s_dns);
    DNS_INIT(d_dns);
    return 0;
}