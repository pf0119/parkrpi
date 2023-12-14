#include <stdio.h>
#include <sys/wait.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

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

int main()
{
    /* 6.2.2. 프로세스 관련 시스템 콜 */
    pid_t spid, gpid, ipid, wpid;
    int status, savedErrno;

    (void)savedErrno;

    /* 6.2.3. 시그널 */
    int sigCnt;

    (void)sigCnt;

    if (signal(SIGCHLD, signalHandler) == SIG_ERR)
    {
        perror("Could not signal user signal");
        abort();

        return 0;
    }

    /* 2. 프로세스 관련 시스템 콜 */
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
