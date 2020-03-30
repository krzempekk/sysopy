#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int sig_count = 0;
bool catching_signals = true;
pid_t pid;

void sigusr1_handle(int sig, siginfo_t* info, void* ucontext) {
    sig_count++;
    pid = info->si_pid;
}

void sigusr2_handle(int sig, siginfo_t* info, void* ucontext) {
    catching_signals = false;
}

int main(int argc, char** argv) {
    printf("Catcher PID: %d\n", getpid());

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigusr1_handle;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigusr2_handle;
    sigaction(SIGUSR2, &act, NULL);

    while(catching_signals) {
        pause();
        usleep(1000);
        kill(pid, SIGUSR1);
    }

    printf("Catched signals: %d\n", sig_count);

    return 0;
}