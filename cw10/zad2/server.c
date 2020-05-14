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

    if((sock_un_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) error_exit("socket");

    if(bind(sock_un_fd, (struct sockaddr*) &sock_unix, sizeof(sock_unix)) < 0) error_exit("bind");

    printf("UNIX socket listening on %s\n", socket_path);

    struct hostent* host_entry = gethostbyname("localhost");
    struct in_addr host_address = *(struct in_addr*) host_entry->h_addr;

    sock_inet.sin_family = AF_INET;
    sock_inet.sin_port = htons(port_number);
    sock_inet.sin_addr.s_addr = host_address.s_addr;

    if((sock_in_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) error_exit("socket");

    if(bind(sock_in_fd, (struct sockaddr*) &sock_inet, sizeof(sock_inet)) < 0) error_exit("bind");

    printf("INET socket listening on %s:%d\n", inet_ntoa(host_address), port_number);
}

void stop_server() {
    if(pthread_cancel(connection_thread) < 0) error_exit("pthread_cancel");

    if(pthread_cancel(ping_thread) < 0) error_exit("pthread_cancel");

    if(close(sock_un_fd) < 0) error_exit("close");

    if(unlink(socket_path) < 0) error_exit("unlink");

    if(close(sock_in_fd) < 0) error_exit("close");

    exit(0);
}

int register_client(int fd, struct sockaddr* addr, char* name) {
    int free_index = -1;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0) return -1;
        if(clients[i] == NULL && free_index == -1) free_index = i;
    }
    if(free_index == -1) return -1;
    clients[free_index] = create_client(fd, addr, name);
    return free_index;
}

void unregister_client(char* name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && strcmp(clients[i]->name, name) == 0) {
            clients[i] = NULL;
        }
    }
}

int get_user_index(char* name) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0) return i;
    }
    return -1;
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
        send_message_to(clients[registered_index]->fd, GAME_WAITING, NULL, clients[registered_index]->addr);
        waiting_client = registered_index;
    } else {
        printf("There is waiting client %d\n", waiting_client);
        int game_index = add_game(registered_index, waiting_client);
        client_games[registered_index] = game_index;
        client_games[waiting_client] = game_index;
        if(random_int(0, 1) == 0) {
            send_message_to(clients[registered_index]->fd, GAME_FOUND, "X", clients[registered_index]->addr);
            send_message_to(clients[waiting_client]->fd, GAME_FOUND, "O", clients[waiting_client]->addr);
            client_signs[registered_index] = X;
            client_signs[waiting_client] = O;
        } else {
            send_message_to(clients[registered_index]->fd, GAME_FOUND, "O", clients[registered_index]->addr);
            send_message_to(clients[waiting_client]->fd, GAME_FOUND, "X", clients[waiting_client]->addr);
            client_signs[registered_index] = O;
            client_signs[waiting_client] = X;
        }
        waiting_client = -1;
    }
}

void* process_connections() {
    pollfd fds[2];

    fds[0].fd = sock_un_fd;
    fds[0].events = POLLIN;

    fds[1].fd = sock_in_fd;
    fds[1].events = POLLIN;

    waiting_client = -1;

    while(1) {
        for(int i = 0; i < 2; i++) {
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }

        printf("Pohling...\n");

        poll(fds, 2, -1);

        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < 2; i++) {
            if(fds[i].revents & POLLIN) {
                printf("Received message\n");
                struct sockaddr* addr = (struct sockaddr*) malloc(sizeof(struct sockaddr));
                socklen_t len = sizeof(&addr);
                message* msg = read_message_from(fds[i].fd, addr, &len);

                if(msg->type == LOGIN_REQUEST) {
                    printf("Registering user with name %s\n", msg->user);
                    int registered_index = register_client(fds[i].fd, addr, msg->user);
                    printf("Registered index %d\n", registered_index);
                    if(registered_index < 0) {
                        send_message_to(fds[i].fd, LOGIN_REJECTED, "user_exists", addr);
                    } else {
                        send_message_to(fds[i].fd, LOGIN_APPROVED, NULL, addr);
                        make_match(registered_index);
                    }
                } else if(msg->type == LOGOUT) {
                    unregister_client(msg->user);
                    printf("User %s is logging out\n", msg->user);
                } else if(msg->type == GAME_MOVE) {
                    int index = get_user_index(msg->user);
                    printf("Move made: %s\n", msg->data);
                    game* g = games[client_games[index]];
                    int field_in = atoi(msg->data);
                    FIELD sign = client_signs[index];
                    make_move(g, field_in, sign);
                    int other = g->player_1 == index ? g->player_2 : g->player_1;

                    GAME_STATUS status = check_game_status(g);
                    if(status == IDLE) {
                        send_message_to(clients[other]->fd, GAME_MOVE, get_board_string(g), clients[other]->addr);
                    } else {
                        send_message_to(clients[other]->fd, GAME_FINISHED, "finished", clients[other]->addr);
                        send_message_to(clients[index]->fd, GAME_FINISHED, "finished", clients[index]->addr);
                    }
                } else if(msg->type == PING) {
                    clients[get_user_index(msg->user)]->responding = 1;
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
                send_message_to(clients[i]->fd, PING, NULL, clients[i]->addr);
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        printf("Waiting for ping responses...\n");

        sleep(PING_TIMEOUT);

        pthread_mutex_lock(&clients_mutex);

        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i] != NULL && clients[i]->responding == 0) {
                printf("Client %d didnt respond to ping, disconnecting...\n", i);
                unregister_client(clients[i]->name);
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