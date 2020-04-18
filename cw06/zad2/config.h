#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CLIENTS 5

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} m_type;

const char* SERVER_QUEUE_NAME = "/SERVER";
const int MAX_MSG_LEN = 8192;
const int NAME_LEN = 20;

#endif //CONFIG_H
