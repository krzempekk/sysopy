#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

const int MAX_LINE_LEN = 10000;
const int MAX_COMMANDS_IN_LINE = 10;
const int MAX_ARGS = 10;
const int MAX_ARG_LEN = 100;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}


int main(int argc, char** argv) {
    if(argc < 2) {
        error_exit("not enough arguments");
    }

    FILE* f = fopen(argv[1], "r");

    char* row = (char*)calloc(MAX_LINE_LEN, sizeof(char));
    while(fgets(row, MAX_LINE_LEN, f) != NULL) {
        row[strcspn(row, "\n")] = 0;

        char*** commands = (char***)calloc(MAX_COMMANDS_IN_LINE, sizeof(char**));

        char* end_command;
        char* command = strtok_r(row, "|", &end_command);
        int cmd_index = 0;
        while(command != NULL) {
            char* end_arg;
            char* arg = strtok_r(command, " ", &end_arg);
            int arg_index = 0;
            commands[cmd_index] = (char**)calloc(MAX_ARGS, sizeof(char*));
            while(arg != NULL) {
                commands[cmd_index][arg_index] = (char*)calloc(MAX_ARG_LEN, sizeof(char));
                strcpy(commands[cmd_index][arg_index], arg);
                arg = strtok_r(NULL, " ", &end_arg);
                arg_index++;
            }
            command = strtok_r(NULL, "|", &end_command);
            cmd_index++;
        }

        pid_t* pids = (pid_t*)calloc(MAX_COMMANDS_IN_LINE, sizeof(pid_t));

        int fd[2];
        pipe(fd);

        pid_t child_pid = fork();
        if(child_pid == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execvp(commands[0][0], commands[0]);
        }
        pids[0] = child_pid;

        int fd_prev[2];
        fd_prev[0] = fd[0]; fd_prev[1] = fd[1];

        int i = 1;
        while(commands[i] != NULL) {
            fd_prev[0] = fd[0]; fd_prev[1] = fd[1];
            pipe(fd);

            pid_t child_pid = fork();

            if(child_pid == 0) {
                dup2(fd_prev[0], STDIN_FILENO);
                dup2(fd[1], STDOUT_FILENO);
                close(fd_prev[0]);
                close(fd_prev[1]);
                close(fd[0]);
                close(fd[1]);
                execvp(commands[i][0], commands[i]);
            }
            close(fd_prev[0]);
            close(fd_prev[1]);
            pids[i] = child_pid;

            i++;
        }

        for(int j = 0; j < i; j++) {
            waitpid(pids[j], NULL, 0);
        }

        close(fd[1]);
        close(fd[0]);

        free(pids);

        int k = 0;
        while(commands[k] != NULL) {
            int l = 0;
            while(commands[k][l] != NULL) {
                free(commands[k][l]);
                l++;
            }
            free(commands[k]);
            k++;
        }
        free(commands);
    }
    free(row);
    fclose(f);
    return 0;
}