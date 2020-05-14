#include "common.h"

client* clients[MAX_CLIENTS];

int port_number;
char* socket_path;

int sock_un_fd;
struct sockaddr_un sock_unix;

int sock_in_fd;
struct sockaddr_in sock_inet;

pthread_t connection_thread;
pthread_t ping_thread;

int waiting_client;

game* games[MAX_CLIENTS / 2];
FIELD client_signs[MAX_CLIENTS];
int client_games[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void start_server() {
    sock_unix.sun_family = AF_UNIX;
    strcpy(sock_unix.sun_path, socket_path);

    if((sock_un_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) error_exit("socket");

    if(bind(sock_un_fd, (struct sockaddr*) &sock_unix, sizeof(sock_unix)) < 0) error_exit("bind");

    if(listen(sock_un_fd, MAX_CLIENTS) < 0) error_exit("listen");

    printf("UNIX socket listening on %s\n", socket_path);

    struct hostent* host_entry = gethostbyname("localhost");
    struct in_addr host_address = *(struct in_addr*) host_entry->h_addr;

    sock_inet.sin_family = AF_INET;
    sock_inet.sin_port = htonl(port_number);
    sock_inet.sin_addr.s_addr = host_address.s_addr;

    if((sock_in_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) error_exit("socket");

    if(bind(sock_in_fd, (struct sockaddr*) &sock_inet, sizeof(sock_inet)) < 0) error_exit("bind");

    if(listen(sock_in_fd, MAX_CLIENTS) < 0) error_exit("listen");

    printf("INET socket listening on %s:%d\n", inet_ntoa(host_address), port_number);
}

void stop_server() {
    if(pthread_cancel(connection_thread) < 0) error_exit("pthread_cancel");

    if(pthread_cancel(ping_thread) < 0) error_exit("pthread_cancel");

    if(shutdown(sock_un_fd, SHUT_RDWR) < 0) error_exit("shutdown");

    if(close(sock_un_fd) < 0) error_exit("close");

    if(unlink(socket_path) < 0) error_exit("unlink");

    if(shutdown(sock_in_fd, SHUT_RDWR) < 0) error_exit("shutdown");

    if(close(sock_in_fd) < 0) error_exit("close");
}

void close_connection(int fd) {
    if(shutdown(fd, SHUT_RDWR) < 0) error_exit("shutdown");
    if(close(fd) < 0) error_exit("close");
}

int register_client(int fd, char* name) {
    int free_index = -1;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0) return -1;
        if(clients[i] == NULL && free_index == -1) free_index = i;
    }
    if(free_index == -1) return -1;
    clients[free_index] = create_client(fd, name);
    return free_index;
}

void unregister_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->fd == fd) {
            clients[i] = NULL;
        }
    }
}

int process_login(int sock_fd) {
    printf("New login pending...\n");

    int client_sock_fd;
    if((client_sock_fd = accept(sock_fd, NULL, NULL)) < 0) error_exit("accept");

    message* msg = read_message(client_sock_fd);
    printf("Received name %s\n", msg->data);

    int registered_index = register_client(client_sock_fd, msg->data);
    if(registered_index < 0) {
        printf("Login rejected...\n");
        send_message(client_sock_fd, LOGIN_REJECTED, "name_exists");
        close_connection(client_sock_fd);
    } else {
        printf("Login accepted...\n");
        send_message(client_sock_fd, LOGIN_APPROVED, NULL);
    }
    return registered_index;
}

int add_game(int player1, int player2) {
    for(int i = 0; i < MAX_CLIENTS / 2; i++) {
        if(games[i] == NULL) {
            games[i] = create_new_game(player1, player2);
            return i;
        }
    }
    return -1;
}

void remove_game(int index) {
    games[index] = NULL;
}

