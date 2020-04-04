#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

const int MAX_LEN = 100;
const int MAX_MSG_LEN = 100;

int main(int argc, char** argv) {
    char* pipe_name = (char*)calloc(MAX_LEN, sizeof(char));
    strcpy(pipe_name, argv[1]);
    char* output_file_name = (char*)calloc(MAX_LEN, sizeof(char));
    strcpy(output_file_name, argv[2]);
    int n = atoi(argv[3]);

    FILE* pipe = fopen(pipe_name, "r");
    FILE* output_file = fopen(output_file_name, "w");
    char* buf = (char*)calloc(n, sizeof(char));
    while(fread(buf, 1, n, pipe) > 0) {
        fwrite(buf, 1, strlen(buf), output_file);
        memset(buf,0,strlen(buf));
    }
    fclose(pipe);
    fclose(output_file);
    free(pipe_name);
    free(output_file_name);
    free(buf);
    return 0;
}
