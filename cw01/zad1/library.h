#ifndef ZAD1_LIBRARY_H
#define ZAD1_LIBRARY_H

struct block {
    int operations_count;
    int index;
    char** operations;
};

struct main_arr {
    int blocks_count;
    int index;
    struct block** blocks;
};

struct main_arr* create_arr(int blocks_count);

struct block* create_block(int operations_count);

int compare_files(struct main_arr* m_arr, char* file1, char* file2);

int block_operations_count(struct main_arr* arr, int block_index);

void remove_operation(struct main_arr* arr, int block_index, int op_index);

void remove_block(struct main_arr* arr, int block_index);

#endif //ZAD1_LIBRARY_H
