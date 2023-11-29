#include <stdio.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

#include <wait.h>

int create_gui()
{
    pid_t systemPid;
    const char *name = "gui";

    printf("여기서 GUI 프로세스를 생성합니다.\n");

    sleep(3);
    /* fork + exec 를 이용하세요 */
    /* exec으로 google-chrome-stable을 실행 하세요. */

    systemPid = fork();
    if(!systemPid)
    {
        if(execl("/usr/bin/google-chrome-stable", "google-chrome-stable", "http://localhost:8080", NULL))
            printf("execl() failed!!\n");
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
