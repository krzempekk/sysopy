#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "config.h"

key_t queue_key;
int queue_id;
int server_queue_id;
int client_id;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void stop_command() {
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    msg->m_type = STOP;
    msg->client_id = client_id;

    if(msgsnd(server_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");
    if(msgctl(queue_id, IPC_RMID, NULL) < 0) error_exit("cannot delete queue");

    msgctl(queue_id, IPC_RMID, NULL);
    exit(0);
}

void chat_mode(int other_id, int other_queue_id) {
    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    while(true) {
        printf("Enter message or DISCONNECT: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        if(msgrcv(queue_id, msg, MSG_SIZE, STOP, IPC_NOWAIT) >= 0) {
            printf("STOP from server, quitting...\n");
            stop_command();
        }

        if(msgrcv(queue_id, msg, MSG_SIZE, DISCONNECT, IPC_NOWAIT) >= 0) {
            printf("Disconnecting...\n");
            break;
        }

        while(msgrcv(queue_id, msg, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
            printf("[%d]: %s\n", other_id, msg->m_text);
        }

        if(strcmp(cmd, "DISCONNECT") == 0) {
            msg->m_type = DISCONNECT;
            msg->client_id = client_id;
            msg->connect_client_id = other_id;
            if(msgsnd(server_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");
            break;
        } else if(strcmp(cmd, "") != 0) {
            msg->m_type = CONNECT;
            strcpy(msg->m_text, cmd);
            if(msgsnd(other_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");
        }
    }
}


int init_connection() {
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    msg->m_type = INIT;
    msg->queue_key = queue_key;
    if(msgsnd(server_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");

    msg_buf* msg_rcv = (msg_buf*)malloc(sizeof(msg_buf));
    if(msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0) < 0) error_exit("cannot receive message");

    int client_id = msg_rcv->m_type;
    return client_id;
}

void connect_command(int id) {
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    msg->m_type = CONNECT;
    msg->client_id = client_id;
    msg->connect_client_id = id;

    if(msgsnd(server_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");

    msg_buf* msg_rcv = (msg_buf*)malloc(sizeof(msg_buf));
    if(msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0) < 0) error_exit("cannot receive message");

    key_t other_queue_key = msg_rcv->queue_key;
    int other_queue_id = msgget(other_queue_key, 0);
    if(other_queue_id < 0) error_exit("cannot access other client queue");

    chat_mode(id, other_queue_id);
}

void list_command() {
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));
    msg->m_type = LIST;
    msg->client_id = client_id;
    if(msgsnd(server_queue_id, msg, MSG_SIZE, 0) < 0) error_exit("cannot send message");

    msg_buf* msg_rcv = (msg_buf*)malloc(sizeof(msg_buf));
    if(msgrcv(queue_id, msg_rcv, MSG_SIZE, 0, 0) < 0) error_exit("cannot receive message");
    printf("%s\n", msg_rcv->m_text);
}

void quit(int signum) {
    stop_command();
}

void check_server_message() {
    msg_buf* msg = (msg_buf*)malloc(sizeof(msg_buf));

    if(msgrcv(queue_id, msg, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
        if(msg->m_type == STOP) {
            printf("STOP from server, quitting...\n");
            stop_command();
        } else if(msg->m_type == CONNECT) {
            printf("Connecting to client %d...\n", msg->client_id);
            int other_queue_id = msgget(msg->queue_key, 0);
            if(other_queue_id < 1) error_exit("cannot access other client queue");
            chat_mode(msg->client_id, other_queue_id);
        }
    }
}


int random_number() {
    return rand() % 255 + 1;
}

int main() {
    srand(time(NULL));

    queue_key = ftok(getenv("HOME"), random_number());
    printf("Queue key: %d\n", queue_key);

    // przy tworzeniu trzeba ustawic dostep
    queue_id = msgget(queue_key, IPC_CREAT | 0666);
    if(queue_id < 0) error_exit("cannot create queue");
    printf("Queue ID: %d\n", queue_id);

    key_t server_key = ftok(getenv("HOME"), SERVER_PROJ_ID);
    server_queue_id = msgget(server_key, 0);
    if(server_queue_id < 0) error_exit("cannot access server queue");
    printf("Server queue ID: %d\n", server_queue_id);

    client_id = init_connection();
    printf("ID received: %d\n", client_id);

    signal(SIGINT, quit);

    char* cmd = NULL;
    size_t len = 0;
    ssize_t read = 0;
    while(true) {
        printf("Enter command: ");
        read = getline(&cmd, &len, stdin);
        cmd[read - 1] = '\0';

        check_server_message();

        if(strcmp(cmd, "") == 0) {
            continue;
        }

        char* tok = strtok(cmd, " ");
        if(strcmp(tok, "LIST") == 0) {
            printf("LIST command\n");
            list_command();
        } else if(strcmp(tok, "CONNECT") == 0) {
            tok = strtok(NULL, " ");
            int id = atoi(tok);
            connect_command(id);
        } else if(strcmp(tok, "STOP") == 0) {
            printf("STOP command\n");
            stop_command();
        } else {
            printf("Unrecognized command: %s\n", cmd);
        }
    }

    return 0;
}