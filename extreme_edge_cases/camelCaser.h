/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#pragma once

/**
 * Takes an input string, and returns the camelCased output. The formal
 * description of camelCase is available in the assignment's documentation.
 * This function allocates memory for the array, as well as each individual
 * C-string. These allocations will be freed by destroy().
 *
 * This function is supposed to work for any valid C-strings of any length,
 * which can contain any combination of ASCII characters.
 *
 * @param input_str - The C-string that needs to be camelCased.
 * @return A NULL terminated array of C-strings, which contains the camelCased
 *         input. Each C-string contains exactly one camelCased sentence.
 */
char **camel_caser(const char *input_str);

/**
 * Destroys the camelCased output returned by the camel_caser() function.
 *
 * @param result - The output from camel_caser() to be freed
 */
void destroy(char **result);
