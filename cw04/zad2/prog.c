#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int main(int argc, char** argv) {
    if(strcmp(argv[1], "pending") != 0) {
        raise(SIGUSR1);
    }
    if(strcmp(argv[1], "ignore") == 0) {
        struct sigaction act;
        sigaction(SIGUSR1, NULL, &act);
        printf("Is ignored %d\n", act.sa_handler == SIG_IGN);
    } else if(strcmp(argv[1], "mask") == 0 || strcmp(argv[1], "pending") == 0) {
        sigset_t mask;
        sigpending(&mask);
        printf("Signal pending: %d\n", sigismember(&mask, SIGUSR1));
    }
    return 0;
}