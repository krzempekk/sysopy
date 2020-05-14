#include "common.h"

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    perror("Errno code");
    exit(EXIT_FAILURE);
}

enum MSG_TYPE get_message_type(char* message) {
    enum MSG_TYPE type;
    sscanf(message, "%d", (int*) &type);
    return type;
}

char* get_message_data(char* message) {
    int tmp;
    char* content = (char*) calloc(MAX_MSG_LEN - 1, sizeof(char));
    sscanf(message, "%d:%s", &tmp, content);
    return content;
}

message* read_message(int sock_fd) {
    message* msg = (message*) malloc(sizeof(message));
    char* msg_raw = (char*) calloc(MAX_MSG_LEN, sizeof(char));
    if(read(sock_fd, (void*) msg_raw, MAX_MSG_LEN) < 0) error_exit("read");
    msg->type = get_message_type(msg_raw);
    strcpy(msg->data, get_message_data(msg_raw));
    free(msg_raw);
    return msg;
}

void send_message(int sock_fd, MSG_TYPE type, char* content) {
    char* msg_raw = (char*) calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(msg_raw, "%d:%s", (int) type, content);
    if(write(sock_fd, (void*) msg_raw, MAX_MSG_LEN) < 0) error_exit("write");
    free(msg_raw);
}


