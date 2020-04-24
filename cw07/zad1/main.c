#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "config.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

pid_t child_pids[W_1_COUNT + W_2_COUNT + W_3_COUNT];

int sem_id;
int shm_id;

void clear() {
    semctl(sem_id, 0, IPC_RMID, NULL);
    shmctl(shm_id, IPC_RMID, NULL);
}

void sigint_handle(int signum) {
    printf("Konczenie dzialania...\n");
    for(int i = 0; i < W_1_COUNT + W_2_COUNT + W_3_COUNT; i++) {
        kill(child_pids[i], SIGINT);
    }
    clear();
    exit(0);
}

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int main() {
    signal(SIGINT, sigint_handle);

    key_t sem_key = ftok(getenv("HOME"), 0);
    sem_id = semget(sem_key, 6, IPC_CREAT | 0666);
    if(sem_id < 0) error_exit("cannot create semafors set");
    /*
     * Semafory
     * 0 - czy aktualnie ktos modyfikuje tablice (0 - wolne, 1 - zajete)
     * 1 - pierwszy wolny indeks w tablicy
     * 2 - pierwszy indeks zamowienia do przygotowania
     * 3 - ilosc zamowien do przygotowania
     * 4 - pierwszy indeks zamowienia do wyslania
     * 5 - ilosc zamowien do wyslania
     */

    union semun arg;
    arg.val = 0;

    for(int i = 0; i < 4; i++) {
        semctl(sem_id, i, SETVAL, arg);
    }

    key_t shm_key = ftok(getenv("HOME"), 1);
    shm_id = shmget(shm_key, sizeof(struct orders), IPC_CREAT | 0666);
    if(shm_id < 0) error_exit("cannot create shared memory segment");


    for(int i = 0; i < W_1_COUNT; i++) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            execlp("./worker1", "worker1", NULL);
        }
        child_pids[i] = child_pid;
    }

    for(int i = 0; i < W_2_COUNT; i++) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            execlp("./worker2", "worker2", NULL);
        }
        child_pids[i + W_1_COUNT] = child_pid;
    }

    for(int i = 0; i < W_3_COUNT; i++) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            execlp("./worker3", "worker3", NULL);
        }
        child_pids[i + W_1_COUNT + W_2_COUNT] = child_pid;
    }

    for(int i = 0; i < W_1_COUNT + W_2_COUNT + W_3_COUNT; i++) {
        wait(NULL);
    }

    clear();

    return 0;
}