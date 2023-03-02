#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <errno.h>

#define MYPORT 12345

//最多处理的connect
#define MAX_EVENTS 500

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

int main()
{
    struct epoll_event eventList[MAX_EVENTS];
    struct itimerspec new_value;
    struct timespec start_time = { 0 }, last_time = { 0 };
    u64 time_space = 0;
    int totalExp = 0;
    /*init time*/
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 100 * 1000;
    /*time interval*/
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 100 * 1000;

    // epoll 初始化
    int epollfd = epoll_create(MAX_EVENTS);
    if (epollfd < 0)
    {
        printf("%s\n", "epoll error");
        return -1;
    }

    int timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerFd < 0)
    {
        printf("%s\n", "timer fd error");
        return -1;
    }

    int ret = timerfd_settime(timerFd, 0, &new_value, NULL);
    if (ret < 0)
    {
        printf("%s\n", "timerfd_settime error");
        close(timerFd);
        return -1;
    }

    /* add to epoll */
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
    ev.data.fd = timerFd;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // add Event
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, timerFd, &ev) < 0)
    {
        printf("epoll add fail : fd = %d\n", timerFd);
        return -1;
    }

    u64 min = 0, max = 0;
    int w_i = 10;
    // epoll
    while (w_i)
    {
        // epoll_wait
        int ret = epoll_wait(epollfd, eventList, MAX_EVENTS, -1);

        if (ret < 0)
        {
            printf("epoll error\n");
            break;
        }

        // usleep(100);

        clock_gettime(CLOCK_MONOTONIC, &last_time);
        time_space = (last_time.tv_sec - start_time.tv_sec) * 1000 * 1000 * 1000;
        time_space = time_space + (last_time.tv_nsec) - (start_time.tv_nsec);
        // printf("sleep timer %llu us\n" ,time_space);
        if (w_i == 10) {
            min = max = time_space;
        } else {
            max = max > time_space ? max : time_space;
            min = min < time_space ? min : time_space;
        }
        start_time = last_time;
        // clock_gettime(CLOCK_MONOTONIC, &start_time);

        直接获取了事件数量,给出了活动的流,这里是和poll区别的关键
        for (int i = 0; i < ret; ++i)
        {
            if (eventList[i].events & EPOLLIN)
            {
                if (timerFd == eventList[i].data.fd)
                {
                    uint64_t tmpExp = 0;
                    read(timerFd, &tmpExp, sizeof(uint64_t));
                    totalExp += tmpExp;
                    printf("timer count %d  tmp %d \n" ,totalExp, tmpExp);
                }
            }
        }
        w_i--;
    }

    printf("timer count %d max %llu main %llu\n" ,totalExp, max, min);

    close(epollfd);
    return 0;
}