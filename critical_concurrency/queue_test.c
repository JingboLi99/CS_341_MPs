/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s test_number return_code\n", argv[0]);
        exit(1);
    }
    printf("Please write tests cases\n");
    return 0;
}
