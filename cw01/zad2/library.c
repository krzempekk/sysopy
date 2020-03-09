#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_OPS = 100000;
const int MAX_CHARS_PER_OP = 1000000;

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

FILE* compare_files(char* file1, char* file2) {
    char command[1000] = "diff ";
    strcat(command, file1);
    strcat(command, " ");
    strcat(command, file2);
    strcat(command, " > tmp");
    system(command);

    FILE* tmp_file = fopen("tmp", "r");
    return tmp_file;
}

int save_comparision_to_block(struct main_arr* m_arr, FILE* tmp_file) {
    char* line = NULL;
    size_t len = 0;

    struct block* tmp = create_block(MAX_OPS);
    tmp->index = -1;

    while(getline(&line, &len, tmp_file) != -1) {
        if(line[0] != '>' && line[0] != '<' && line[0] != '-') {
            tmp->operations[++tmp->index] = (char*) calloc(MAX_CHARS_PER_OP, sizeof(char));
            strcpy(tmp->operations[tmp->index], line);
        } else {
            strcat(tmp->operations[tmp->index], line);
        }
    }
    tmp->index++;

    m_arr->blocks[m_arr->index++] = tmp;

    fclose(tmp_file);

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
    free(arr->blocks[block_index]->operations);
    free(arr->blocks[block_index]);
    for(int i = block_index; i < arr->index - 1; i++) {
        arr->blocks[i] = arr->blocks[i + 1];
    }
    arr->index--;
}

void remove_all_blocks(struct main_arr* arr) {
    for(int i = 0; i < arr->index; i++) {
        remove_block(arr, i);
    }
    free(arr->blocks);
    free(arr);
}