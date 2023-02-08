/**
 * shell
 * CS 341 - Spring 2023
 */
#include "format.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage() {
    printf("./shell -f <filename> -h <history_file>\n \
        <filename>\t\tFile to read commands from\n \
        <history_file>\t\tFile to load history from and/or write history to\n");
}

void print_command(const char *command) {
    printf("%s\n", command);
}

void print_script_file_error() {
    printf("Unable to open script file!\n");
}

void print_history_file_error() {
    printf("Unable to open history file!\n");
}

void print_redirection_file_error() {
    printf("Unable to open redirection file!\n");
}

void print_prompt(const char *directory, pid_t pid) {
    printf("(pid=%d)%s$ ", pid, directory);
}

void print_no_directory(const char *path) {
    printf("%s: No such file or directory!\n", path);
}

void print_command_executed(pid_t pid) {
    printf("Command executed by pid=%d\n", pid);
}

void print_fork_failed() {
    printf("Fork Failed!\n");
}

void print_exec_failed(const char *command) {
    printf("%s: not found\n", command);
}

void print_wait_failed() {
    printf("Failed to wait on child!\n");
}

void print_setpgid_failed() {
    printf("Failed to start new process group!\n");
}

void print_invalid_command(const char *command) {
    printf("Invalid command: %s\n", command);
}

void print_process_info_header() {
    printf("PID\tNLWP\tVSZ\tSTAT\tSTART\tTIME\tCOMMAND\n");
}

void print_process_info(process_info *pinfo) {
    printf("%d\t%ld\t%lu\t%c\t%s\t%s\t%s\n", pinfo->pid, pinfo->nthreads,
           pinfo->vsize, pinfo->state, pinfo->start_str, pinfo->time_str,
           pinfo->command);
}

size_t time_struct_to_string(char *buffer, size_t buffer_len,
                             struct tm *tm_info) {
    return strftime(buffer, buffer_len, "%H:%M", tm_info);
}

int execution_time_to_string(char *buffer, size_t buffer_len, size_t minutes,
                             size_t seconds) {
    return snprintf(buffer, buffer_len, "%01ld:%02ld", minutes, seconds);
}

void print_process_fd_info_header() {
    puts("FD_NO\tPOS\tREALPATH");
}

void print_process_fd_info(size_t fd_no, size_t file_pos, char *realpath) {
    printf("%lu\t%lu\t%s\n", fd_no, file_pos, realpath);
}

void print_no_process_found(int pid) {
    printf("%d: no such process\n", pid);
}

void print_stopped_process(int pid, char *command) {
    printf("%d suspended\t%s\n", pid, command);
}

void print_killed_process(int pid, char *command) {
    printf("%d killed\t%s\n", pid, command);
}

void print_continued_process(int pid, char *command) {
    printf("%d continued\t%s\n", pid, command);
}

void print_history_line(size_t index, const char *command) {
    printf("%zu\t%s\n", index, command);
}

void print_invalid_index() {
    printf("Invalid Index!\n");
}

void print_no_history_match() {
    printf("No Match!\n");
}

char *get_full_path(char *filename) {
    char *full_path = malloc(sizeof(char *) * (PATH_MAX + 1));
    realpath(filename, full_path);
    return full_path;
}
