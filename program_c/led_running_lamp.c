#include <stdio.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>

int unknown = 0;

static int smbus_get_file_lock(u_int8_t dev_addr)
{
	char file_str[30];
	sprintf(file_str, "/root/bxqc_%u", dev_addr);
	int fd = open(file_str, O_RDWR|O_CREAT);
	if (fd < 0) {
		printf("error file name %s id %d", file_str, fd);
		return -1;
	}

	if (flock(fd, LOCK_EX) != 0) {
		printf("file lock error");
		return -1;
	}

	return fd;
}

static void hal_driver_unlock(int fd)
{

    if (fd >= 0) {
        flock(fd, LOCK_UN);
        close(fd);
    }
}

void* smbus_ptintf_file_lock(void* tid)
{
    int wh = (int) (tid);
    int sp = 0, id = 0;

    while (wh) {
        wh --;
        sp = rand()%3 + 1;
        // printf("sleep %d\n", sp);
        sleep(sp);
        id = smbus_get_file_lock(rand()%5);
        printf("unknown statr %d\n", unknown);
        unknown ++;
        sp = rand()%3 + 1;
        printf("fileid %d  id %u sleep %d\n", id, getpid(), sp);
        sleep(sp);
        printf("unknown end %d\n", unknown);
        hal_driver_unlock(id);
    }
}

void* led_ptintf_file_lock(void* tid)
{
    int wh = (int) (tid);
    int sp = 0;

    while (wh) {
        wh --;
        sp = rand()%3 + 1;
        printf("type %d\n", sp);
		char file_str[20];
		sprintf(file_str, "python3 led.py %u", sp);
        if (system(file_str) != 0) {
            printf("system error %s \n", file_str);
        }
        sleep(1);
    }
}

void* ptintf_file_lock(void* tid)
{
    int wh = (int) (tid);
    int sp = 0;

    while (wh) {
        wh --;
        sp = rand()%3 + 1;
        // printf("sleep %d\n", sp);
        sleep(sp);
        int fp = smbus_get_file_lock(1);
        printf("unknown statr %d\n", unknown);
        unknown ++;
        sp = rand()%3 + 1;
        printf("id %u sleep %d\n", getpid(), sp);
        sleep(sp);
        printf("unknown end %d\n", unknown);
        hal_driver_unlock(fp);
    }
}

void multiple_concurrency(int be, int wh, int ty)
{
    srand((unsigned)getpid());
    pthread_t td[be];
    for (int i = 0; i < be; i++) {
        if (ty == 1) {
            if (pthread_create(&td[i], NULL, ptintf_file_lock, (void *)wh) != 0) {
                printf("pthread_create error\n");
                assert(0);
            }
        } else if (ty == 0) {
            if (pthread_create(&td[i], NULL, smbus_ptintf_file_lock, (void *)wh) != 0) {
                printf("pthread_create error\n");
                assert(0);
            }
        } else if (ty == 2) {
            if (pthread_create(&td[i], NULL, led_ptintf_file_lock, (void *)wh) != 0) {
                printf("pthread_create error\n");
                assert(0);
            }
        }
    }

    for (int i = 0; i < be; i++) {
        pthread_join(td[i], NULL);
    }

    return;
}

void fork_multiple_concurrency(int fk ,int be, int wh, int ty)
{
    pid_t fpid;
    for (int i = 1; i < fk; i++) {
        fpid = fork();
        if (fpid== 0) {
            multiple_concurrency(be, wh, ty);
            return;
        }
    }

    multiple_concurrency(be, wh, ty);
    return;
}

void file_not_closed(void)
{
    sleep(10);
}

int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++) {
        printf("arg %d:%s\n", i, argv[i]);
    }

    if (argc < 2) {
        printf("hhhhhhhhhhhhhhhhhh");
        return 0;
    }
    

    if (!strcmp(argv[1], "1")) {
        file_not_closed();
    } else if (!strcmp(argv[1], "2")) {
        int whsize = atoi(argv[2]);
        int be = atoi(argv[3]);
        int fk = atoi(argv[4]);
        fork_multiple_concurrency(fk, be, whsize, 1);
    } else if (!strcmp(argv[1], "3")) {
        int whsize = atoi(argv[2]);
        int be = atoi(argv[3]);
        int fk = atoi(argv[4]);
        fork_multiple_concurrency(fk, be, whsize, 0);
    } else if (!strcmp(argv[1], "4")) {
        int whsize = atoi(argv[2]);
        int be = atoi(argv[3]);
        int fk = atoi(argv[4]);
        fork_multiple_concurrency(fk, be, whsize, 2);
    }
    
    
    return 0;
}