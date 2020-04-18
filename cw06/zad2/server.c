#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>

#include "config.h"

char* clients_queues[MAX_CLIENTS];
bool clients_available[MAX_CLIENTS];

mqd_t server_q;

int next_client_id() {
    int i = 0;
    while(i < MAX_CLIENTS && clients_queues[i] != NULL) i++;
    return i < MAX_CLIENTS ? i + 1 : -1;
}

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void quit(int signum) {
    char* msg_res = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients_queues[i] != NULL) {
            mqd_t client_queue_desc = mq_open(clients_queues[i], O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");
            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, STOP) < 0) error_exit("cannot send message");
            if(mq_receive(server_q, msg_res, MAX_MSG_LEN, NULL) < 0) error_exit("cannot receive message");
            if(mq_close(client_queue_desc) < 0) error_exit("cannot close queue");
        }
    }

    if(mq_close(server_q) < 0) error_exit("cannot close queue");
    if(mq_unlink(SERVER_QUEUE_NAME) < 0) error_exit("cannot delete queue");
    exit(0);
}

void process_message(char* msg, int prio) {
    char* msg_res = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    mqd_t client_queue_desc;
    int client_id;
    int other_client_id;

    switch(prio) {
        case INIT: ;
            printf("INIT received\n");

            int id = next_client_id();
            if(id < 0) return;

            client_queue_desc = mq_open(msg, O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");

            clients_available[id - 1] = true;
            clients_queues[id - 1] = (char*)calloc(NAME_LEN, sizeof(char));
            strcpy(clients_queues[id - 1], msg);

            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, id) < 0) error_exit("cannot send message");
            if(mq_close(client_queue_desc) < 0) error_exit("cannot close queue");
            break;
        case LIST:
            printf("LIST received\n");

            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(clients_queues[i] != NULL) {
                    sprintf(msg_res + strlen(msg_res), "ID %d, client %s\n", i + 1, clients_available[i] ? "available" : "not available");
                }
            }

            client_id = (int) msg[0];
            client_queue_desc = mq_open(clients_queues[client_id - 1], O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");

            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, LIST) < 0) error_exit("cannot send message");
            if(mq_close(client_queue_desc) < 0) error_exit("cannot close queue");
            break;
        case CONNECT:
            printf("CONNECT received\n");
            client_id = (int) msg[0];
            other_client_id = (int) msg[1];

            client_queue_desc = mq_open(clients_queues[client_id - 1], O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");
            msg_res[0] = other_client_id;
            strcat(msg_res, clients_queues[other_client_id - 1]);
            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, CONNECT) < 0) error_exit("cannot send message");

            memset(msg_res, 0, strlen(msg_res));
            client_queue_desc = mq_open(clients_queues[other_client_id - 1], O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");
            msg_res[0] = client_id;
            strcat(msg_res, clients_queues[client_id - 1]);
            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, CONNECT) < 0) error_exit("cannot send message");
            if(mq_close(client_queue_desc) < 0) error_exit("cannot close queue");

            clients_available[client_id - 1] = false;
            clients_available[other_client_id - 1] = false;
            break;
        case DISCONNECT:
            printf("DISCONNECT received\n");
            client_id = (int) msg[0];
            other_client_id = (int) msg[1];

            client_queue_desc = mq_open(clients_queues[other_client_id - 1], O_RDWR);
            if(client_queue_desc < 0) error_exit("cannot access client queue");
            if(mq_send(client_queue_desc, msg_res, MAX_MSG_LEN, DISCONNECT) < 0) error_exit("cannot send message");
            if(mq_close(client_queue_desc) < 0) error_exit("cannot close queue");

            clients_available[client_id - 1] = true;
            clients_available[other_client_id - 1] = true;
            break;
        case STOP:
            printf("STOP received\n");
            client_id = (int) msg[0];

            clients_available[client_id - 1] = false;
            clients_queues[client_id - 1] = NULL;
            break;
        default:
            printf("Cannot recognize received message\n");
    }
}

int main() {
    for(int i = 0; i < MAX_CLIENTS; i++) { clients_queues[i] = NULL; }

    server_q = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(server_q < 0) error_exit("cannot create queue");

    signal(SIGINT, quit);
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    unsigned int prio;

    while(true) {
        if(mq_receive(server_q, msg, MAX_MSG_LEN, &prio) < 0) error_exit("cannot receive message");
        process_message(msg, prio);
    }

    return 0;
}