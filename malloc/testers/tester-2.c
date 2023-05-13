/**
 * malloc
 * CS 341 - Spring 2023
 */
#include "tester-utils.h"

#define TOTAL_ALLOCS 5 * M

int main(int argc, char *argv[]) {
    malloc(1);

    int i;
    int **arr = malloc(TOTAL_ALLOCS * sizeof(int *));
    if (arr == NULL) {
        fprintf(stderr, "Memory failed to allocate!\n");
        return 1;
    }
    fprintf(stderr, "HERE 1\n");
    for (i = 0; i < TOTAL_ALLOCS; i++) {
        arr[i] = malloc(sizeof(int));
        if (arr[i] == NULL) {
            fprintf(stderr, "Memory failed to allocate!\n");
            return 1;
        }

        *(arr[i]) = i;
    }
    fprintf(stderr, "HERE 2\n");
    for (i = 0; i < TOTAL_ALLOCS; i++) {
        if (*(arr[i]) != i) {
            fprintf(stderr, "Memory failed to contain correct data after many "
                            "allocations!\n");
            return 2;
        }
    }
    fprintf(stderr, "HERE 3\n");
    for (i = 0; i < TOTAL_ALLOCS; i++)
        free(arr[i]);

    free(arr);
    fprintf(stderr, "Memory was allocated, used, and freed!\n");
    fprintf(stderr, "HERE 4\n");
    return 0;
}
