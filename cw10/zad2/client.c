#include "common.h"

int is_local_connection;
char* name;
char* server_address;
int port_number;

int server_sock_fd;

pthread_t input_thread;
char input[2];

struct sockaddr_un server_sock;

void open_connection() {
    if(is_local_connection) {
//        struct sockaddr_un server_sock;

        server_sock.sun_family = AF_UNIX;
        strcpy(server_sock.sun_path, server_address);

        if((server_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) error_exit("socket");

        int optval = 1;
        if(setsockopt(server_sock_fd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) == -1) error_exit("setsockopt");

//        struct sockaddr_un client_sock;
//        client_sock.sun_family = AF_UNIX;
//        strcpy(client_sock.sun_path, "\0KURWAAAAAAAA");

//        if(bind(server_sock_fd, (struct sockaddr*) &client_sock, sizeof(sa_family_t)) < 0) error_exit("bind");

        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) < 0) error_exit("connect");
    } else {
//        server_sock.sin_family = AF_INET;
//        server_sock.sin_port = htons(port_number);
//        server_sock.sin_addr.s_addr = inet_addr(server_address);
//
//        if((server_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) error_exit("socket");
//
////        if(bind(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) < 0) error_exit("bind");
//
//        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) < 0) error_exit("connect");
    }
}

void close_connection() {
    send_message(server_sock_fd, LOGOUT, NULL);

    if(shutdown(server_sock_fd, SHUT_RDWR) < 0) error_exit("shutdown");

    if(close(server_sock_fd) < 0) error_exit("close");
}

void* process_input() {
    printf("Your move: ");
    scanf("%s", input);

    pthread_exit((void *) 0);
}

int main(int argc, char** argv) {
    if(argc < 2) error_exit("too few arguments");

    is_local_connection = strcmp(argv[1], "LOCAL") ? 0 : 1;

    if((is_local_connection && argc < 4) || (!is_local_connection && argc < 5)) error_exit("too few arguments");

    name = argv[2];
    server_address = argv[3];

    if(!is_local_connection) {
        port_number = atoi(argv[4]);
    }

    signal(SIGINT, close_connection);

    open_connection();

    send_message(server_sock_fd, LOGIN_REQUEST, name);
    message* msg = read_message(server_sock_fd);

    if(msg->type == LOGIN_APPROVED) {
        printf("Registered\n");
    }

//    message* msg = read_message(server_sock_fd);

//    if(msg->type == LOGIN_APPROVED) {
//        printf("Registered\n");
//
//        while(1) {
//            msg = read_message(server_sock_fd);
//
//            if(msg->type == GAME_WAITING) {
//                printf("Waiting for game...\n");
//            } else if(msg->type == GAME_FOUND) {
//                printf("Found game. Received sign %s\n", msg->data);
//
//                if(msg->data[0] == 'X') {
//                    printf("Making move 0\n");
//                    send_message(server_sock_fd, GAME_MOVE, "0");
//                }
//
//                while(1) {
//                    msg = read_message(server_sock_fd);
//
//                    if(msg->type == GAME_MOVE) {
//                        printf("Opponent made move. Current board:\n");
//                        printf("%s", msg->data);
//
//                        input[0] = '.';
//                        if(pthread_create(&input_thread, NULL, process_input, NULL) < 0) error_exit("pthread_create");
//                        while(input[0] == '.') {
//                            msg = read_message_nonblocking(server_sock_fd);
//                            if(msg != NULL) {
//                                if(msg->type == PING) {
//                                    printf("Received PING\n");
//                                    send_message(server_sock_fd, PING, NULL);
//                                }
//                            }
//                        }
//
//                        printf("Making move %s\n", input);
//                        send_message(server_sock_fd, GAME_MOVE, input);
//                    } else if(msg->type == GAME_FINISHED) {
//                        printf("Game finished, message: %s\n", msg->data);
//                        send_message(server_sock_fd, LOGOUT, NULL);
//                        printf("Logged out from server\n");
//                        break;
//                    } else if(msg->type == PING) {
//                        printf("Received PING\n");
//                        send_message(server_sock_fd, PING, NULL);
//                    }
//                }
//
//                break;
//            } else if(msg->type == PING) {
//                printf("Received PING\n");
//                send_message(server_sock_fd, PING, NULL);
//            }
//        }
//
//    } else {
//        printf("Server rejected login. Reason: %s\n", msg->data);
//    }

    close_connection();

    return 0;
}