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

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

int if_prime_number(int i)
{
    if (i <= 0) {
        return 0;
    } else if (i == 1 || i == 2) {
        return 1;
    }

    for (int j = 2; j <= i / j; j++) {
        if (i == j * (i / j)) {
            return 0;
        }
    }

    return 1;
}

void range_prime_number(int i)
{
    for (int j = 1; j <= i; j++) {
        if (if_prime_number(j) == 1) {
            printf("%d\n", j);
        }
    }
    
}

int range_mathematical_conjecture(int i)
{
    if (i < 6) {
        return 0;
    }

    for (int aoran = 6; aoran <= i; aoran += 2) {
        int j;
        for (j = 1; j <= aoran / 2; j++) {
            if (if_prime_number(j) == 1 && if_prime_number(aoran - j) == 1) {
                break;
            }
        }

        if (j > aoran / 2) {
           printf("aoran:%d  j%d\n", aoran, j);
           return 0;
        }
        
    }

    return 1;
    
}


#define __DEBUG              0                                             // 调试模式开关，会打开额外输出
#define __TRYTIMES           50                                            // 每个字符尝试读取次数
#define CACHE_HIT_THRESHOLD  (50)
#define __MAGICWORDS         "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"     
#define __MAGICWORDSCOUNT     (sizeof(__MAGICWORDS) - 1)                   // 测试数据长度

unsigned int array1_size = 16;                                            // 前 16 个字符
u8 array1[160]      = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };    // 一个字典
u8 array2[256 * 512];                                                // 256 对应一个字节所能代表的最大个数（2^8）
const char *secret       = __MAGICWORDS;                                  // 测试读取的数据
int iThreshold           = CACHE_HIT_THRESHOLD;                           // 读取时间阀值




int main(int argc, char **argv)
{
    size_t malicious_x = (size_t)((char*)array1 - secret);
    printf("读取地址：%p  %p %p", (void*)malicious_x, secret, array1);
    int range = 100;
    if (!strcmp(argv[1], "0")) {
        range = atoi(argv[2]);
        range_prime_number(range);
    } else if (!strcmp(argv[1], "1")) {
        range = atoi(argv[2]);
        if (range_mathematical_conjecture(range) == 1) {
            printf("YES!\n");
        } else {
            printf("NO!!\n");
        }
    } 
}