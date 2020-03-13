#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include <unistd.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <ctype.h>
#include <time.h>

double time_diff(clock_t t1, clock_t t2){
    return ((double)(t2 - t1) / sysconf(_SC_CLK_TCK));
}

void write_result(clock_t start, clock_t end, struct tms* t_start, struct tms* t_end){
    printf("\tREAL_TIME: %f\n", time_diff(start,end));
    printf("\tUSER_TIME: %f\n", time_diff(t_start->tms_cutime, t_end->tms_cutime));
    printf("\tSYSTEM_TIME: %f\n", time_diff(t_start->tms_cstime, t_end->tms_cstime));
}

int main(int argc, char** argv) {
    struct main_arr* m_arr;

    if(argc <= 1) {
        printf("Pomoc:\ncreate_table size - tworzy tablicę o rozmiarze size\ncompare_pairs file1A.txt:file1B.txt file2A.txt:file2B.txt … — porównanie para plików:  file1A.txt z file1B.txt, file2A.txt z file2B.txt, itd\nremove_block index- usuń z tablicy bloków o indeksie index\nremove_operation block_index operation_index — usuń z bloku o indeksie block_index operację o indeksie operation_index\n");
        return 0;
    }

    struct tms* tms_before = calloc(1, sizeof(struct tms));
    struct tms* tms_after = calloc(1, sizeof(struct tms));
    clock_t time_before = 0, time_after = 0;

    int arr_size = atoi(argv[1]);
    m_arr = create_arr(arr_size);

    time_before = times(tms_before);

    for(int i = 2; i < argc; ) {

        if (strcmp(argv[i], "compare_pairs") == 0) {
            i++;
            while(i < argc && strchr(argv[i], ':') != NULL) {
                char* file1 = strtok(argv[i], ":");
                char* file2 = strtok(NULL, ":");
                FILE* tmp_file = compare_files(file1, file2);
                save_comparision_to_block(m_arr, tmp_file);
                i++;
            }
        } else if (strcmp(argv[i], "remove_block") == 0) {
            int block_index = atoi(argv[i + 1]);
            remove_block(m_arr, block_index);
            i += 2;
        } else if (strcmp(argv[i], "remove_operation") == 0) {
            int block_index = atoi(argv[i + 1]), operation_index = atoi(argv[i + 2]);
            remove_operation(m_arr, block_index, operation_index);
            i += 3;
        } else {
            printf("Error, bad argument\n");
            return 1;
        }

    }

    time_after = times(tms_after);
    write_result(time_before, time_after, tms_before, tms_after);

    remove_all_blocks(m_arr);
    return 0;
}