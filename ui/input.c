#include <stdio.h>
#include <sys/prctl.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

#include <assert.h>
#include <pthread.h>
#include <execinfo.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define BUFFSIZE 1024

//#define PCP

/* 6.2.3. 시그널 */
typedef struct _sig_ucontext {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t uc_sigmask;
} sig_ucontext_t;

#ifdef PCP
/* 6.3.1 락과 뮤텍스 */
static pthread_mutex_t global_message_mutex  = PTHREAD_MUTEX_INITIALIZER;
static char global_message[BUFFSIZE];
#endif /* PCP */

// 레퍼런스 코드
void segfault_handler(int sig_num, siginfo_t* info, void* ucontext) {
  void* array[50];
  void* caller_address;
  char** messages;
  int size, i;
  sig_ucontext_t* uc;

  uc = (sig_ucontext_t*) ucontext;

  /* Get the address at the time the signal was raised */
  caller_address = (void*) uc->uc_mcontext.rip;  // RIP: x86_64 specific     arm_pc: ARM

  fprintf(stderr, "\n");

  if (sig_num == SIGSEGV)
    printf("signal %d (%s), address is %p from %p\n", sig_num, strsignal(sig_num), info->si_addr, (void*) caller_address);
  else
    printf("signal %d (%s)\n", sig_num, strsignal(sig_num));

  size = backtrace(array, 50);
  /* overwrite sigaction with caller's address */
  array[1] = caller_address;
  messages = backtrace_symbols(array, size);

  /* skip first stack frame (points here) */
  for (i = 1; i < size && messages != NULL; ++i) {
    printf("[bt]: (%d) %s\n", i, messages[i]);
  }

  free(messages);

  exit(EXIT_FAILURE);
}

/*
 *  sensor thread
 */
void* sensor_thread(void* arg)
{
    char *s = arg;

    printf("%s", s);

    while(1)
        sleep(1);

    return 0;
}

/*
 *  command thread
 */
int toy_send(char** args);
int toy_mutex(char** args);
int toy_shell(char** args);
int toy_exit(char** args);

char* builtin_str[] = {
    "send",
    "sh",
    "exit"
};

int (*builtin_func[]) (char**) = {
    &toy_send,
    &toy_shell,
    &toy_exit
};

int toy_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int toy_send(char** args)
{
    printf("send message: %s\n", args[1]);

    return 1;
}

int toy_exit(char** args)
{
    (void)args;
    return 0;
}

int toy_shell(char** args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("toy");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("toy");
    } else
{
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int toy_execute(char** args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < toy_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return 1;
}

char* toy_read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror(": getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char** toy_split_line(char *line)
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char** tokens = malloc(bufsize* sizeof(char *));
    char* token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "toy: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize* sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "toy: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void toy_loop(void)
{
    char* line;
    char** args;
    int status;

    do {
        printf("TOY>");
        line = toy_read_line();
        args = toy_split_line(line);
        status = toy_execute(args);

        free(line);
        free(args);
    } while (status);
}

void* command_thread(void* arg)
{
    char* s = arg;

    printf("%s", s);

    toy_loop();

    return 0;
}

#ifdef PCP
// lab 9: 토이 생산자 소비자 실습
// 임시로 추가
#define MAX 30
#define NUMTHREAD 3 /* number of threads */

char buffer[BUFFSIZE];
int read_count = 0, write_count = 0;
int buflen;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
int thread_id[NUMTHREAD] = {0, 1, 2};
int producer_count = 0, consumer_count = 0;

void *toy_consumer(int *id)
{
    pthread_mutex_lock(&count_mutex);
    while (consumer_count < MAX) {
        pthread_cond_wait(&empty, &count_mutex);
        // 큐에서 하나 꺼낸다.
        printf("                           소비자[%d]: %c\n", *id, buffer[read_count]);
        read_count = (read_count + 1) % BUFFSIZE;
        fflush(stdout);
        consumer_count++;
    }
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

void *toy_producer(int *id)
{
    while (producer_count < MAX) {
        pthread_mutex_lock(&count_mutex);
        strcpy(buffer, "");
        buffer[write_count] = global_message[write_count % buflen];
        // 큐에 추가한다.
        printf("%d - 생산자[%d]: %c \n", producer_count, *id, buffer[write_count]);
        fflush(stdout);
        write_count = (write_count + 1) % BUFFSIZE;
        producer_count++;
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&count_mutex);
        sleep(rand() % 3);
    }

    return (void*)0;
}
#endif /* PCP */

int input()
{
    /* 6.2.3. 시그널 */
    struct sigaction sa;
    /* 6.2.5. 스레드 */
    int retcode;
    pthread_t cTid, sTid;

    printf("나 input 프로세스!\n");

    /* 6.2.3. 시그널 */
    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = segfault_handler;

    if(sigaction(SIGSEGV, &sa, NULL) < 0)
    {
        printf("sigaction failed!!\n");

        return 0;
    }

    /* 6.2.5. 스레드 */
    if((retcode = pthread_create(&cTid, NULL, command_thread, "command thread\n") < 0))
    {
        assert(retcode != 0);
        perror("thread create error:");
        exit(0);
    }

    if((retcode = pthread_create(&sTid, NULL, sensor_thread, "sensor thread\n") < 0))
    {
        assert(retcode != 0);
        perror("thread create error:");
        exit(0);
    }

#ifdef PCP
    /* 생산자 소비자 실습 */
    int i;
    pthread_t thread[NUMTHREAD];

    pthread_mutex_lock(&global_message_mutex);
    strcpy(global_message, "hello world!");
    buflen = strlen(global_message);
    pthread_mutex_unlock(&global_message_mutex);
    pthread_create(&thread[0], NULL, (void *)toy_consumer, &thread_id[0]);
    pthread_create(&thread[1], NULL, (void *)toy_producer, &thread_id[1]);
    pthread_create(&thread[2], NULL, (void *)toy_producer, &thread_id[2]);

    for (i = 0; i < NUMTHREAD; i++) {
        pthread_join(thread[i], NULL);
    }
#endif /* PCP */

    while (1) {
        sleep(1);
    }

    return 0;
}

int create_input()
{
    /* 6.2.2. 프로세스 관련 시스템 콜 */
    pid_t systemPid;
    const char *name = "input";

    printf("여기서 input 프로세스를 생성합니다.\n");

    systemPid = fork();
    if(!systemPid)
    {
        if(prctl(PR_SET_NAME, (unsigned long)name) < 0)
            perror("prctl() error");
        input();
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
