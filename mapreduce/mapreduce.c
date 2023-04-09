/**
 * mapreduce
 * CS 341 - Spring 2023
 */
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    //Process input arguments:
    if (argc != 6){
        printf("Wrong argument format: \n");
        printf("$ ./mapreduce <input_file> <output_file> <mapper_executable> <reducer_executable> <mapper_count>\n");
    }
    char * infile = argv[1];
    char * outfile = argv[2];
    char * mapper = argv[3];
    char * reducer = argv[4];
    int map_count = atoi(argv[5]);
    // int fds[map_count][2]; //initialize a file des for each pipe
    int * fds[map_count];
    for (int i = 0; i< map_count; i++) {
        fds[i] = malloc(sizeof(int) * 2);
        pipe(fds[i]);
    }
    // Create one input pipe for the reducer.
    int redfd[2];
    pipe(redfd);
    // Open the output file.
    int out_file = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);
    // Start all the splitter processes.
    pid_t split_pids[map_count];
    for (int i = 0; i < map_count; i++){
        split_pids[i] = fork();
        if (split_pids[i] == 0) { //child process
            close(fds[i][0]);
            char temp[64];
            sprintf(temp, "%d", i);
            dup2(fds[i][1], STDOUT_FILENO);
            execl("./splitter", "./splitter", infile, argv[5], temp, NULL);
            exit(0);
        }
    }

    //start all mapper processes
    pid_t map_children[map_count];
    for (int i = 0; i < map_count; i++) {
        close(fds[i][1]);
        map_children[i] = fork();
        if (map_children[i] == 0) {
            close(redfd[0]);
            dup2(fds[i][0], STDIN_FILENO);
            dup2(redfd[1], STDOUT_FILENO);
            execl(mapper, mapper, NULL);
            exit(0);
        }
    }
    close(redfd[1]);
    pid_t red_pid = fork();
    if (red_pid == 0) {
        dup2(redfd[0], STDIN_FILENO);
        dup2(out_file, STDOUT_FILENO);
        execl(reducer, reducer, NULL);
        exit(0);
    }
    close(redfd[0]);
    close(out_file);

    for (int i = 0; i < map_count; i++){
        int si;
        waitpid(split_pids[i], &si, 0);
    }
    //wait for all mapper processes to finish:
    for (int i = 0; i < map_count; i++){
        int si;
        close(fds[i][0]);
        waitpid(map_children[i], &si, 0);
    }
    // Wait for the reducer to finish.
    int stat;
    waitpid(red_pid, &stat, 0);
    // Print nonzero subprocess exit codes.
    if (stat) print_nonzero_exit_status(reducer, stat);
    // Count the number of lines in the output file.
    print_num_lines(outfile);
    for (int i = 0; i < map_count; i++) {
        free(fds[i]);
    }
    return 0;
}