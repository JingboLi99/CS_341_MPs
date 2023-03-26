/**
 * mapreduce
 * CS 341 - Spring 2023
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "mapper.h"

void mapper(const char *data, FILE *output) {
    // Create and write to temp folder to check that multiple mappers are used

    char *filename = NULL;
    asprintf(&filename, "./temp/mapper-%u.out", getpid());

    FILE *file = fopen(filename, "a");

    free(filename);
    filename = NULL;

    if (file) {
        while (*data) {
            int c = *data++;
            if (isalpha(c)) {
                c = tolower(c);
                fprintf(file, "%c: 1\n", c);
                printf("%c: 1\n", c);
            }
        }

        fclose(file);
    }
}

MAKE_MAPPER_MAIN(mapper)
