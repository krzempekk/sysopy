#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void signal_handle(int sigint) {
    printf("Signal received\n");
}

int main(int argc, char** argv) {
    if(strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    } else if(strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, signal_handle);
    } else if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        sigset_t mask;
        sigemptyset (&mask);
        sigaddset (&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }

    raise(SIGUSR1);

    sigset_t mask;

    if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        sigpending(&mask);
        printf("Signal pending: %d\n", sigismember(&mask, SIGUSR1));
    }

    if(strcmp(argv[2], "fork") == 0) {
        pid_t child_pid = fork();

        if(child_pid == 0) {
            if(strcmp(argv[1], "pending") != 0) {
                raise(SIGUSR1);
            }
            if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
                sigpending(&mask);
                printf("Signal pending: %d\n", sigismember(&mask, SIGUSR1));
            }
        }
    } else {
        execl("./prog", "./prog", argv[1], NULL);
    }


    return 0;
}