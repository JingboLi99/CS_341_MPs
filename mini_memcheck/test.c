/**
 * mini_memcheck
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    // Your tests here using malloc and free
    void *p1 = malloc(90);
    p1 = realloc(p1, 1);
    free(p1);
    return 0;
}
