#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define CPU_HZ (2900006000)
#define S_NS   (1000000000)

typedef long long           sf_int64_t;
typedef unsigned long long  sf_uint64_t;
typedef int                 sf_int32_t;
typedef unsigned int        sf_uint32_t;
typedef short               sf_int16_t;
typedef unsigned short      sf_uint16_t;
typedef char                sf_int8_t;
typedef unsigned char       sf_uint8_t;

static inline sf_uint64_t sf_rdtsc(void)
{
    struct timespec tp;
    clock_gettime (CLOCK_MONOTONIC_RAW, &tp);
    return (sf_uint64_t)(tp.tv_sec * (sf_uint64_t)1000000000) + (sf_uint64_t)tp.tv_nsec;
}

int main()
{
    sf_uint64_t x1 = 0, x2 = 0;
    struct timespec sleeptime = {.tv_sec = 1, .tv_nsec = 0 };
    x1 = sf_rdtsc();
    nanosleep(&sleeptime,NULL);
    x2 = sf_rdtsc();
    printf("%lu %lu  %lu", x1, x2, x2 - x1);
    return 1;
}