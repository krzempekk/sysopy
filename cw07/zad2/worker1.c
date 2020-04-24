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
#include <time.h>

#include "config.h"

const int MIN_VAL = 1;
const int MAX_VAL = 100;

sem_t* semaphors[6];
int shm_fd;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void sigint_handle(int signum) {
    for(int i = 0; i < 6; i++) {
        if(sem_close(semaphors[i]) < 0) error_exit("cannot close semaphor");
    }
    exit(0);
}

int random_int() {
    return rand() % (MAX_VAL - MIN_VAL + 1) + MIN_VAL;
}

int random_time() {
    return (rand() % (MAX_SLEEP - MIN_SLEEP + 1) + MIN_SLEEP) * 1000;
}

int sem_value(int index) {
    int value;
    sem_getvalue(semaphors[index], &value);
    return value;
}

void add_order_init() {
    sem_wait(semaphors[0]);
    sem_post(semaphors[1]);
}

void add_order() {
    orders* ord = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ord == (void *) -1) error_exit("cannot map memory");

    int add_value = random_int();
    int index = (sem_value(1) - 1) % MAX_ORDERS;
    ord->values[index] = add_value;
    printf("[%d %ld] Dodalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d.\n", getpid(), time(NULL), add_value, sem_value(3) + 1, sem_value(5));

    if(munmap(ord, sizeof(orders)) < 0) error_exit("cannot unmap memory");
}

void add_order_finalize() {
    sem_post(semaphors[0]);
    sem_post(semaphors[3]);
}

int main() {
    srand(time(NULL));

    signal(SIGINT, sigint_handle);

    for(int i = 0; i < 6; i++) {
        semaphors[i] = sem_open(NAMES[i], O_RDWR);
        if(semaphors[i] < 0) error_exit("cannot access semaphor");
    }

    shm_fd = shm_open(SHM_NAME, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if(shm_fd < 0) error_exit("cannot access shared memory segment");

    while(1) {
        usleep(random_time());

        if(sem_value(3) + sem_value(5) < MAX_ORDERS) {
            add_order_init();
            add_order();
            add_order_finalize();
        }
    }

    return 0;
}