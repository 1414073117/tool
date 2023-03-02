#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <cpuid.h>

#define FUN_RUN_TIME(ret, fun, ...) \
    { \
        struct timespec start_time, last_time; \
        clock_gettime(CLOCK_MONOTONIC, &start_time); \
        ret = fun(__VA_ARGS__); \
        clock_gettime(CLOCK_MONOTONIC, &last_time); \
        uint64_t slot = (last_time.tv_sec - start_time.tv_sec) * 1000000000 + last_time.tv_nsec - start_time.tv_nsec; \
        printf("%s:%llus%llums%lluus%llums\n", #fun, slot/1000000000, slot/1000000%1000, slot/1000%1000, slot%1000); \
    }

#define FUN_NO_RUN_TIME(fun, ...)     \
    { \
        struct timespec start_time, last_time; \
        clock_gettime(CLOCK_MONOTONIC, &start_time); \
        fun(__VA_ARGS__); \
        clock_gettime(CLOCK_MONOTONIC, &last_time); \
        uint64_t slot = (last_time.tv_sec - start_time.tv_sec) * 1000000000 + last_time.tv_nsec - start_time.tv_nsec; \
        printf("%s:%llus%llums%lluus%llums\n", #fun, slot/1000000000, slot/1000000%1000, slot/1000%1000, slot%1000); \
    }

union {
    uint64_t tsc_64;
    struct {
        uint32_t lo_32;
        uint32_t hi_32;
    };
} tsc;

static unsigned int rte_cpu_get_model(uint32_t fam_mod_step)
{
    uint32_t family, model, ext_model;

    family = (fam_mod_step >> 8) & 0xf;
    model = (fam_mod_step >> 4) & 0xf;

    if (family == 6 || family == 15) {
        ext_model = (fam_mod_step >> 16) & 0xf;
        model += (ext_model << 4);
    }

    return model;
}

static int32_t
rdmsr(int msr, uint64_t *val)
{
    int fd;
    int ret;

    fd = open("/dev/cpu/0/msr", O_RDONLY);
    if (fd < 0)
        return fd;

    ret = pread(fd, val, sizeof(uint64_t), msr);

    close(fd);

    return ret;
}

static uint32_t check_model_wsm_nhm(uint8_t model)
{
    switch (model) {
    /* Westmere */
    case 0x25:
    case 0x2C:
    case 0x2F:
    /* Nehalem */
    case 0x1E:
    case 0x1F:
    case 0x1A:
    case 0x2E:
        return 1;
    }

    return 0;
}

static uint32_t check_model_gdm_dnv(uint8_t model)
{
    switch (model) {
    /* Goldmont */
    case 0x5C:
    /* Denverton */
    case 0x5F:
        return 1;
    }

    return 0;
}

uint64_t get_tsc_freq_arch(void)
{
    uint64_t tsc_hz = 0;
    uint32_t a, b, c, d, maxleaf;
    uint8_t mult, model;
    int32_t ret;

    /*
     * Time Stamp Counter and Nominal Core Crystal Clock
     * Information Leaf
     */
    maxleaf = __get_cpuid_max(0, NULL);
    // printf("maxleaf:%u\n", maxleaf);

    if (maxleaf >= 0x15) {
        __cpuid(0x15, a, b, c, d);

        /* EBX : TSC/Crystal ratio, ECX : Crystal Hz */
        if (b && c)
            return c * (b / a);
    }

    __cpuid(0x1, a, b, c, d);
    model = rte_cpu_get_model(a);
    // printf("model:%u\n", model);

    if (check_model_wsm_nhm(model))
        mult = 133;
    else if ((c & bit_AVX) || check_model_gdm_dnv(model))
        mult = 100;
    else
        return 0;

    ret = rdmsr(0xCE, &tsc_hz);
    // printf("tsc_hz:%llu\n", tsc_hz);
    // printf("mult:%u\n", mult);
    if (ret < 0)
        return 0;

    return ((tsc_hz >> 8) & 0xff) * mult * 1E6;
}


uint64_t rte_rdtsc(void)
{
#ifdef RTE_LIBRTE_EAL_VMWARE_TSC_MAP_SUPPORT
    if (unlikely(rte_cycles_vmware_tsc_map)) {
        /* ecx = 0x10000 corresponds to the physical TSC for VMware */
        asm volatile("rdpmc" : "=a" (tsc.lo_32), "=d" (tsc.hi_32) : "c"(0x10000));
        return tsc.tsc_64;
    }
#endif

    asm volatile("rdtsc" : "=a" (tsc.lo_32), "=d" (tsc.hi_32));
    return tsc.tsc_64;
}

uint64_t get_tsc_freq(void)
{
#define NS_PER_SEC 1E9

    struct timespec sleeptime = {.tv_nsec = NS_PER_SEC / 10 }; /* 1/10 second */

    struct timespec t_start, t_end;
    uint64_t tsc_hz;

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &t_start) == 0) {
        uint64_t ns, end, start = rte_rdtsc();
        nanosleep(&sleeptime,NULL);
        clock_gettime(CLOCK_MONOTONIC_RAW, &t_end);
        end = rte_rdtsc();
        ns = ((t_end.tv_sec - t_start.tv_sec) * NS_PER_SEC);
        ns += (t_end.tv_nsec - t_start.tv_nsec);

        double secs = (double)ns/NS_PER_SEC;
        tsc_hz = (uint64_t)((end - start)/secs);
        return tsc_hz;
    }
    return 0;
}

int main(int argc, char **argv)
{
    uint64_t hz;
    FUN_RUN_TIME(hz, get_tsc_freq);
    printf("%llu\n", hz);
    FUN_RUN_TIME(hz, get_tsc_freq_arch);
    printf("%llu\n", hz);
    return 0;
}