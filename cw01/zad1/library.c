#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_OPS = 100;
const int MAX_CHARS_PER_OP = 1000;

struct main_arr* create_arr(int blocks_count) {
    struct main_arr* tmp = malloc(sizeof(struct main_arr));
    tmp->blocks_count = blocks_count;
    tmp->index = 0;
    tmp->blocks = (struct block**) calloc(blocks_count, sizeof(struct block*));
    return tmp;
}

struct block* create_block(int operations_count) {
    struct block* tmp = malloc(sizeof(struct block));
    tmp->operations_count = operations_count;
    tmp->index = 0;
    tmp->operations = (char**) calloc(operations_count, sizeof(char*));
    return tmp;
}

int compare_files(struct main_arr* m_arr, char* file1, char* file2) {
    char command[1000] = "diff ";
    strcat(command, file1);
    strcat(command, " ");
    strcat(command, file2);
    strcat(command, " > tmp");
    system(command);

    FILE* fp = fopen("tmp", "r");
    char* line = NULL;
    size_t len = 0;

    struct block* tmp = create_block(MAX_OPS);
    tmp->index = -1;

    while(getline(&line, &len, fp) != -1) {
        if(line[0] != '>' && line[0] != '<' && line[0] != '-') {
            tmp->operations[++tmp->index] = (char*) calloc(MAX_CHARS_PER_OP, sizeof(char));
            strcpy(tmp->operations[tmp->index], line);
        } else {
            strcat(tmp->operations[tmp->index], line);
        }
    }
    tmp->index++;

    m_arr->blocks[m_arr->index++] = tmp;

    fclose(fp);

    return m_arr->index - 1;
}

int block_operations_count(struct main_arr* arr, int block_index) {
    return arr->blocks[block_index]->index;
}

void remove_operation(struct main_arr* arr, int block_index, int op_index) {
    struct block* tmp = arr->blocks[block_index];
    free(tmp->operations[op_index]);
    for(int i = op_index; i < tmp->index - 1; i++) {
        tmp->operations[i] = tmp->operations[i + 1];
    }
    tmp->index--;
}

void remove_block(struct main_arr* arr, int block_index) {
    for(int i = 0; i < arr->blocks[block_index]->index; i++) {
        free(arr->blocks[block_index]->operations[i]);
    }
    free(arr->blocks[block_index]);
    for(int i = block_index; i < arr->index - 1; i++) {
        arr->blocks[i] = arr->blocks[i + 1];
    }
    arr->index--;
}

//int main(void) {
//    struct main_arr* m_arr = create_arr(100);
//    compare_files(m_arr, "file1.txt", "file2.txt");
//    compare_files(m_arr, "file3.txt", "file4.txt");
//    remove_block(m_arr, 0);
//    printf("%d\n", block_operations_count(m_arr, 0));
//    remove_operation(m_arr, 0, 1);
//    printf("%d\n", block_operations_count(m_arr, 0));
//    remove_operation(m_arr, 0, 0);
//    printf("%d\n", block_operations_count(m_arr, 0));
//    for(int i = 0; i < m_arr->blocks[0]->index; i++) {
//        printf("%s\n", m_arr->blocks[0]->operations[i]);
//    }
//    printf("after remove\n");
//    remove_operation(m_arr, 0, 2);
//    remove_operation(m_arr, 0, 0);
//    for(int i = 0; i < m_arr->blocks[0]->index; i++) {
//        printf("%s\n", m_arr->blocks[0]->operations[i]);
//    }
//    return 0;
//}