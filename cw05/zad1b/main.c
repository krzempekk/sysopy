#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

const int MAX_LINE_LEN = 10000;
const int MAX_COMMANDS_IN_LINE = 10;
const int MAX_COMMAND_LEN = 1000;

int main(int argc, char** argv) {

    FILE* f = fopen("commands", "r");

    char* row = (char*)calloc(MAX_LINE_LEN, sizeof(char));
    while(fgets(row, MAX_LINE_LEN, f) != NULL) {
        row[strcspn(row, "\n")] = 0;

        char** commands = (char**)calloc(MAX_COMMANDS_IN_LINE, sizeof(char*));

        char* end_command;
        char* command = strtok_r(row, "|", &end_command);
        int cmd_index = 0;
        while(command != NULL) {
            commands[cmd_index] = (char*)calloc(MAX_COMMAND_LEN, sizeof(char));
            strcpy(commands[cmd_index], command);
            command = strtok_r(NULL, "|", &end_command);
            cmd_index++;
        }



//        char* buf = (char*)calloc(10000, sizeof(char));
//        read(fd[0], buf, 10000);
//        printf("command output: %s\n", buf);

    }
    free(row);
    fclose(f);
    return 0;
}