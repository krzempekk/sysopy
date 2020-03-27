#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

bool catching_signals = true;
int received_count = 0;
int last_index;

void sigusr1_handle() {
    received_count++;
}

void sigusr2_handle() {
    catching_signals = false;
}

void sigusr2_handle_sigaction(int sig, siginfo_t* info, void* ucontext) {
    catching_signals = false;
    last_index = info->si_value.sival_int;
}


int main(int argc, char** argv) {
    int pid = atoi(argv[1]);
    int sig_count = atoi(argv[2]);

    if(strcmp(argv[3], "kill") == 0) {

        for(int i = 0; i < sig_count; i++) {
            kill(pid, SIGUSR1);
        }
        kill(pid, SIGUSR2);

        signal(SIGUSR1, sigusr1_handle);
        signal(SIGUSR2, sigusr2_handle);

    } else if(strcmp(argv[3], "sigqueue") == 0) {

        union sigval sval;
        for(int i = 0; i < sig_count; i++) {
            sval.sival_int = i;
            sigqueue(pid, SIGUSR1, sval);
        }
        sval.sival_int = sig_count;
        sigqueue(pid, SIGUSR2, sval);

        signal(SIGUSR1, sigusr1_handle);
        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr2_handle_sigaction;
        sigaction(SIGUSR2, &act, NULL);

    } else if(strcmp(argv[3], "sigrt") == 0) {

        for(int i = 0; i < sig_count; i++) {
            kill(pid, SIGRTMIN);
        }
        kill(pid, SIGRTMIN+1);

        signal(SIGRTMIN, sigusr1_handle);
        signal(SIGRTMIN+1, sigusr2_handle);
    }

    while(catching_signals) {
        pause();
    }

    if(strcmp(argv[3], "sigqueue") == 0) {
        printf("Received %d signals, expected %d. Actually %d signals was sent.\n", received_count, sig_count, last_index);
    } else {
        printf("Received %d signals, expected %d.\n", received_count, sig_count);
    }

    return 0;
}