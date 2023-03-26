/**
 * password_cracker
 * CS 341 - Spring 2023
 */
/**
 * format.c - implementation of functions defined in format.h
 * DO NOT MODIFY THIS FILE, OTHERWISE YOU RISK THE CHANCE OF FAILING THIS MP
 */
#include "format.h"
#include <assert.h>
#include <stdio.h>

void v1_print_thread_start(int threadId, char *username) {
    printf("Thread %d: Start %s\n", threadId, username);
}

void v1_print_thread_result(int threadId, char *username, char *password,
                            int hashCount, double timeElapsed, int result) {
    if (result == 0) {
        printf("Thread %d: Password for %s is %s (%d hashes in %.2f seconds)\n",
               threadId, username, password, hashCount, timeElapsed);
    } else if (result == 1) {
        printf("Thread %d: Password for %s not found (%d hashes in %.2f "
               "seconds)\n",
               threadId, username, hashCount, timeElapsed);
    }
}

void v1_print_summary(int numRecovered, int numFailed) {
    printf("%d passwords recovered, %d failed.\n", numRecovered, numFailed);
}

void v2_print_start_user(char *username) {
    printf("Start %s\n", username);
}

void v2_print_thread_start(int threadId, char *username, long offset,
                           char *startPassword) {
    printf("Thread %d: Start %s at %ld (%s)\n", threadId, username, offset,
           startPassword);
}

void v2_print_thread_result(int threadId, int hashCount, int result) {
    assert(result >= 0 && result <= 2);
    if (result == 0) {
        printf("Thread %d: Stop after %d iterations (found)\n", threadId,
               hashCount);
    } else if (result == 1) {
        printf("Thread %d: Stop after %d iterations (cancelled)\n", threadId,
               hashCount);
    } else {
        printf("Thread %d: Stop after %d iterations (end)\n", threadId,
               hashCount);
    }
}

void v2_print_summary(char *username, char *password, int hashCount,
                      double timeElapsed, double totalCPUTime, int result) {
    assert(result >= 0 && result <= 2);

    if (result == 0) {
        printf("Password for %s is %s (%d hashes in %.2f seconds)\n", username,
               password, hashCount, timeElapsed);
    } else if (result == 1) {
        printf("Password for %s not found (%d hashes in %.2f seconds)\n",
               username, hashCount, timeElapsed);
    }
    printf("Total CPU time: %.2f seconds.\n", totalCPUTime);
    printf("CPU usage: %.2fx\n", totalCPUTime / timeElapsed);
}