void make_match(int registered_index) {
    if(waiting_client < 0) {
        printf("There is no waiting client\n");
        send_message(clients[registered_index]->fd, GAME_WAITING, NULL);
        waiting_client = registered_index;
    } else {
        printf("There is waiting client %d\n", waiting_client);
        int game_index = add_game(registered_index, waiting_client);
        client_games[registered_index] = game_index;
        client_games[waiting_client] = game_index;
        if(random_int(0, 1) == 0) {
            send_message(clients[registered_index]->fd, GAME_FOUND, "X");
            send_message(clients[waiting_client]->fd, GAME_FOUND, "O");
            client_signs[registered_index] = X;
            client_signs[waiting_client] = O;
        } else {
            send_message(clients[registered_index]->fd, GAME_FOUND, "O");
            send_message(clients[waiting_client]->fd, GAME_FOUND, "X");
            client_signs[registered_index] = O;
            client_signs[waiting_client] = X;
        }
        waiting_client = -1;
    }
}

void* process_connections() {
    pollfd fds[MAX_CLIENTS + 2];

    fds[MAX_CLIENTS].fd = sock_un_fd;
    fds[MAX_CLIENTS].events = POLLIN;

    fds[MAX_CLIENTS + 1].fd = sock_in_fd;
    fds[MAX_CLIENTS + 1].fd = POLLIN;

    waiting_client = -1;

    while(1) {
        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < MAX_CLIENTS; i++) {
            fds[i].fd = clients[i] != NULL ? clients[i]->fd : -1;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }

        pthread_mutex_unlock(&clients_mutex);

        fds[MAX_CLIENTS].revents = 0;
        fds[MAX_CLIENTS + 1].revents = 0;

        printf("Pohling...\n");

        poll(fds, MAX_CLIENTS + 2, -1);

        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < MAX_CLIENTS + 2; i++) {
            if(i < MAX_CLIENTS && clients[i] == NULL) continue;

            if(fds[i].revents & POLLHUP) {
                close_connection(fds[i].fd);
                unregister_client(fds[i].fd);
            } else if(fds[i].revents & POLLIN) {
                if(fds[i].fd == sock_un_fd || fds[i].fd == sock_in_fd) {
                    int registered_index = process_login(fds[i].fd);
                    printf("Client registered at index %d\n", registered_index);
                    if(registered_index >= 0) make_match(registered_index);
                } else {
                    printf("Received message from client\n");
                    message* msg = read_message(fds[i].fd);
                    if(msg->type == GAME_MOVE) {
                        printf("Move made: %s\n", msg->data);
                        game* g = games[client_games[i]];
                        int field_in = atoi(msg->data);
                        FIELD sign = client_signs[i];
                        make_move(g, field_in, sign);
                        int other = g->player_1 == i ? g->player_2 : g->player_1;

                        GAME_STATUS status = check_game_status(g);
                        if(status == IDLE) {
                            send_message(fds[other].fd, GAME_MOVE, get_board_string(g));
                        } else {
                            send_message(fds[i].fd, GAME_FINISHED, "finished");
                            send_message(fds[other].fd, GAME_FINISHED, "finished");
                        }
                    } else if(msg->type == PING) {
                        clients[i]->responding = 1;
                    }
                }
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    pthread_exit((void *) 0);
}

void* process_ping() {
    while(1) {
        sleep(PING_INTERVAL);

        printf("Pinging clients...\n");

        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i] != NULL) {
                clients[i]->responding = 0;
                send_message(clients[i]->fd, PING, NULL);
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        printf("Waiting for ping responses...\n");

        sleep(PING_TIMEOUT);

        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i] != NULL && clients[i]->responding == 0) {
                printf("Client %d didnt respond to ping, disconnecting...\n", i);
                close_connection(clients[i]->fd);
                unregister_client(clients[i]->fd);
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }


    pthread_exit((void *) 0);
}

int main(int argc, char** argv) {
    srand(time(NULL));
    if(argc < 3) error_exit("too few arguments");

    port_number = atoi(argv[1]);
    socket_path = argv[2];

    signal(SIGINT, stop_server);

    start_server();

    if(pthread_create(&connection_thread, NULL, process_connections, NULL) < 0) error_exit("pthread_create");
    if(pthread_create(&ping_thread, NULL, process_ping, NULL) < 0) error_exit("pthread_create");

    if(pthread_join(connection_thread, NULL) < 0) error_exit("pthread_join");
    if(pthread_join(ping_thread, NULL) < 0) error_exit("pthread_join");

    stop_server();

    return 0;
}