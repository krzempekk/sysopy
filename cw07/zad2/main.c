#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "config.h"

pid_t child_pids[W_1_COUNT + W_2_COUNT + W_3_COUNT];


void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void clear() {
    for(int i = 0; i < 6; i++)
        if(sem_unlink(NAMES[i]) < 0) error_exit("cannot delete semaphor");
    if(shm_unlink("/ORDERS")) error_exit("cannot delete shared memory segment");
}

void sigint_handle(int signum) {
    printf("Konczenie dzialania...\n");
    for(int i = 0; i < W_1_COUNT + W_2_COUNT + W_3_COUNT; i++) {
        kill(child_pids[i], SIGINT);
    }
    clear();
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handle);

    sem_t* sem = sem_open(NAMES[0], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
    if(sem == SEM_FAILED) error_exit("cant create semaphor");
    sem_close(sem);

    for(int i = 1; i < 6; i++) {
        sem = sem_open(NAMES[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
        if(sem == SEM_FAILED) error_exit("cant create semaphor");
        sem_close(sem);
    }

    /*
     * Semafory
     * 0 - czy aktualnie ktos modyfikuje tablice (1 - wolne, 0 - zajete)
     * 1 - pierwszy wolny indeks w tablicy
     * 2 - pierwszy indeks zamowienia do przygotowania
     * 3 - ilosc zamowien do przygotowania
     * 4 - pierwszy indeks zamowienia do wyslania
     * 5 - ilosc zamowien do wyslania
     */

    int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd < 0) error_exit("cannot create shared memory segment");
    if(ftruncate(fd, sizeof(orders)) < 0) error_exit("cannot set memory segment size");


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