#include <stdio.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int create_web_server()
{
    pid_t systemPid;

    const char *name = "web_server";

    printf("여기서 Web Server 프로세스를 생성합니다.\n");

    systemPid = fork();
    if(!systemPid)
    {
        if(execl("/usr/local/bin/filebrowser", "filebrowser", "-p", "8080", (char *)NULL))
            printf("execfailed\n");
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
