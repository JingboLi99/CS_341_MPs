/**
 * teaching_threads
 * CS 341 - Spring 2023
 */
#include "reduce.h"
#include <stdlib.h>

int reduce(int *list, size_t length, reducer reduce_func, int base_case) {
    int result = base_case;

    for (size_t i = 0; i < length; ++i) {
        result = reduce_func(result, list[i]);
    }

    return result;
}
