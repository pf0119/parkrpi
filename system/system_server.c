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

/* 6.2.4. 타이머 */
static int timer = 0;

/* 레퍼런스 - 의미파악 필요
static void timer_expire_signal_handler()
{
    // signal 문맥에서는 비동기 시그널 안전 함수(async-signal-safe function) 사용
    // man signal 확인
    // sem_post는 async-signal-safe function
    // 여기서는 sem_post 사용
    sem_post(&global_timer_sem);
} */

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

    return ;
}

static void* timer_thread(void* not_used)
{
    (void)not_used;
    struct sigaction sigact;

    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = timer_handler;
    if(sigaction(SIGALRM, &sigact, NULL) < 0)
    {
        printf("sigaction err!!\n");
        return (void*)(-1);
        // 에러처리 어떻게 하는지 모름
    }

    set_timer(1, 1);

    return 0;
}

/* 6.2.5. 스레드 */
void* watchdog_thread(void* arg)
{
    (void)arg;
    while(1)
        sleep(1);

    return 0;
}

void* monitor_thread(void* arg)
{
    (void)arg;
    while(1)
        sleep(1);

    return 0;
}

void* disk_service_thread(void* arg)
{
    (void)arg;
    while(1)
        sleep(1);

    return 0;
}

void* camera_service_thread(void* arg)
{
    (void)arg;
    while(1)
        sleep(1);

    return 0;
}

int system_server()
{
    /* 6.2.5. 스레드 */
    int retcode;
    pthread_t wTid, mTid, dTid, cTid, tTid;

    printf("나 system_server 프로세스!\n");


    /* 6.2.5. 스레드 */
    retcode = pthread_create(&wTid, NULL, watchdog_thread, "watchdog thread\n");
    assert(retcode == 0);

    retcode = pthread_create(&mTid, NULL, monitor_thread, "monitor thread\n");
    assert(retcode == 0);

    retcode = pthread_create(&dTid, NULL, disk_service_thread, "disk service thread\n");
    assert(retcode == 0);

    retcode = pthread_create(&cTid, NULL, camera_service_thread, "camera service thread\n");
    assert(retcode == 0);

    retcode = pthread_create(&tTid, NULL, timer_thread, "timer thread\n");
    assert(retcode == 0);

    while (1) {
        sleep(1);
    }

    return 0;
}

int create_system_server()
{
    /* 6.2.2. 프로세스 관련 시스템 콜 */
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
