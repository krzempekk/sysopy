#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int sig_count = 0;
bool catching_signals = true;
int sender_pid;

void sigusr1_handle(int sig, siginfo_t* info, void* ucontext) {
    sig_count++;
    kill(info->si_pid, SIGUSR1);
}

void sigusr2_handle(int sig, siginfo_t* info, void* ucontext) {
    sender_pid = info->si_pid;
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
    }

    printf("Catched signals: %d\n", sig_count);
    printf("Sender PID: %d\n", sender_pid);
//
//    for(int i = 0; i < sig_count; i++) {
//        kill(sender_pid, SIGUSR1);
//    }
//    kill(sender_pid, SIGUSR2);


    return 0;
}