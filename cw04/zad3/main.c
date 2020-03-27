#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

void handle_child(int sig, siginfo_t* info, void* ucontext) {
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
    printf("Child exit code %d\n", info->si_status);
}

void handle_status(int sig, siginfo_t* info, void* ucontext) {
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
    if(info->si_code == SI_KERNEL) {
        printf("Send by kernel\n");
    } else if(info->si_code == SI_USER) {
        printf("Send by user\n");
    }
}

void handle_queue(int sig, siginfo_t* info, void* ucontext) {
    printf("Signal number %d\n", info->si_signo);
    printf("Sending PID %d\n", info->si_pid);
    printf("Signal value %d\n", info->si_value.sival_int);
}

int main(int argc, char** argv) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if(strcmp(argv[1], "child") == 0) {
        act.sa_sigaction = handle_child;
        sigaction(SIGCHLD, &act, NULL);

        pid_t child_pid = fork();

        if(child_pid == 0) {
            exit(42);
        }

        wait(NULL);
    } else if(strcmp(argv[1], "status") == 0) {
        act.sa_sigaction = handle_status;
        sigaction(SIGINT, &act, NULL);

        pause();
    } else if(strcmp(argv[1], "queue") == 0) {
        act.sa_sigaction = handle_queue;
        sigaction(SIGINT, &act, NULL);

        union sigval sval;
        sval.sival_int = 42;

        sigqueue(getpid(), SIGINT, sval);
    }

    return 0;
}