#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int main(int argc, char** argv) {
    srand(time(NULL));

    int n = atoi(argv[1]), min = atoi(argv[2]), max = atoi(argv[3]);

    char* file_name = (char*) calloc(100, sizeof(char));

    FILE* tests_list = fopen("tests_list", "w+");

    for(int i = 0; i < n; i++) {
        char str[3];
        sprintf(str, "%d", i);

        strcpy(file_name, "tests/");
        strcat(file_name, str);
        strcat(file_name, "_A");
        FILE* f_a = fopen(file_name, "w+");
        fprintf(tests_list, "%s ", file_name);

        strcpy(file_name, "tests/");
        strcat(file_name, str);
        strcat(file_name, "_B");
        FILE* f_b = fopen(file_name, "w+");
        fprintf(tests_list, "%s ", file_name);

        strcpy(file_name, "tests/");
        strcat(file_name, str);
        strcat(file_name, "_C");
        fprintf(tests_list, "%s\n", file_name);

        int a_height = random_int(min, max);
        int a_width = random_int(min, max);
        int b_width = random_int(min, max);

        for(int i = 0; i < a_height; i++) {
            for(int j = 0; j < a_width; j++) {
                fprintf(f_a, "%d", random_int(-100, 100));
                fprintf(f_a, " ");
            }
            fprintf(f_a, "\n");
        }

        for(int i = 0; i < a_width; i++) {
            for(int j = 0; j < b_width; j++) {
                fprintf(f_b, "%d", random_int(-100, 100));
                fprintf(f_b, " ");
            }
            fprintf(f_b, "\n");
        }

        fclose(f_a);
        fclose(f_b);
    }

    return 0;
}