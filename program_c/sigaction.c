#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

void xqc_001(void)
{
    printf("atexit\n");
}

void* sf_kill_signal(void* tid)
{
    int signum = (int)tid;

    while(1) {
        sleep(1);
        printf("dp exit kill interval %ums, signum %d\n", 1000, signum);
        kill(getpid(), SIGABRT);
    }

    return NULL;
}

void exit_loop_kill_signal(int signum)
{
    pthread_t thread;
    pthread_create(&thread, NULL, sf_kill_signal, (void*)signum);
    return;
}

void xqc_sig(int signum)
{
    int pid = getpid();
    signal(signum, SIG_DFL);
    printf("xqc:%d,pid:%d\n", signum, pid);
    exit_loop_kill_signal(signum);

    // struct sigaction action;
    // action.sa_flags = 0;
    // action.sa_handler = SIG_DFL;
    // sigaction(signum, &action, NULL);

    while(1){
        sleep(1);
    }

    // exit(signum);
    kill(pid, signum);
}


int main()
{
    // struct sigaction action;
    // action.sa_flags = 0;
    // action.sa_handler = xqc_sig;
    // atexit(xqc_001);

    // printf("SA_INTERRUPT:%d\n", SA_INTERRUPT);
    // printf("SA_RESTART:%d\n", SA_RESTART);
    // printf("SA_SIGINFO:%d\n", SA_SIGINFO);
    // printf("SIG_DFL:%p\n", SIG_DFL);

    // if (sigaction(SIGBUS, &action, NULL) == -1) {
    //     perror("sigsegv: sigaction");
    //     _exit(1);
    // }

    printf("SIGBUS:%d\n", SIGBUS);
    printf("SIGTERM:%d\n", SIGTERM);

    
    int xqc_int;
    scanf("%d", &xqc_int);
    abort();
    printf("#######################end############ %d\n", xqc_int);
    while (1) { sleep(10);}
    return 0;
}