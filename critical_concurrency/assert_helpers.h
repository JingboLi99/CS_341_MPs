/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *LOG_TAG = "Log";
static const char *FEEDBACK_TAG = "Feedback";
static const char *INCORRECT_FEEDBACK = "Incorrect ";

void _annotated_int_assert(int expected, int actual, const char *description,
                           int line_number, int magic_return_code);
void _annotated_string_assert(char *expected, char *actual,
                              const char *description, int line_number,
                              int magic_return_code);
void _annotated_modulo_assert(int expected, int dividend, int divisor,
                              const char *description, int line_number,
                              int magic_return_code);

#define int_assert(exp, act, des) \
    _annotated_int_assert(exp, act, des, __LINE__, magic_return_code)
#define string_assert(exp, act, des) \
    _annotated_string_assert(exp, act, des, __LINE__, magic_return_code)
#define modulo_assert(exp, dividend, divisor, des)                  \
    _annotated_modulo_assert(exp, dividend, divisor, des, __LINE__, \
                             magic_return_code)
