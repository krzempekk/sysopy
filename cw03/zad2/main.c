#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

typedef struct matrix {
    char* file;
    int width;
    int height;
    int out_col_width;
} matrix;

matrix** matrices_A;
matrix** matrices_B;
matrix** matrices_C;

const int MAX_ABS_VAL = 100;
const int MAX_PATH_LEN = 100;
const int MAX_INPUT_LINES = 100;

int* get_column(matrix* m, FILE* f, int index) {
    rewind(f);
    int* column = calloc(m->height, sizeof(int));
    int i = 0;
    char row[MAX_PATH_LEN];
    while(fgets(row, MAX_PATH_LEN, f) != NULL) {
        int j = 0;
        char* elem = strtok(row, " ");
        while(elem != NULL && j < index) {
            elem = strtok(NULL, " ");
            j++;
        }
        column[i++] = atoi(elem);
    }
    return column;
}

int* get_row(matrix* m, FILE* f, int index) {
    rewind(f);
    int* row = calloc(m->width, sizeof(int));
    char buff[MAX_PATH_LEN];
    for(int i = 0; i <= index; i++) fgets(buff, MAX_PATH_LEN, f);

    int j = 0;
    char* elem = strtok(buff, " ");

    while(elem != NULL) {
        row[j++] = atoi(elem);
        elem = strtok(NULL, " ");
    }

    return row;
}

void write_column(matrix* m, FILE* f, int* column, int index) {
    flock(fileno(m->file), LOCK_EX);
    for(int i = 0; i < m->height; i++) {
        int pos = m->out_col_width * (index + i * m->width) + i;
        fseek(f, pos, 0);
        char* str = (char*)calloc(m->out_col_width, sizeof(char));
        sprintf(str, "%d", column[i]);
        int j = m->out_col_width - 1;
        while(str[j] == NULL) { str[j] = ' '; j--; }
        fwrite(str, 1, m->out_col_width, f);
        free(str);
    }
    flock(fileno(f), LOCK_UN);
}

int matrix_width(FILE* f) {
    rewind(f);
    char* buffer[MAX_PATH_LEN];
    fgets(buffer, MAX_PATH_LEN, f);
    int i = 1;
    strtok(buffer, " ");
    while(strtok(NULL, " ") != NULL) i++;
    return i;
}

int matrix_height(FILE* f) {
    rewind(f);
    char buffer[MAX_PATH_LEN];
    int i = 0;
    while(fgets(buffer, MAX_PATH_LEN, f) != NULL) i++;
    return i;
}

void multiply_matrix(int col_start, int col_end, int pair_in) {
    matrix* A = matrices_A[pair_in];
    matrix* B = matrices_B[pair_in];
    matrix* C = matrices_C[pair_in];
    FILE* f_A = fopen(A->file, "r+");
    FILE* f_B = fopen(B->file, "r+");
    FILE* f_C = fopen(C->file, "r+");

    int** m_A = (int**)calloc(A->height, sizeof(int*));
    for(int i = 0; i < A->height; i++) {
        m_A[i] = get_row(A, f_A, i);
    }
//
//    for(int i = 0; i < A->height; i++) {
//        for(int j = 0; j < A->width; j++) {
//            printf("%d ", m_A[i][j]);
//        }
//        printf("\n");
//    }

    for(int col_in = col_start; col_in <= col_end; col_in++) {
        int* m_B_col = get_column(B, f_B, col_in);
        int* m_C_col = (int*)calloc(C->height, sizeof(int));

        for(int i = 0; i < C->height; i++) {
            int val = 0;
            for(int j = 0; j < A->width; j++) {
                val += m_A[i][j] * m_B_col[j];
            }
            m_C_col[i] = val;
        }

        write_column(C, f_C, m_C_col, col_in);
        free(m_B_col);
        free(m_C_col);
    }
}

void multiply_matrices(int proc_in, int proc_count, int matrices_pair_count) {
    for(int i = 0; i < matrices_pair_count; i++) {
        int col_count = matrices_B[i]->width;
        if(proc_in < col_count) {
            int col_start = proc_in * (col_count / proc_count), col_end = (proc_in + 1) * (col_count / proc_count) - 1;
            multiply_matrix(col_start, col_end, i);
        }
    }
}

int main(int argc, char** argv) {
    int proc_count = 3;
    pid_t* children_pids = calloc(proc_count, sizeof(pid_t));
    char buffer[MAX_PATH_LEN];
    FILE* f = fopen("tests_list", "r");

    matrices_A = calloc(MAX_INPUT_LINES, sizeof(matrix*));
    matrices_B = calloc(MAX_INPUT_LINES, sizeof(matrix*));
    matrices_C = calloc(MAX_INPUT_LINES, sizeof(matrix*));

    int i = 0;
    FILE* tmp;
    while(fscanf(f, "%s", buffer) != EOF) {
        int in = i / 3;
        switch (i % 3) {
            case 0:
                matrices_A[in] = malloc(sizeof(matrix));
                tmp = fopen(buffer, "r");
                matrices_A[in]->file = (char*)calloc(MAX_PATH_LEN, sizeof(char));
                strcpy(matrices_A[in]->file, buffer);
                matrices_A[in]->width = matrix_width(tmp);
                matrices_A[in]->height = matrix_height(tmp);
                fclose(tmp);
                break;
            case 1:
                matrices_B[in] = malloc(sizeof(matrix));
                tmp = fopen(buffer, "r");
                matrices_B[in]->file = (char*)calloc(MAX_PATH_LEN, sizeof(char));
                strcpy(matrices_B[in]->file, buffer);
                matrices_B[in]->width = matrix_width(tmp);
                matrices_B[in]->height = matrix_height(tmp);
                fclose(tmp);
                break;
            case 2:
                matrices_C[in] = malloc(sizeof(matrix));
                tmp = fopen(buffer, "w+");
                matrices_C[in]->file = (char*)calloc(MAX_PATH_LEN, sizeof(char));
                strcpy(matrices_C[in]->file, buffer);
                matrices_C[in]->width = matrices_B[in]->width;
                matrices_C[in]->height = matrices_A[in]->height;

                int width = matrices_C[in]->width;
                int height = matrices_C[in]->height;
                int out_col_width = (int) ceil(log10(matrices_A[in]->width * MAX_ABS_VAL * MAX_ABS_VAL)) + 3;

                matrices_C[in]->out_col_width = out_col_width;

                char buf[10000];
                strcpy(buf, "");
                for(int i = 0; i < height; i++) {
                    for(int j = 0; j < width * out_col_width; j++) {
                        strcat(buf, "@");
                    }
                    strcat(buf, "\n");
                }
                fwrite(buf, 1, (width * out_col_width + 1) * height, tmp);
                fclose(tmp);
                break;
        }
        i++;
    }
    int matrices_pair_count = i / 3;

    fclose(f);

//    int* row= get_row(matrices_A[0], 1);
//    for(int i = 0; i < matrices_A[0]->width; i++) {
//        printf("%d ", row[i]);
//    }

//    multiply_matrices(0, 1, matrices_pair_count);


    for(int i = 0; i < proc_count; i++) {
        pid_t child_pid = fork();
        if(child_pid == 0) {
            printf("jestem procesem o indeksie %d jebac sysopy\n", i);
            multiply_matrices(i, proc_count, matrices_pair_count);
            exit(0);
        } else {
            children_pids[i] = child_pid;
        }
    }

    return 0;
}
