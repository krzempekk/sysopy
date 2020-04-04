#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        error_exit("not enough arguments");
    }

    FILE* f = fopen(argv[1], "r");

    char* row = (char*)calloc(1000, sizeof(char));
    FILE* sort_input = popen("sort", "w");
    while(fgets(row, 1000, f) != NULL) {
        fputs(row, sort_input);
    }
    pclose(sort_input);
    free(row);

    return 0;
}