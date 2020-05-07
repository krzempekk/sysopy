#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>

int clients_processed = 0;

int clients_count;
int chairs_count;
int chairs_taken = 0;
int barber_sleeping = 0;

pthread_t barber_chair;
pthread_t* chairs;
int next_free_chair = 0;
int next_chair = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int random_int() {
    return rand() % 3 + 1;
}

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void* barber(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        if(chairs_taken == 0) {
            printf("Golibroda: ide spac\n");
            barber_sleeping = 1;
            pthread_cond_wait(&cond, &mutex);
            barber_sleeping = 0;
        } else {
            chairs_taken--;
            barber_chair = chairs[next_chair];
            next_chair = (next_chair + 1) % chairs_count;
        }
        printf("Golibroda: czeka %d klientow, gole klienta %ld\n", chairs_taken, barber_chair);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        sleep(random_int() * 3);

        pthread_mutex_lock(&mutex);
        pthread_cancel(barber_chair);
        barber_chair = 0;
        clients_processed++;
        if(clients_processed == clients_count) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit((void *) 0);
}

void* client(void* arg) {
    pthread_t id = pthread_self();
    while(1) {
        pthread_mutex_lock(&mutex);
        if(barber_sleeping) {
            barber_chair = id;
            pthread_cond_broadcast(&cond);
            printf("Klient: budze golibrode; %ld\n", id);
            pthread_mutex_unlock(&mutex);
            break;
        } else if(chairs_taken < chairs_count) {
            chairs[next_free_chair] = id;
            next_free_chair = (next_free_chair + 1) % chairs_count;
            chairs_taken++;
            printf("Klient: poczekalnia, wolne miejsca %d; %ld\n", chairs_count - chairs_taken, id);
            pthread_mutex_unlock(&mutex);
            break;
        }
        printf("Klient: zajete; %ld\n", id);
        pthread_mutex_unlock(&mutex);
        sleep(random_int() * 3);
    }

    pthread_exit((void *) 0);
}


int main(int argc, char** argv) {
    srand(time(NULL));
    if(argc < 3) error_exit("not enough arguments");

    if(pthread_mutex_init(&mutex, NULL) != 0) error_exit("cannot init mutex");

    chairs_count = atoi(argv[1]);
    clients_count = atoi(argv[2]);

    chairs = (pthread_t*) calloc(chairs_count, sizeof(pthread_t));

    pthread_t* thread_ids = (pthread_t*) calloc(clients_count + 1, sizeof(pthread_t));
    pthread_create(&thread_ids[0], NULL, barber, NULL);
    for(int i = 0; i < clients_count; i++) {
        sleep(random_int());
        pthread_create(&thread_ids[i + 1], NULL, client, NULL);
    }

    for(int i = 0; i < clients_count + 1; i++) {
        if(pthread_join(thread_ids[i], NULL) > 0) error_exit("thread error");
    }
    return 0;
}