#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main() {
    if(mkfifo("pipe", S_IRWXU | S_IRWXG | S_IRWXO) < 0) error_exit("cannot create named pipe");

    char input_file_name[7];

    for(int i = 0; i < 5; i++) {
        sprintf(input_file_name, "input%d", i);

        FILE* f = fopen(input_file_name, "w");
        for(int j = 0; j < 25; j++) {
            char c = 'A' + i;
            fwrite(&c, 1, 1, f);
        }

        if(fork() == 0) {
            execl("./producer", "./producer", "pipe", input_file_name, "5", NULL);
        }
    }

    if(fork() == 0) {
        execl("./consumer", "./consumer", "pipe", "output", "5", NULL);
    }

    return 0;
}
