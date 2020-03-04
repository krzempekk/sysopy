#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

int main(int argc, char** argv) {
    struct main_arr* m_arr;

    if(argc <= 1) {
        printf("Pomoc:\ncreate_table size - tworzy tablicę o rozmiarze size\ncompare_pairs file1A.txt:file1B.txt file2A.txt:file2B.txt … — porównanie para plików:  file1A.txt z file1B.txt, file2A.txt z file2B.txt, itd\nremove_block index- usuń z tablicy bloków o indeksie index\nremove_operation block_index operation_index — usuń z bloku o indeksie block_index operację o indeksie operation_index\n");
        return 0;
    }

    m_arr = create_arr(atoi(argv[1]));

    for(int i = 2; i < argc; ) {
        if (strcmp(argv[i], "compare_pairs") == 0) {
            i++;
            while(i < argc && strchr(argv[i], ':') != NULL) {
                char* file1 = strtok(argv[i], ":");
                char* file2 = strtok(NULL, ":");
                compare_files(m_arr, file1, file2);
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

    return 0;
}