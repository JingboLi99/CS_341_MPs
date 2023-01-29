/**
 * vector
 * CS 341 - Spring 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    return NULL;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    return NULL;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    return -1;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    return NULL;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    return NULL;
}

void sstring_destroy(sstring *this) {
    // your code goes here
}
