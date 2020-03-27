#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

bool stopped = false;

void sigint_handle(int signum) {
    printf("Odebrano sygnal SIGINT\n");
    exit(0);
}

void sigtstp_handle() {
    if(!stopped) {
        printf("Oczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
        stopped = true;
    } else {
        stopped = false;
    }
}

int main() {
    struct sigaction act;
    act.sa_handler = sigtstp_handle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    signal(SIGINT, sigint_handle);
    sigaction(SIGTSTP, &act, NULL);

    while(true) {
        if(!stopped) {
            system("ls");
            sleep(1);
        } else {
            pause();
        }
    }

    return 0;
}