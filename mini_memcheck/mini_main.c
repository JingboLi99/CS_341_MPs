/**
 * mini_memcheck
 * CS 341 - Spring 2023
 */
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * This file implements the main function for ./mini_memcheck.
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage:   %s <program> [<args> ...]\n", argv[0]);
        printf("example: %s ./test\n", argv[0]);
        return 1;
    }

#ifdef DEBUG
    const char *library = "/mini_memcheck-debug.so";
#else
    const char *library = "/mini_memcheck.so";
#endif

    char current[PATH_MAX + 1] = "./mini_memcheck";
    readlink("/proc/self/exe", current, PATH_MAX);

    char preload[PATH_MAX + 1] = "LD_PRELOAD=";
    strncat(preload, dirname(current), PATH_MAX - strlen(preload));
    strncat(preload, library, PATH_MAX - strlen(preload));

    char *env[] = {preload, NULL};
    execvpe(argv[1], argv + 1, env);

    perror(argv[1]);
    return 2;
}
