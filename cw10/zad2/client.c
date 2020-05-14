#include "common.h"

int is_local_connection;
char* name;
char* server_address;
int port_number;

int server_sock_fd;

pthread_t input_thread;
char input[2];

void open_connection() {
    if(is_local_connection) {
        struct sockaddr_un server_sock;

        server_sock.sun_family = AF_UNIX;
        strcpy(server_sock.sun_path, server_address);

        if((server_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) error_exit("socket");

        struct sockaddr_un client_sock;
        client_sock.sun_family = AF_UNIX;
        sprintf(client_sock.sun_path, "%s", name);

        if(bind(server_sock_fd, (struct sockaddr*) &client_sock, sizeof(client_sock)) < 0) error_exit("bind");

        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) < 0) error_exit("connect");
    } else {
        struct sockaddr_in* server_sock = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));

        server_sock->sin_family = AF_INET;
        server_sock->sin_port = htons(port_number);
        server_sock->sin_addr.s_addr = inet_addr(server_address);

        if((server_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) error_exit("socket");

        struct sockaddr_in client_sock;
        client_sock.sin_family = AF_INET;
        client_sock.sin_port = 0;
        client_sock.sin_addr.s_addr = inet_addr(server_address);

        if(bind(server_sock_fd, (struct sockaddr*) &client_sock, sizeof(client_sock)) < 0) error_exit("bind");

        if(connect(server_sock_fd, (struct sockaddr*) server_sock, sizeof(struct sockaddr_in)) < 0) error_exit("connect");
    }
}

void close_connection() {
    send_message(server_sock_fd, LOGOUT, NULL, name);
    printf("Logged out from server\n");

    if(close(server_sock_fd) < 0) error_exit("close");
}

void* process_input() {
    printf("Your move: ");
    scanf("%s", input);

    pthread_exit((void *) 0);
}

void read_input() {
    input[0] = '.';
    if(pthread_create(&input_thread, NULL, process_input, NULL) < 0) error_exit("pthread_create");
    message* msg;
    while(input[0] == '.') {
        msg = read_message_nonblocking(server_sock_fd);
        if(msg != NULL) {
            if(msg->type == PING) {
                printf("Received PING\n");
                send_message(server_sock_fd, PING, NULL, name);
            }
        }
    }
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

    send_message(server_sock_fd, LOGIN_REQUEST, NULL, name);
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
                    read_input();
                    printf("Making move %s\n", input);
                    send_message(server_sock_fd, GAME_MOVE, input, name);
                }

                while(1) {
                    msg = read_message(server_sock_fd);

                    if(msg->type == GAME_MOVE) {
                        printf("Opponent made move. Current board:\n");
                        printf("%s", msg->data);

                        read_input();
                        printf("Making move %s\n", input);
                        send_message(server_sock_fd, GAME_MOVE, input, name);
                    } else if(msg->type == GAME_FINISHED) {
                        printf("Game finished, message: %s\n", msg->data);
                        break;
                    } else if(msg->type == PING) {
                        printf("Received PING\n");
                        send_message(server_sock_fd, PING, NULL, name);
                    }
                }

                break;
            } else if(msg->type == PING) {
                printf("Received PING\n");
                send_message(server_sock_fd, PING, NULL, name);
            }

        }
    } else {
        printf("Server rejected login. Reason: %s\n", msg->data);
    }

    close_connection();

    return 0;
}