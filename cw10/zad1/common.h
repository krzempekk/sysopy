#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <endian.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>

#define _BSD_SOURCE

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 16

// message format: TYPE:data

typedef enum MSG_TYPE {
    LOGIN_REQUEST = 0, // 0:client_name
    LOGIN_APPROVED = 1, // just 1
    LOGIN_REJECTED = 2, // 2:reason (name exists or max client number achieved)
    PING = 3, // just 3
    GAME_WAITING = 4, // just 4
    GAME_FOUND = 5, // 5:sign (X or O)
    GAME_MOVE = 6, // 6:field
    GAME_FINISHED = 7, // just 7
    LOGOUT = 8 // just 8
} MSG_TYPE;

typedef struct message {
    MSG_TYPE type;
    char data[MAX_MSG_LEN - 1];
} message;

void error_exit(char* msg);

message* read_message(int sock_fd);

void send_message(int sock_fd, MSG_TYPE type, char* content);

#endif //COMMON_H
