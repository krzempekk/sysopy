#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

double time_diff(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void write_result(struct tms* t_start, struct tms* t_end){
    printf("\tUSER: %f\tSYSTEM: %f\n", time_diff(t_start->tms_utime, t_end->tms_utime), time_diff(t_start->tms_stime, t_end->tms_stime));
}

int random_int(int a, int b) {
    return rand() % (b - a + 1) + a;
}

char* generate(int rec_count, int rec_size) {
    srand(time(0));

    int char_count = rec_count * (rec_size + 1);

    char* recs = (char*)calloc(char_count, sizeof(char));

    for(int i = 0; i < char_count; i++) {
        if(i % (rec_size + 1) == rec_size) {
            recs[i] = '\n';
        } else {
            recs[i] = (char)random_int('A', 'Z');
        }
    }

    return recs;
}

char* get_record(int fd, int rec_size, int index) {
    lseek(fd, (rec_size + 1) * index, 0);
    char* rec = (char*)calloc(rec_size, sizeof(char));

    if(read(fd, rec, rec_size) > 0) {
        return rec;
    }
    return NULL;
}

char* get_record_lib(FILE* f, int rec_size, int index) {
    fseek(f, (rec_size + 1) * index, 0);
    char* rec = (char*)calloc(rec_size, sizeof(char));

    fread(rec, 1, rec_size, f);
    return rec;
}

void save_record(int fd, int rec_size, int index, char* rec) {
    lseek(fd, (rec_size + 1) * index, 0);

    write(fd, rec, rec_size);
    write(fd, "\n", 1);
}

void save_record_lib(FILE* f, int rec_size, int index, char* rec) {
    fseek(f, (rec_size + 1) * index, 0);

    fwrite(rec, 1, rec_size, f);
    fwrite("\n", 1, 1, f);
}

void switch_records(int fd, int rec_size, int index_1, int index_2) {
    char* rec_1 = (char*)calloc(rec_size, sizeof(char));
    char* rec_2 = (char*)calloc(rec_size, sizeof(char));

    lseek(fd, (rec_size + 1) * index_1, 0);
    read(fd, rec_1, rec_size);

    lseek(fd, (rec_size + 1) * index_2, 0);
    read(fd, rec_2, rec_size);

    lseek(fd, (rec_size + 1) * index_1, 0);
    write(fd, rec_2, rec_size);

    lseek(fd, (rec_size + 1) * index_2, 0);
    write(fd, rec_1, rec_size);

    free(rec_1);
    free(rec_2);
}

void switch_records_lib(FILE* f, int rec_size, int index_1, int index_2) {
    char* rec_1 = (char*)calloc(rec_size, sizeof(char));
    char* rec_2 = (char*)calloc(rec_size, sizeof(char));

    fseek(f, (rec_size + 1) * index_1, 0);
    fread(rec_1, 1, rec_size, f);

    fseek(f, (rec_size + 1) * index_2, 0);
    fread(rec_2, 1, rec_size, f);

    fseek(f, (rec_size + 1) * index_1, 0);
    fwrite(rec_2, 1, rec_size, f);

    fseek(f, (rec_size + 1) * index_2, 0);
    fwrite(rec_1, 1, rec_size, f);

    free(rec_1);
    free(rec_2);
}

int partition(int fd, int rec_size, int left, int right) {
    char* pivot = get_record(fd, rec_size, right);

    int i = left - 1;

    for(int j = left; j < right; j++) {
        char* el = get_record(fd, rec_size, j);
        if(strcmp(el, pivot) < 0) {
            i++;
            switch_records(fd, rec_size, i, j);
        }
        free(el);
    }

    switch_records(fd, rec_size, i + 1, right);

    free(pivot);
    return i + 1;
}

int partition_lib(FILE* f, int rec_size, int left, int right) {
    char* pivot = get_record_lib(f, rec_size, right);

    int i = left - 1;

    for(int j = left; j < right; j++) {
        char* el = get_record_lib(f, rec_size, j);
        if(strcmp(el, pivot) < 0) {
            i++;
            switch_records_lib(f, rec_size, i, j);
        }
        free(el);
    }

    switch_records_lib(f, rec_size, i + 1, right);

    free(pivot);
    return i + 1;
}

void quicksort(int fd, int rec_size, int left, int right) {
    if(left < right) {
        int pi = partition(fd, rec_size, left, right);

        quicksort(fd, rec_size, left, pi - 1);
        quicksort(fd, rec_size, pi + 1, right);
    }
}

void quicksort_lib(FILE* f, int rec_size, int left, int right) {
    if(left < right) {
        int pi = partition_lib(f, rec_size, left, right);

        quicksort_lib(f, rec_size, left, pi - 1);
        quicksort_lib(f, rec_size, pi + 1, right);
    }
}

int main(int argc, char** argv) {
    srand(time(0));

    if(argc < 5) {
        printf("Not enough arguments provided\n");
        return 1;
    }

    char* command = argv[1];

    struct tms* tms_start = malloc(sizeof(struct tms));
    struct tms* tms_end = malloc(sizeof(struct tms));

    if(strcmp(command, "generate") == 0) {
        if(argc < 5) {
            printf("Too few arguments\n");
            return 1;
        }

        char* file = argv[2];
        int rec_count = atoi(argv[3]), rec_size = atoi(argv[4]);

        int fd = open(file, O_RDWR | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU);
        write(fd, generate(rec_count, rec_size), rec_count * (rec_size + 1));
        close(fd);
    } else if(strcmp(command, "sort") == 0) {
        if(argc < 6) {
            printf("Too few arguments\n");
            return 1;
        }

        times(tms_start);

        char* file = argv[2];
        int rec_count = atoi(argv[3]), rec_size = atoi(argv[4]);
        char* mode = argv[5];

        if(strcmp(mode, "sys") == 0) {
            int fd = open(file, O_RDWR);
            quicksort(fd, rec_size, 0, rec_count - 1);
            close(fd);
        } else if(strcmp(mode, "lib") == 0) {
            FILE* f = fopen(file, "r+");
            quicksort_lib(f, rec_size, 0, rec_count - 1);
            fclose(f);
        } else {
            printf("Unrecognized mode %s\n", mode);
            return 1;
        }

        times(tms_end);
        write_result(tms_start, tms_end);
    } else if(strcmp(command, "copy") == 0) {
        if(argc < 7) {
            printf("Too few arguments\n");
            return 1;
        }

        times(tms_start);

        char* file1 = argv[2];
        char* file2 = argv[3];
        int rec_count = atoi(argv[4]), rec_size = atoi(argv[5]);
        char* mode = argv[6];

        char* buf = (char*)calloc(rec_size, sizeof(char));
        if(strcmp(mode, "sys") == 0) {
            int fd1 = open(file1, O_RDONLY), fd2 = open(file2, O_WRONLY | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU);
            for(int i = 0; i < rec_count; i++) {
                buf = get_record(fd1, rec_size, i);
                save_record(fd2, rec_size, i, buf);
            }

            close(fd1);
            close(fd2);
        } else if(strcmp(mode, "lib") == 0) {
            FILE* f1 = fopen(file1, "r+");
            FILE* f2 = fopen(file2, "w");

            for(int i = 0; i < rec_count; i++) {
                buf = get_record_lib(f1, rec_size, i);
                save_record_lib(f2, rec_size, i, buf);
            }

            fclose(f1);
            fclose(f2);
        } else {
            printf("Unrecognized mode %s\n", mode);
            return 1;
        }
        free(buf);

        times(tms_end);
        write_result(tms_start, tms_end);
    } else {
        printf("Unrecognized command %s\n", command);
        return 1;
    }

    return 0;
}