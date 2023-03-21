/**
 * malloc
 * CS 341 - Spring 2023
 */
#include "tester-utils.h"

#define SIZE (2L * 1024L * M)
#define ITERS 10000

int main(int argc, char *argv[]) {
    malloc(1);
    int i;
    for (i = 0; i < 10000; i++) {
        // Write to end

        srand(rand_today());
        int r = rand() % 10;
        char *a = malloc(SIZE + r);

        if (!a){
            return 1;
        }
        verify_write(a, SIZE);
        if (!verify_read(a, SIZE)){
            return 1;
        }
        free(a);

        char *b = malloc(SIZE / 2);
        verify_write(b, SIZE / 2);

        char *c = malloc(SIZE / 4);
        verify_write(c, SIZE / 4);

        if (!b || !c){
            return 1;
        }
        if (!verify_read(b, SIZE / 2) || !verify_read(c, SIZE / 4) ||
            overlap(b, SIZE / 2, c, SIZE / 4)){
            return 1;
        }
        free(b);
        free(c);
    }

    fprintf(stderr, "Memory was allocated, used, and freed!\n");
    return 0;
}
