#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

const int MAX_LEN = 100;
const int MAX_MSG_LEN = 100;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if(argc < 4) {
        error_exit("not enough arguments");
    }

    srand(time(NULL));

    char* pipe_name = (char*)calloc(MAX_LEN, sizeof(char));
    strcpy(pipe_name, argv[1]);
    char* input_file_name = (char*)calloc(MAX_LEN, sizeof(char));
    strcpy(input_file_name, argv[2]);
    int n = atoi(argv[3]);

    FILE* input_file = fopen(input_file_name, "r");
    FILE* pipe = fopen(pipe_name, "w");
    char* buf = (char*)calloc(n, sizeof(char));
    char* msg = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    pid_t pid = getpid();

    while(fread(buf, 1, n, input_file) > 0) {
        sleep(rand() % 2 + 1);
        sprintf(msg, "#%d#%s\n", pid, buf);
        fwrite(msg, 1, strlen(msg), pipe);

        memset(buf,0,strlen(buf));
        memset(msg,0,strlen(msg));
    }
    free(pipe_name);
    free(input_file_name);
    free(buf);
    free(msg);
    fclose(input_file);
    fclose(pipe);

    return 0;
}
