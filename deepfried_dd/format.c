/**
 * deepfried_dd
 * CS 341 - Spring 2023
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>

void print_invalid_input(char *filename) {
    printf("dd: failed to open '%s': %s\n", filename, strerror(errno));
}

void print_invalid_output(char *filename) {
    printf("dd: failed to open '%s': %s\n", filename, strerror(errno));
}

void print_status_report(size_t full_blocks_in, size_t partial_blocks_in,
                         size_t full_blocks_out, size_t partial_blocks_out,
                         size_t total_bytes_copied, double time_elapsed) {
    double kilobytes = total_bytes_copied / 1000.0;
    double kilobytes_per_sec = kilobytes / time_elapsed;

    printf("%zu+%zu records in\n", full_blocks_in, partial_blocks_in);
    printf("%zu+%zu records out\n", full_blocks_out, partial_blocks_out);
    printf("%zu bytes copied, ", total_bytes_copied);
    printf("%.3f s, ", time_elapsed);
    printf("%.3f kB/s\n", kilobytes_per_sec);
}