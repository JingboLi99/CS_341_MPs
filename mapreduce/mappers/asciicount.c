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

void mapper(const char *data, FILE *output) {
    while (*data) {
        int c = *data++;
        if (isalpha(c)) {
            c = tolower(c);
            fprintf(output, "%c: 1\n", c);
        }
    }
}

MAKE_MAPPER_MAIN(mapper)
