/**
 * deepfried_dd
 * CS 341 - Spring 2023
 */
#pragma once
#include <stddef.h>

/**
 * This function prints an invalid input file message for the given filename.
 */
void print_invalid_input(char *filename);

/**
 * This function prints an invalid output file message for the given filename.
 */
void print_invalid_output(char *filename);

/**
 * This function prints a status report for dd.
 * full_blocks_in: number of full blocks read
 * partial_blocks_in: number of non-full blocks read
 * full_blocks_out: number of full blocks written
 * partial_blocks_out: number of partial blocks written
 * total_bytes_copied: total number of bytes copied
 * time_elapsed: total time (in seconds) spent copying
 */
void print_status_report(size_t full_blocks_in, size_t partial_blocks_in,
                         size_t full_blocks_out, size_t partial_blocks_out,
                         size_t total_bytes_copied, double time_elapsed);