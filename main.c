#include <stdio.h>
#include <sys/wait.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

#include <unistd.h>
#include <assert.h>

/* 6.3.4. POSIX 메시지 큐 */
#include <mq_message.h>
#include <mqueue.h>

#define NUM_MESSAGES 10

static mqd_t watchdog_queue;
static mqd_t monitor_queue;
static mqd_t disk_queue;
static mqd_t camera_queue;

/* 6.2.3. 시그널 */
static void signalHandler(int sig)
{
    int status, savedErrno;
    pid_t childPid;

    savedErrno = errno;

    printf("signalHandler param sig : %d\n", sig);

    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("handler: Reaped child %ld - ", (long) childPid);
        //(NULL, status);
    }

    if (childPid == -1 && errno != ECHILD)
        printf("waitpid");

    printf("signalHandler now ret\n");

    errno = savedErrno;

    return;
}

/* 6.3.4. POSIX 메시지 큐 */
int create_message_queue(mqd_t *mq_fd_ptr, const char *queue_name, int num_messages, int message_size)
{
    struct mq_attr attr;
    // int mq_errno
    mqd_t mq_fd;

    printf("func : %s, name : %s, msg_num : %d\n", __func__, queue_name, num_messages);

    memset(&attr, 0, sizeof(attr));
    attr.mq_msgsize = message_size;
    attr.mq_maxmsg = num_messages;

    mq_unlink(queue_name);
    mq_fd = mq_open(queue_name, O_RDWR | O_CREAT | O_CLOEXEC, 0777, &attr);
    if(mq_fd == -1)
    {
        printf("func : %s, queue : %s already exists so try to open\n", __func__, queue_name);
        mq_fd = mq_open(queue_name, O_RDWR);
        assert(mq_fd != (mqd_t)-1);
        printf("%s queue=%s opened successfully\n", __func__, queue_name);
        printf("But the program will end.\n");
        return -1;
    }

    *mq_fd_ptr = mq_fd;
    return 0;
}

int main()
{
    /* 6.2.2. 프로세스 관련 시스템 콜 */
    pid_t spid, gpid, ipid, wpid;
    int status, savedErrno;

    (void)savedErrno;

    /* 6.2.3. 시그널 */
    // int sigCnt;
    if (signal(SIGCHLD, signalHandler) == SIG_ERR)
    {
        perror("Could not signal user signal");
        abort();

        return 0;
    }

    printf("메인 함수입니다.\n");

    /* 6.3.4. POSIX 메시지 큐 */
    int retcode;

    retcode = create_message_queue(&watchdog_queue, "/watchdog_queue", NUM_MESSAGES, sizeof(mq_msg_t));
    assert(retcode == 0);
    retcode = create_message_queue(&monitor_queue, "/monitor_queue", NUM_MESSAGES, sizeof(mq_msg_t));
    assert(retcode == 0);
    retcode = create_message_queue(&disk_queue, "/disk_queue", NUM_MESSAGES, sizeof(mq_msg_t));
    assert(retcode == 0);
    retcode = create_message_queue(&camera_queue, "/camera_queue", NUM_MESSAGES, sizeof(mq_msg_t));
    assert(retcode == 0);

    /* 6.2.2. 프로세스 관련 시스템 콜 */
    printf("메인 함수입니다.\n");
    printf("시스템 서버를 생성합니다.\n");
    spid = create_system_server();
    printf("웹 서버를 생성합니다.\n");
    wpid = create_web_server();
    printf("입력 프로세스를 생성합니다.\n");
    ipid = create_input();
    printf("GUI를 생성합니다.\n");
    gpid = create_gui();

    waitpid(spid, &status, 0);
    waitpid(gpid, &status, 0);
    waitpid(ipid, &status, 0);
    waitpid(wpid, &status, 0);

    return 0;
}
