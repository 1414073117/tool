#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

typedef long long int int64;
const int NUM_THREADS = 1; //CPU core nums
int INTERVAL = 100;
int cpuinfo = 15; //CPU utilization rate

// time unit is "ms"
int64 GetTickCount()
{
    struct timespec now;
    int64 sec, nsec;

    clock_gettime(CLOCK_MONOTONIC, &now);
    sec = now.tv_sec;
    nsec = now.tv_nsec;

    return sec * 1000 + nsec / 1000000;
}

void* CPUCost(void *args)
{
    int busyTime = INTERVAL * cpuinfo / 100;
    int idleTime = INTERVAL - busyTime;
    int64 startTime = 0;

    printf("XXXX CPUCost\n");
    printf("XXXX cpuinfo = %d\n", cpuinfo);

    /*
     * within INTERVAL ms, INTERVAL = busyTime + idleTime,
     * spend busyTime ms to let cpu busy,
     * spend idleTime ms top let cpu idle
     */
    while (1) {
        startTime = GetTickCount();
        while((GetTickCount() - startTime) <= busyTime);
        usleep(idleTime * 1000);
    }
}

int main(int argc, char **argv)
{
    pthread_t t[NUM_THREADS];
    int ret;

    printf("please input cpu utilization rate");
    cpuinfo = 10;
    for(int i = 0; i < NUM_THREADS; i++) {
        ret = pthread_create(&t[i], NULL, CPUCost, NULL);
        if(ret)
            printf("XXXX create err");
    }

    pthread_exit(NULL);
    return 0;
}