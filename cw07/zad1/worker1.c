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
#include <time.h>

#include "config.h"

const int MIN_VAL = 1;
const int MAX_VAL = 100;
int sem_id;
int shm_id;

typedef struct sembuf sembuf;


void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int random_int() {
    return rand() % (MAX_VAL - MIN_VAL + 1) + MIN_VAL;
}

int random_time() {
    return (rand() % (MAX_SLEEP - MIN_SLEEP + 1) + MIN_SLEEP) * 1000;
}

void add_order_init() {
    sembuf* sops = (sembuf*) calloc(3, sizeof(sembuf));
    sops[0].sem_num = 0;
    sops[0].sem_op = 0;
    sops[0].sem_flg = 0;

    sops[1].sem_num = 0;
    sops[1].sem_op = 1;
    sops[1].sem_flg = 0;

    sops[2].sem_num = 1;
    sops[2].sem_op = 1;
    sops[2].sem_flg = 0;

    semop(sem_id, sops, 3);
}

void add_order() {
    orders* ord = shmat(shm_id, NULL, 0);

    int index = (semctl(sem_id, 1, GETVAL, NULL) - 1) % MAX_ORDERS;
    int add_value = random_int();
    ord->values[index] = add_value;
    printf("[%d %ld] Dodalem liczbe: %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d.\n", getpid(), time(NULL), add_value, semctl(sem_id, 3, GETVAL, NULL) + 1, semctl(sem_id, 5, GETVAL, NULL));

    shmdt(ord);
}

void add_order_finalize() {
    sembuf* sops = (sembuf*) calloc(2, sizeof(sembuf));

    sops[0].sem_num = 0;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;

    sops[1].sem_num = 3;
    sops[1].sem_op = 1;
    sops[1].sem_flg = 0;

    semop(sem_id, sops, 2);
}

int main() {
    srand(time(NULL));

    key_t sem_key = ftok(getenv("HOME"), 0);
    sem_id = semget(sem_key, 0, 0);
    if(sem_id < 0) error_exit("cannot access semafors set");

    key_t shm_key = ftok(getenv("HOME"), 1);
    shm_id = shmget(shm_key, 0, 0);
    if(shm_id < 0) error_exit("cannot access shared memory segment");


    while(1) {
        usleep(random_time());
        if(semctl(sem_id, 3, GETVAL, NULL) + semctl(sem_id, 5, GETVAL, NULL) < MAX_ORDERS) {
            add_order_init();
            add_order();
            add_order_finalize();
        }
    }


    return 0;
}