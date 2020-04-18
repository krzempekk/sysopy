#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include "config.h"

key_t clients_queues[MAX_CLIENTS];
bool clients_available[MAX_CLIENTS];

int server_q;

int next_client_id() {
    int i = 0;
    while(i < MAX_CLIENTS && clients_queues[i] != -1) i++;
    return i < MAX_CLIENTS ? i + 1 : -1;
}

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void process_message(msg_buf* msg) {
    msg_buf* msg_res = (msg_buf*)malloc(sizeof(msg_buf));
    int client_id = msg->client_id;
    int client_queue_id;
    int other_client_id;

    switch(msg->m_type) {
        case INIT: ; // XDDDDDDD
            printf("INIT received\n");
            int id = next_client_id();
            if(id < 0) return;

            msg_res->m_type = id;

            client_queue_id = msgget(msg->queue_key, 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");

            clients_queues[id - 1] = msg->queue_key;
            clients_available[id - 1] = true;

            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");

            break;
        case LIST:
            printf("LIST received\n");
            strcpy(msg_res->m_text, "");

            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(clients_queues[i] != -1) {
                    sprintf(msg_res->m_text + strlen(msg_res->m_text), "ID %d, client %s\n", i + 1, clients_available[i] ? "available" : "not available");
                }
            }

            client_queue_id = msgget(clients_queues[client_id - 1], 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");

            msg_res->m_type = client_id;
            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");
            break;
        case CONNECT:
            printf("CONNECT received\n");
            other_client_id = msg->connect_client_id;

            msg_res->m_type = CONNECT;
            msg_res->queue_key = clients_queues[other_client_id - 1];
            client_queue_id = msgget(clients_queues[client_id - 1], 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");
            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");

            msg_res->m_type = CONNECT;
            msg_res->queue_key = clients_queues[client_id - 1];
            msg_res->client_id = client_id;
            client_queue_id = msgget(clients_queues[other_client_id - 1], 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");
            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");

            clients_available[client_id - 1] = false;
            clients_available[other_client_id - 1] = false;
            break;
        case DISCONNECT:
            printf("DISCONNECT received\n");
            other_client_id = msg->connect_client_id;

            msg_res->m_type = DISCONNECT;
            client_queue_id = msgget(clients_queues[other_client_id - 1], 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");
            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");

            clients_available[client_id - 1] = true;
            clients_available[other_client_id - 1] = true;
            break;
        case STOP:
            printf("STOP received\n");
            clients_queues[client_id - 1] = -1;
            clients_available[client_id - 1] = false;
            break;
        default:
            printf("Cannot recognize received message\n");
    }
}

void quit(int signum) {
    msg_buf* msg_res = (msg_buf*)malloc(sizeof(msg_buf));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        key_t queue_key = clients_queues[i];
        if(queue_key != -1) {
            msg_res->m_type = STOP;
            int client_queue_id = msgget(queue_key, 0);
            if(client_queue_id < 0) error_exit("cannot access client queue");
            if(msgsnd(client_queue_id, msg_res, MSG_SIZE, 0) < 0) error_exit("cannot send message");
            if(msgrcv(server_q, msg_res, MSG_SIZE, STOP, 0) < 0) error_exit("cannot receive message");
        }
    }
    msgctl(server_q, IPC_RMID, NULL);
    exit(0);
}

int main() {
    for(int i = 0; i < MAX_CLIENTS; i++) { clients_queues[i] = -1; }

    key_t queue_key = ftok(getenv("HOME"), SERVER_PROJ_ID);
    printf("Server queue key: %d\n", queue_key);
    server_q = msgget(queue_key, IPC_CREAT | 0666);
    printf("Server queue ID: %d\n", server_q);

    signal(SIGINT, quit);

    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    while(true) {
        if(msgrcv(server_q, msg, MSG_SIZE, -6, 0) < 0) error_exit("cannot receive message");
        process_message(msg);
    }

    return 0;
}