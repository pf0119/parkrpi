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

#include <semaphore.h>
#include <camera_HAL.h>

#include <mqueue.h>
#include <mq_message.h>

#include <shared_memory.h>

#define BUFSIZE 1024

pthread_mutex_t system_loop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  system_loop_cond  = PTHREAD_COND_INITIALIZER;
bool            system_loop_exit = false;

/* 6.3.2. POSIX 메시지 큐 */
static mqd_t watchdog_queue;
static mqd_t monitor_queue;
static mqd_t disk_queue;
static mqd_t camera_queue;

/* 6.2.4. 타이머 */
static int timer = 0;

/* 6.4.1. POSIX 세마포어 & 공유메모리 */
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t global_timer_sem;
static bool global_timer_stopped;

/* 6.4.2. 시스템 V 메시지 큐 & 세마포어 & 공유 메모리 */
static shm_sensor_t *the_sensor_info = NULL;

static void timer_expire_signal_handler()
{
    /* 6.4.1. POSIX 세마포어 & 공유메모리 */
    // signal 문맥에서는 비동기 시그널 안전 함수(async-signal-safe function) 사용
    // man signal 확인
    // sem_post는 async-signal-safe function
    // 여기서는 sem_post 사용
    sem_post(&global_timer_sem);
}

static void system_timeout_handler()
{
    /* 6.4.1. POSIX 세마포어 & 공유메모리 */
    // 여기는 signal hander가 아니기 때문에 안전하게 mutex lock 사용 가능
    // timer 변수는 전역 변수이므로 뮤텍스를 사용한다
    pthread_mutex_lock(&timer_mutex);
    timer++;
    //printf("timer: %d\n", timer);
    pthread_mutex_unlock(&timer_mutex);
    return;
}

void set_periodic_timer(long sec_delay, long usec_delay)
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

int posix_sleep_ms(unsigned int timeout_ms)
{
    struct timespec sleep_time;

    sleep_time.tv_sec = timeout_ms / MILLISEC_PER_SECOND;
    sleep_time.tv_nsec = (timeout_ms % MILLISEC_PER_SECOND) * (NANOSEC_PER_USEC * USEC_PER_MILLISEC);

    return nanosleep(&sleep_time, NULL);
}

static void* timer_thread(void* not_used)
{
    (void)not_used;
    struct sigaction sigact;
    int ret;

    //printf("%s!!\n", __func__);

    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = timer_expire_signal_handler;
    ret = sigaction(SIGALRM, &sigact, NULL);
    if(ret == -1)
    {
        printf("sigaction err!!\n");
        assert(ret == 0);
    }

    set_periodic_timer(1, 1);

    /* 6.4.1. POSIX 세마포어 & 공유메모리 */
    while(!global_timer_stopped)
    {
        ret = sem_wait(&global_timer_sem);
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            else
                assert(ret == 0);
        }

        sleep(1);
        system_timeout_handler();
    }

    return 0;
}

/* 6.2.5. 스레드 */
void* watchdog_thread(void* arg)
{
    int ret;
    mq_msg_t msg;

    printf("%s", (char*)arg);

    while(1)
    {
        /* 6.3.2. POSIX 메시지 큐 */
        ret = (int)mq_receive(watchdog_queue, (void*)&msg, sizeof(mq_msg_t), 0);
        assert(ret >= 0);
        printf("%s : 메시지가 도착했습니다.\n", __func__);
        printf("msg.type : %d\n", msg.msg_type);
        printf("msg.param1 : %d\n", msg.param1);
        printf("msg.param2 : %d\n", msg.param2);
    }

    return 0;
}

#define SENSOR_DATA 1

