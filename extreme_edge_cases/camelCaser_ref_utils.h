/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#pragma once

/**
 * The two functions below are provided to you to help you understand
 * how camelCase works. They are NOT intended to be used in your test cases,
 * nor should you use these functions to test your camelCase implementation.
 */

/**
 * This function takes in an input string, converts it to camelCase, and
 * prints it out onto stdout. Note that special characters may not display
 * correctly on the terminal. For such cases, you are recommended to use
 * the second function below.
 *
 * @param input - The C-string to be camelCased.
 */
void print_camelCaser(char *input);

/**
 * This function takes an input C-string as well as the camelCased input
 * of the input C-string. Checks if the supplied output is the correct
 * camelCase of the given input. This function is useful when you expect
 * non-printable characters in your output.
 *
 * @param input - The C-string to be camelCased.
 * @param output - The camelCased input, formatted exactly as it would be in
 *                 your camel_caser() function.
 * @return 1 if the supplied output is correct, and 0 otherwise.
 */
int check_output(char *input, char **output);
