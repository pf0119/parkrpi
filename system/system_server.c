#include <stdio.h>
#include <sys/prctl.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

static int timer = 0;

static void timer_handler()
{
    timer++;
    //printf("one tick : %d!!\n", timer);
    return;
}

void set_timer(long sec_delay, long usec_delay)
{
    /*
    struct itimerval itimer_val = {
        .it_interval = { .tv_sec = sec_delay, .tv_usec = usec_delay },
        .it_value = { .tv_sec = sec_delay, .tv_usec = usec_delay }
    };
    */
    struct itimerval itimer_val;

    itimer_val.it_interval.tv_sec = sec_delay;
    itimer_val.it_interval.tv_usec = usec_delay;
    itimer_val.it_value.tv_sec = sec_delay;
    itimer_val.it_value.tv_usec = usec_delay;

    setitimer(ITIMER_REAL, &itimer_val, (struct itimerval*)0);

    return;
}


int system_server()
{
    /* 4. 타이머 */
    struct sigaction sigact;

    printf("나 system_server 프로세스!\n");

    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = timer_handler;
    if(sigaction(SIGALRM, &sigact, NULL) < 0)
    {
        printf("sigaction err!!\n");
        return -1;
    }

    set_timer(1, 0);

    while (1) {
        sleep(1);
    }

    return 0;
}

int create_system_server()
{
    /* 2. 프로세스 관련 시스템 콜 */
    pid_t systemPid;
    const char *name = "system_server";

    printf("여기서 시스템 프로세스를 생성합니다.\n");

    systemPid = fork();
    if(!systemPid)
    {
        if(prctl(PR_SET_NAME, (unsigned long)name) < 0)
            perror("prctl() error");

        system_server();
    }
    else if(systemPid > 0)
    {
        ;
    }
    else
    {
        printf("fork() failed!!\n");
    }

    return 0;
}
