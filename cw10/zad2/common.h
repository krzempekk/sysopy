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
#include <poll.h>
#include <time.h>
#include <netdb.h>

#define _BSD_SOURCE

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 16

#define PING_INTERVAL 10
#define PING_TIMEOUT 5

typedef struct pollfd pollfd;

// message format: TYPE:data

typedef enum MSG_TYPE {
    LOGIN_REQUEST, // TYPE:client_name
    LOGIN_APPROVED, // just TYPE
    LOGIN_REJECTED, // TYPE:reason (name_exists or max_clients)
    PING, // just TYPE
    GAME_WAITING, // just TYPE
    GAME_FOUND, // TYPE:sign (X or O)
    GAME_MOVE, // TYPE:field (0 - 8)
    GAME_FINISHED, // TYPE:result
    LOGOUT // just TYPE
} MSG_TYPE;

typedef struct message {
    MSG_TYPE type;
    char data[MAX_MSG_LEN - 1];
} message;

typedef struct client {
    char name[MAX_MSG_LEN];
    int responding;
    struct sockaddr* addr;
} client;

void error_exit(char* msg);

message* read_message(int sock_fd);

message* read_message_from(int sock_fd, struct sockaddr* addr, socklen_t* addrlen);

message* read_message_nonblocking(int sock_fd);

void send_message(int sock_fd, MSG_TYPE type, char* content);

void send_message_to(int sock_fd, MSG_TYPE type, char* content, struct sockaddr* addr, socklen_t addrlen);

client* create_client(struct sockaddr* addr, char* name);

typedef enum FIELD {
    O = 0,
    X = 1,
    EMPTY = 100
} FIELD;

typedef struct game {
    int player_1;
    int player_2;
    FIELD board[9];
} game;

typedef enum GAME_STATUS {
    O_WIN,
    X_WIN,
    IDLE,
    DRAW
} GAME_STATUS;

game* create_new_game(int player_1, int player_2);

void make_move(game* g, int field_in, FIELD sign);

char* get_board_string(game* g);

GAME_STATUS check_game_status(game* g);

#endif //COMMON_H
