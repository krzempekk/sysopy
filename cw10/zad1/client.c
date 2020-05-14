#include "common.h"

int is_local_connection;
char* name;
char* server_address;
int port_number;

int server_sock_fd;
struct sockaddr_un server_sock;

void open_connection() {
    if(is_local_connection) {
        if((server_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) error_exit("socket");

        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) < 0) error_exit("connect");
    } else {
        if((server_sock_fd = socket(AF_INIT, SOCK_STREAM, 0)) < 0) error_exit("socket");
    }
}

void close_connection() {
    if(shutdown(server_sock_fd, SHUT_RDWR) < 0) error_exit("shutdown");

    if(close(server_sock_fd) < 0) error_exit("close");
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

    server_sock.sun_family = AF_UNIX;
    strcpy(server_sock.sun_path, server_address);

    open_connection();

    send_message(server_sock_fd, LOGIN_REQUEST, name);
    message* msg = read_message(server_sock_fd);

    if(msg->type == LOGIN_APPROVED) {
        printf("Registered\n");

        while(1) {
            msg = read_message(server_sock_fd);

            if(msg->type == GAME_WAITING) {
                printf("Waiting for game...\n");
            } else if(msg->type == GAME_FOUND) {
                printf("Found game. Received sign %s\n", msg->data);

                if(msg->data[0] == 'X') {
                    printf("Making move 0\n");
                    send_message(server_sock_fd, GAME_MOVE, "0");
                }

                while(1) {
                    msg = read_message(server_sock_fd);

                    if(msg->type == GAME_MOVE) {
                        printf("Opponent made move. Current board:\n");
                        printf("%s", msg->data);
                        char in[2];
                        printf("Your move: ");
                        scanf("%s", in);
                        printf("Making move %s\n", in);
                        send_message(server_sock_fd, GAME_MOVE, in);
                    } else if(msg->type == GAME_FINISHED) {
                        printf("Game finished, message: %s\n", msg->data);
                        send_message(server_sock_fd, LOGOUT, NULL);
                        printf("Logged out from server\n");
                        break;
                    } else if(msg->type == PING) {
                        printf("Received PING\n");
                        send_message(server_sock_fd, PING, NULL);
                    }
                }

                break;
            } else if(msg->type == PING) {
                printf("Received PING\n");
                send_message(server_sock_fd, PING, NULL);
            }
        }

    } else {
        printf("Server rejected login. Reason: %s\n", msg->data);
    }

    close_connection();

    return 0;
}