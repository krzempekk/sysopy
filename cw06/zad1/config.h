#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CLIENTS 5

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} m_type;

typedef struct msg_buf {
    long m_type;
    char m_text[1024];
    key_t queue_key;
    int client_id;
    int connect_client_id;
} msg_buf;

const int SERVER_PROJ_ID = 1;
const int MSG_SIZE = sizeof(msg_buf) - sizeof(long);

#endif //CONFIG_H
