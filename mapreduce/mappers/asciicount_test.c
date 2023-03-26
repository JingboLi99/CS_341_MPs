/**
 * mapreduce
 * CS 341 - Spring 2023
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mapper.h"

void mapper(const char *data, FILE *file) {
    // Create and write to temp folder to check that multiple mappers are used
    // FILE *file = fopen("./temp/mapper.out", "a");
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
