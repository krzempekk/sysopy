#include "common.h"

char* clients[MAX_CLIENTS];

int port_number;
char* socket_path;

int sock_un_fd;
struct sockaddr_un sock_unix;

pthread_t connection_thread;

void start_server() {
    sock_unix.sun_family = AF_UNIX;
    strcpy(sock_unix.sun_path, socket_path);

    if((sock_un_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) error_exit("socket");

    if(bind(sock_un_fd, (struct sockaddr*) &sock_unix, sizeof(sock_unix)) < 0) error_exit("bind");

    if(listen(sock_un_fd, MAX_CLIENTS) < 0) error_exit("listen");
}

void stop_server() {
    if(pthread_cancel(connection_thread) < 0) error_exit("pthread_cancel");

    if(shutdown(sock_un_fd, SHUT_RDWR) < 0) error_exit("shutdown");

    if(close(sock_un_fd) < 0) error_exit("close");

    if(unlink(socket_path) < 0) error_exit("unlink");
}

int register_client(char* name) {
    int free_index = -1;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL && strcmp(clients[i], name) == 0) return -1;
        if(clients[i] == NULL && free_index == -1) free_index = i;
    }
    clients[free_index] = name;
    return free_index;
}

void* process_connections() {
    while(1) {
        printf("Waiting for connections...\n");

//        zabezpieczyc przed za duza iloscia klientow

        int client_sock_fd;
        if((client_sock_fd = accept(sock_un_fd, NULL, NULL)) < 0) error_exit("accept");

        message* msg = read_message(client_sock_fd);
        printf("Received name %s\n", msg->data);
        send_message(client_sock_fd, LOGIN_APPROVED, NULL);

//        char* client_name = read_message(client_sock_fd);
//        if(register_client(client_name) < 0) {
//            send_message(client_sock_fd, NAME_EXISTS);
//        } else {
//            send_message(client_sock_fd, REGISTERED);
//        }

        printf("Connection accepted...\n");
    }

    pthread_exit((void *) 0);
}

void* process_matchmaking() {
    return NULL;
}

void* process_moves() {
    return NULL;
}

int main(int argc, char** argv) {
    if(argc < 3) error_exit("too few arguments");

    port_number = atoi(argv[1]);
    socket_path = argv[2];

    signal(SIGINT, stop_server);

    start_server();

    if(pthread_create(&connection_thread, NULL, process_connections, NULL) < 0) error_exit("pthread_create");

    if(pthread_join(connection_thread, NULL) < 0) error_exit("pthread_join");

    stop_server();

    return 0;
}