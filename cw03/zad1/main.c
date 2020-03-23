#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/wait.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>

struct time_compare {
    int m_time_days;
    int m_time_operator;
    int a_time_days;
    int a_time_operator;
};

const char* date_format = "%Y-%m-%d %H:%M:%S";
char date_string[20];
struct time_compare* tc;
int max_depth;

int day_diff(time_t date) {
    return difftime(time(NULL), date) / 86400;
}

bool is_time_correct(const struct stat* stats) {
    bool is_correct = true;

    if(tc->m_time_days >= 0) {
        int diff = day_diff(stats->st_mtime);
        switch(tc->m_time_operator) {
            case -1:
                is_correct = is_correct && diff < tc->m_time_days;
                break;
            case 0:
                is_correct = is_correct && diff == tc->m_time_days;
                break;
            case 1:
                is_correct = is_correct && diff > tc->m_time_days;
                break;
        }
    }

    if(tc->a_time_days >= 0) {
        int diff = day_diff(stats->st_atime);
        switch(tc->a_time_operator) {
            case -1:
                is_correct = is_correct && diff < tc->a_time_days;
                break;
            case 0:
                is_correct = is_correct &&  diff == tc->a_time_days;
                break;
            case 1:
                is_correct = is_correct &&  diff > tc->a_time_days;
                break;
        }
    }

    return is_correct;
}

char* file_type(mode_t mode) {
    if(S_ISREG(mode))
        return "file";
    if(S_ISDIR(mode))
        return "dir";
    if(S_ISCHR(mode))
        return "char dev";
    if(S_ISBLK(mode))
        return "block dev";
    if(S_ISFIFO(mode))
        return "fifo";
    if(S_ISLNK(mode))
        return "slink";
    if(S_ISSOCK(mode))
        return "sock";
    return "unrecognized";
}

void print_file_info(const char* file_path, const struct stat* stats) {
    printf("file: %s\n", file_path);
    printf("\thardlinks: %lu\n", stats->st_nlink);
    printf("\ttype: %s\n", file_type(stats->st_mode));
    printf("\tsize: %ld bytes\n", stats->st_size);
    strftime(date_string, 20, date_format, localtime(&stats->st_atime));
    printf("\tlast access: %s\n", date_string);
    strftime(date_string, 20, date_format, localtime(&stats->st_mtime));
    printf("\tlast modification: %s\n", date_string);
}

void traverse_directory(char* dir_path, int depth) {
    DIR* dir = opendir(dir_path);
    if(dir == NULL) {
        printf("Directory %s don't exist or can't be read\n", dir_path);
        exit(1);
    }

    struct dirent* dir_entry = readdir(dir);

    while(dir_entry != NULL) {
        char* file_name = dir_entry->d_name;

        if(strcmp(file_name, ".") != 0 && strcmp(file_name, "..") != 0) {
            char* file_path = (char*)calloc(strlen(dir_path) + strlen(file_name) + 2, sizeof(char));
            strcpy(file_path, dir_path);
            strcat(file_path, "/");
            strcat(file_path, file_name);

            struct stat* stats = (struct stat*)malloc(sizeof(struct stat));
            lstat(file_path, stats);

            if(is_time_correct(stats)) {
//                print_file_info(file_path, stats);
            }

            if(S_ISDIR(stats->st_mode) && (max_depth < 0 || depth < max_depth)) {
                pid_t child_pid = fork();
                if(child_pid == 0) {
                    execlp("ls", "ls", "-l", file_path, NULL);
                } else {
                    printf("path: %s\n", file_path);
                    printf("process PID: %d\n", child_pid);
                    waitpid(child_pid, NULL, 0);
                }
                traverse_directory(file_path, depth + 1);
            }

            free(stats);
            free(file_path);
        }

        dir_entry = readdir(dir);
    }

    free(dir_entry);
    closedir(dir);
}

static int nftw_callback(const char* file_path, const struct stat* stats, int type_flag, struct FTW* ftw_buf) {
    if(type_flag == FTW_D) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            execlp("ls", "ls", "-l", file_path, NULL);
        } else {
            printf("path: %s\n", file_path);
            printf("process PID: %d\n", child_pid);
            waitpid(child_pid, NULL, 0);
        }
    }
    if(is_time_correct(stats) && (max_depth < 0 || ftw_buf->level <= max_depth)) {
        return 0;
    }
    return 1;
}


int main(int argc, char** argv) {
    max_depth = -1;
    tc = (struct time_compare*)malloc(sizeof(struct time_compare));
    tc->a_time_days = -1;
    tc->m_time_days = -1;
    bool nftw_mode = false;

    if(argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    char* directory_path = argv[1];
    for(int i = 2; i < argc;) {
        if(strcmp(argv[i], "-maxdepth") == 0) {
            if(i + 1 >= argc) { printf("maxdepth value not provided\n"); return 1; }
            max_depth = strtol(argv[i + 1], NULL, 10);
            i += 2;
        } else if(strcmp(argv[i], "-mtime") == 0) {
            if(i + 1 >= argc) { printf("mtime value not provided\n"); return 1; }
            char sign = argv[i + 1][0];
            tc->m_time_operator = (sign == '+') ? 1 : (sign == '-' ? -1 : 0);
            tc->m_time_days = abs(strtol(argv[i + 1], NULL, 10));
            i += 2;
        } else if(strcmp(argv[i], "-atime") == 0) {
            if(i + 1 >= argc) { printf("atime value not provided\n"); return 1; }
            char sign = argv[i + 1][0];
            tc->a_time_operator = (sign == '+') ? 1 : (sign == '-' ? -1 : 0);
            tc->a_time_days = abs(strtol(argv[i + 1], NULL, 10));
            i += 2;
        } else if(strcmp(argv[i], "-nftw") == 0) {
            nftw_mode = true;
            i++;
        } else {
            printf("Unrecognized option %s\n", argv[i]);
            return 1;
        }
    }

    char* absolute_path = realpath(directory_path, NULL);

    if(absolute_path == NULL) {
        printf("Directory %s don't exist or can't be read\n", directory_path);
        return 1;
    }

    if (nftw_mode) {
        nftw(absolute_path, nftw_callback, 10, FTW_PHYS);
    } else {
        traverse_directory(absolute_path, 1);
    }

    free(tc);
    free(absolute_path);

    return 0;
}