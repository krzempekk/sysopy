#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>

#define MAX_VAL 256

int width;
int height;
int** image;
int** histograms;

int thread_count;
char* split_method;
char* input_file;
char* output_file;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int time_diff(clock_t t1, clock_t t2){
    return (1e6 * (t2 - t1) / sysconf(_SC_CLK_TCK));
}

int min(int a, int b) {
    return a > b ? b : a;
}

void read_image_from_file() {
    FILE* f = fopen(input_file, "r");
    if(f == NULL) error_exit("cannot open file");

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int line_no = 0;
    int pixel_no = 0;

    while((read = getline(&line, &len, f)) != -1) {
        if(line_no == 1) {
            char* tok = strtok(line, " \n");
            width = atoi(tok);
            tok = strtok(NULL, " \n");
            height = atoi(tok);

            image = (int**) calloc(height, sizeof(int*));
            for(int i = 0; i < height; i++)
                image[i] = (int*) calloc(width, sizeof(int));
        } else if(line_no > 3) {
            char* tok = strtok(line, " \n");
            while(tok != NULL) {
                image[pixel_no / width][pixel_no % width] = atoi(tok);
                tok = strtok(NULL, " \n");
                pixel_no++;
            }
        }
        line_no++;
    }

    fclose(f);
}

void* calculate_histogram_part(void* arg) {
    int index = *((int*) arg);

    clock_t start = clock();

    if(strcmp(split_method, "sign") == 0) {
        int start = index * ceil((double) MAX_VAL / thread_count);
        int end = min((index + 1) * ceil((double) MAX_VAL / thread_count) - 1, MAX_VAL - 1);

        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                if(image[i][j] >= start && image[i][j] <= end) {
                    histograms[index][image[i][j]]++;
                }
            }
        }
    } else if(strcmp(split_method, "block") == 0) {
        int x_min = index * ceil((double) width / thread_count);
        int x_max = min((index + 1) * ceil((double) width / thread_count) - 1, width - 1);

        for(int i = 0; i < height; i++) {
            for(int j = x_min; j <= x_max; j++) {
                histograms[index][image[i][j]]++;
            }
        }
    } else if(strcmp(split_method, "interleaved") == 0) {
        int start = index;
        int step = thread_count;
        for(int i = 0; i < height; i++) {
            for(int j = start; j < width; j += step) {
                histograms[index][image[i][j]]++;
            }
        }
    }

    clock_t end = clock();

    int* time = (int*) malloc(sizeof(int));
    *time = time_diff(start, end);
    pthread_exit((void *) time);
}

void save_result_to_file() {
    FILE* out = fopen(output_file, "w");

    for(int i = 0; i < MAX_VAL; i++) {
        int val = 0;
        for(int j = 0; j < thread_count; j++) val += histograms[j][i];
        fprintf(out, "%d %d\n", i, val);
    }

    fclose(out);

}

void free_memory() {
    for(int i = 0; i < height; i++) free(image[i]);
    free(image);

    for(int i = 0; i < thread_count; i++) free(histograms[i]);
    free(histograms);
}

int main(int argc, char** argv) {
    if(argc < 5) error_exit("not enough arguments");

    thread_count = atoi(argv[1]);
    split_method = argv[2];
    input_file = argv[3];
    output_file = argv[4];

    read_image_from_file();

    histograms = (int**) calloc(thread_count, sizeof(int*));
    for(int i = 0; i < thread_count; i++)
        histograms[i] = (int*) calloc(MAX_VAL, sizeof(int*));

    clock_t start = clock();

    pthread_t* thread_ids = (pthread_t*) calloc(thread_count, sizeof(pthread_t));
    for(int i = 0; i < thread_count; i++) {
        int* index = (int*) malloc(sizeof(int)); *index = i;
        pthread_create(&thread_ids[i], NULL, calculate_histogram_part, (void*) index);
    }

    for(int i = 0; i < thread_count; i++) {
        void* return_val;
        if(pthread_join(thread_ids[i], &return_val) > 0) error_exit("thread error");
        int value =  *((int*) return_val);

        printf("Thread %d time [us]: %d\n", i + 1, value);
    }

    clock_t end = clock();

    printf("Full time [us]: %d\n", time_diff(start, end));

    save_result_to_file();

    free_memory();

    return 0;
}