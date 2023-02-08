/**
 * utilities_unleashed
 * CS 341 - Spring 2023
 */
#define _DEFAULT_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "format.h"

int main(int argc, char *argv[]) {
    if (argc < 2){
        print_time_usage();
        return 1;
    }
    //start the clock
    struct timespec start, stop;
    double time_taken;
    clock_gettime( CLOCK_MONOTONIC, &start);
    

    pid_t child_id = fork();

    if (child_id > 0){
        // this is the parent
        int status;
        waitpid(child_id, &status, 0);
        if (!WEXITSTATUS(status)) {
            clock_gettime( CLOCK_MONOTONIC, &stop);
            time_taken = ( stop.tv_sec - start.tv_sec ) + (double)( stop.tv_nsec - start.tv_nsec ) / (double) 1000000000;
            display_results(argv, time_taken);
        }
        else{
            exit(1);
        }
        
        // // calculate time taken:
        // double time_taken = (double) ((end_time - start_time) / CLOCKS_PER_SEC);
        
    }else if (!child_id){
        //this is child
        
        execvp(argv[1], &argv[1]);
        print_exec_failed();
        exit(1);
    }else{
        print_fork_failed();
        exit(1);
    }
    return 0;
}
