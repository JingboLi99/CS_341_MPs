/**
 * utilities_unleashed
 * CS 341 - Spring 2023
 */
#pragma once

/**
 * This function gets called whenever
 * a user incorrectly uses your time program.
 */
void print_time_usage();

/**
 * This function gets called whenever
 * a user incorrectly uses your env program.
 */
void print_env_usage();

/**
 * This function gets called whenever
 * a either env or time fails to fork.
 */
void print_fork_failed();

/**
 * This function gets called whenever
 * a either env or time fails to exec.
 */
void print_exec_failed();

/**
 * This function gets called whenever your env fails to
 * change an environment variable.
 * To be more specific, you should only call this function when
 * setenv() or getenv() fails (when do they fail?).
 * Note: You are still responsible for
 * freeing all resources BEFORE calling this function.
 */
void print_environment_change_failed();

/**
 * Displays the result for your time program.
 * Feed in argv and how long the program took (in seconds).
 * Note: this should not be called on if anything failed
 * in your child process.
 */
void display_results(char **argv, double duration);
