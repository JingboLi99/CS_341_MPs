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
    // printf("Size of p1 %zu\n", sizeof(*p1));
    void *p2 = calloc(10,4);
    p2 = realloc(p2, 80);
    void * p3 = malloc(50);
    p1 = realloc(p1, 9000);
    void * p4 = "bb";
    void * p5 = "aa";
    realloc(p4, 500);
    free(p1);
    // free(p2);
    free(p3);
    free(p5);
    return 0;
}
