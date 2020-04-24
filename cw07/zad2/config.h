#ifndef CONFIG_H
#define CONFIG_H

#define MAX_ORDERS 5
#define W_1_COUNT 3
#define W_2_COUNT 3
#define W_3_COUNT 3

const int MIN_SLEEP = 100;
const int MAX_SLEEP = 1000;

const char* NAMES[6] = { "/MEMACCESS", "/FREEINDEX", "/PACKINDEX", "/PACKCOUNT", "/SENDINDEX", "/SENDCOUNT" };
const char* SHM_NAME = "/ORDERS";

struct orders {
    int values[MAX_ORDERS];
};

typedef struct orders orders;


#endif //CONFIG_H