void* monitor_thread(void* arg)
{
    int ret;
    mq_msg_t msg;
    int shmid;

    printf("%s", (char*)arg);

    while(1)
    {
        /* 6.3.2. POSIX 메시지 큐 */
        ret = (int)mq_receive(monitor_queue, (void*)&msg, sizeof(mq_msg_t), 0);
        assert(ret >= 0);
        printf("%s : 메시지가 도착했습니다.\n", __func__);
        printf("msg.type : %d\n", msg.msg_type);
        printf("msg.param1 : %d\n", msg.param1);
        printf("msg.param2 : %d\n", msg.param2);
        /* 6.4.2. 시스템 V 메시지 큐 & 세마포어 & 공유 메모리 */
        if(msg.msg_type == SENSOR_DATA)
        {
            shmid = msg.param1;
            the_sensor_info = toy_shm_attach(shmid);
            printf("sensor temp: %d\n", the_sensor_info->temp);
            printf("sensor info: %d\n", the_sensor_info->press);
            printf("sensor humidity: %d\n", the_sensor_info->humidity);
            toy_shm_detach(the_sensor_info);
        }
    }

    return 0;
}

void* disk_service_thread(void* arg)
{
    char buf[BUFSIZE];
    FILE* file;

    int ret;
    mq_msg_t msg;

    printf("%s\n", (char*)arg);

    while(1)
    {
        // popen으로 shell을 실행하면 성능과 보안 문제가 있습니다.
        // 향후 파일 관련 시스템 콜 시간에 위 코드 개선합니다.
        file = popen("df -h ./", "r");
        while(fgets(buf, BUFSIZE, file))
        {
            printf("%s", buf);
        }
        pclose(file);

        sleep(10);

        /* 6.3.2. POSIX 메시지 큐 */
        ret = (int)mq_receive(disk_queue, (void*)&msg, sizeof(mq_msg_t), 0);
        assert(ret >= 0);
        printf("%s : 메시지가 도착했습니다.\n", __func__);
        printf("msg.type : %d\n", msg.msg_type);
        printf("msg.param1 : %d\n", msg.param1);
        printf("msg.param2 : %d\n", msg.param2);
    }

    return 0;
}

#define CAMERA_TAKE_PICTURE 1

void* camera_service_thread(void* arg)
{
    (void)arg;
    char *s = arg;

    printf("%s", s);

    toy_camera_open();

    /* 6.3.2. POSIX 메시지 큐 */
    int ret;
    mq_msg_t msg;

    printf("%s", (char*)arg);

    while(1)
    {
        ret = (int)mq_receive(camera_queue, (void*)&msg, sizeof(mq_msg_t), 0);
        assert(ret >= 0);
        printf("%s : 메시지가 도착했습니다.\n", __func__);
        printf("msg.type : %d\n", msg.msg_type);
        printf("msg.param1 : %d\n", msg.param1);
        printf("msg.param2 : %d\n", msg.param2);
        if(msg.msg_type == CAMERA_TAKE_PICTURE)
        {
            printf("received %d!! toy_camera_take_picture() will be called!!\n", msg.msg_type);
            toy_camera_take_picture();
        }
    }
    return 0;
}

/* 6.3.1. 락과 뮤텍스 */
void signal_exit(void)
{
    /* 6.3.2 멀티 스레드 동기화 & C와 C++ 연동, 종료 메세지를 보내도록 */
    pthread_mutex_lock(&system_loop_mutex);
    system_loop_exit = true;
    // pthread_cond_broadcast(&system_loop_cond);
    pthread_cond_signal(&system_loop_cond);
    pthread_mutex_unlock(&system_loop_mutex);
}

int system_server()
{
    /* 6.2.5. 스레드 */
    int retcode;
    pthread_t wTid, mTid, dTid, cTid, tTid;

    printf("나 system_server 프로세스!\n");

    /* 6.3.4. POSIX 메시지 큐 */
    watchdog_queue = mq_open("/watchdog_queue", O_RDWR);
    assert(watchdog_queue != -1);
    monitor_queue = mq_open("/monitor_queue", O_RDWR);
    assert(monitor_queue != -1);
    disk_queue = mq_open("/disk_queue", O_RDWR);
    assert(disk_queue != -1);
    camera_queue = mq_open("/camera_queue", O_RDWR);
    assert(camera_queue != -1);

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

    printf("system init done.  waiting...");

    // 여기서 cond wait로 대기한다. 10초 후 알람이 울리면 <== system 출력
    pthread_mutex_lock(&system_loop_mutex);
    while (system_loop_exit == false) {
        pthread_cond_wait(&system_loop_cond, &system_loop_mutex);
    }
    pthread_mutex_unlock(&system_loop_mutex);

    printf("<== system\n");

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
