#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int sig_count = 0;
bool catching_signals = true;
int sender_pid;

void sigusr1_handle() {
    sig_count++;
}

void sigusr2_handle(int sig, siginfo_t* info, void* ucontext) {
    sender_pid = info->si_pid;
    catching_signals = false;
}

int main(int argc, char** argv) {
    printf("Catcher PID: %d\n", getpid());

    if(strcmp(argv[1], "sigrt") == 0) {
        signal(SIGRTMIN, sigusr1_handle);

//        sigset_t mask;
//        sigfillset(&mask);
//        sigdelset(&mask, SIGRTMIN);
//        sigdelset(&mask, SIGRTMIN+1);
//        sigprocmask(SIG_BLOCK, &mask, NULL);

        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr2_handle;
        sigaction(SIGRTMIN+1, &act, NULL);
    } else {
        signal(SIGUSR1, sigusr1_handle);

//        sigset_t mask;
//        sigfillset(&mask);
//        sigdelset(&mask, SIGUSR1);
//        sigdelset(&mask, SIGUSR2);
//        sigprocmask(SIG_BLOCK, &mask, NULL);

        struct sigaction act;
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = sigusr2_handle;
        sigaction(SIGUSR2, &act, NULL);
    }


    while(catching_signals) {
        pause();
    }

    printf("Catched signals: %d\n", sig_count);
    printf("Sender PID: %d\n", sender_pid);

    if(strcmp(argv[1], "kill") == 0) {
        for(int i = 0; i < sig_count; i++) {
            kill(sender_pid, SIGUSR1);
        }
        kill(sender_pid, SIGUSR2);
    } else if(strcmp(argv[1], "sigqueue") == 0) {
        union sigval sval;
        for(int i = 0; i < sig_count; i++) {
            sval.sival_int = i;
            sigqueue(sender_pid, SIGUSR1, sval);
        }
        sval.sival_int = sig_count;
        sigqueue(sender_pid, SIGUSR2, sval);
    } else if(strcmp(argv[1], "sigrt") == 0) {
        for(int i = 0; i < sig_count; i++) {
            kill(sender_pid, SIGRTMIN);
        }
        kill(sender_pid, SIGRTMIN+1);
    }


    return 0;
}