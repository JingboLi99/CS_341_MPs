/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#pragma once

/**
 * This function will be the test suite which is used to test the other interns'
 * camel_caser() implementations. You are to provide a maximum of 16 inputs,
 * where each input is less than 256 characters.
 *
 * @param camelCaser - Function pointer to the camelCaser function to be tested.
 * @param destroy - Function pointer to the corresponding destroy function for
 *                  the given camelCaser function.
 * @return 1 if the implementation is correct, and 0 if the implementation is
 *         wrong.
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **));
