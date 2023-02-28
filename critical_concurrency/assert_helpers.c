/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include "assert_helpers.h"

void _annotated_int_assert(int expected, int actual, const char *description,
                           int line_number, int magic_return_code) {
    if (expected != actual) {
        fprintf(stderr,
                "[%s-%d]: assert %s (%d (expected) == %d (actual)) failed, on "
                "line %d.\n",
                LOG_TAG, magic_return_code, description, expected, actual,
                line_number);
        fprintf(stderr, "[%s-%d]: %s %s\n", FEEDBACK_TAG, magic_return_code,
                INCORRECT_FEEDBACK, description);
        abort();
    }
}

void _annotated_string_assert(char *expected, char *actual,
                              const char *description, int line_number,
                              int magic_return_code) {
    if (strcmp(expected, actual) != 0) {
        fprintf(stderr,
                "[%s-%d]: assert %s (%s (expected) == %s (actual)) failed, on "
                "line %d.\n",
                LOG_TAG, magic_return_code, description, expected, actual,
                line_number);
        fprintf(stderr, "[%s-%d]: %s %s\n", FEEDBACK_TAG, magic_return_code,
                INCORRECT_FEEDBACK, description);
        abort();
    }
}

void _annotated_modulo_assert(int expected, int divisor, int dividend,
                              const char *description, int line_number,
                              int magic_return_code) {
    if (divisor % dividend != expected) {
        fprintf(stderr,
                "[%s-%d]: assert %s (%d (expected) == %d (actual "
                "derived from %d mod %d) failed, on "
                "line %d.\n",
                LOG_TAG, magic_return_code, description, expected,
                (divisor % dividend), divisor, dividend, line_number);
        fprintf(stderr, "[%s-%d]: %s %s\n", FEEDBACK_TAG, magic_return_code,
                INCORRECT_FEEDBACK, description);
        abort();
    }
}
