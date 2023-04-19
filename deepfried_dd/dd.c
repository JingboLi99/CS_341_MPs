/**
 * deepfried_dd
 * CS 341 - Spring 2023
 */
#include "format.h"
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <sys/syscall.h>
//**GLOBALS: For status report
static struct timespec start_time, end_time; //to get elapsed time
static size_t n_fullblks;
static size_t n_ptblks;
static size_t total_copied;
// **HELPER FUNCTIONS
size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}
void sig_handler(){
    // TODO: print status report
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec)) / 1000000000.0;
    print_status_report(n_fullblks, n_ptblks, n_fullblks, n_ptblks, 
                        total_copied, elapsed_time);
}
void parseInputs(int argc, char ** argv, 
            char ** infile, bool * hasIn, char ** outfile, bool * hasOut, size_t * blk_size, 
            size_t* bCopied, size_t* in_blkskip, size_t* out_blkskip){
    int opt;
    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch (opt){
            case 'i':{
                strcpy(*infile, optarg);
                *hasIn = true;
                break;
            }case 'o':{       
                strcpy(*outfile, optarg);
                *hasOut = true;
                break;
            }case 'c':{
                char blks_copied[64];
                strcpy( blks_copied, optarg);
                *bCopied = (size_t) atoi(blks_copied);
                break;
            }case 'p':{
                char in_blks_to_skip[64];
                strcpy( in_blks_to_skip, optarg);
                *in_blkskip = (size_t) atoi(in_blks_to_skip);
                break;
            }case 'b':{
                char cblk_size[64];
                strcpy( cblk_size, optarg);
                *blk_size = (size_t) atoi(cblk_size);
                break;
            }case 'k':{
                char out_blks_to_skip[64];
                strcpy( out_blks_to_skip, optarg);
                *out_blkskip = (size_t) atoi(out_blks_to_skip);
                break;
            }case '?': //Unrecognized options: getopt auto sets opterr, exit(1)
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv) {
    // signal handler:
    signal(SIGUSR1, sig_handler);
    char * infile = calloc(128, 1); bool hasIn = false;
    char * outfile = calloc(128, 1); bool hasOut = false;
    size_t blk_size = 512; //size of each block
    size_t blks_toCopy = __INT_MAX__; //total number of blocks TO copy: defaults to max value aka entire file
    size_t in_blkskip = 0; // number of BLOCK offset from start for input file
    size_t out_blkskip = 0; // number of BLOCK offset from start for output file
    //Start time: 
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // clock_t before = clock();
    //parse inputs
    parseInputs(argc, argv, &infile, &hasIn, &outfile, &hasOut, &blk_size, &blks_toCopy, &in_blkskip, &out_blkskip);
    // fprintf(stderr, "\n%s, %d, %s, %d, %zu, %zu, %zu, %zu\n", infile, hasIn, outfile, hasOut, blk_size, blks_toCopy, in_blkskip, out_blkskip);
    fflush(stderr);
    // configure input file pointer:
    FILE * fin = stdin; //input file defaults to stdin
    if (hasIn) fin = fopen(infile, "rb");
    if (!fin){
        // printf("INPUT FAILED\n");
        print_invalid_input(infile);
        return 1;
    }
    //account for offset if its not default stdin:
    if (infile) fseek(fin, in_blkskip * blk_size, SEEK_SET);
    // configure output file pointer:
    FILE * fout = stdout; //output file defaults to stdout
    if (hasOut) fout = fopen(outfile, "wb+");
    if (!fout){
        // printf("output FAILED\n");
        print_invalid_output(outfile);
        if (fin) fclose(fin);
        return 1;
    }
    //account for offset if its not default stdout:
    if (outfile) fseek(fout, out_blkskip * blk_size, SEEK_SET);

    // configure helpers for copying data and status report:
    char buffer[blk_size]; //temp array to store information to be copied over: 1 block worth of info
    size_t toCopy = blk_size * blks_toCopy; //number of bytes to be copied
    // printf("to copy size: %zu\n", toCopy);
    n_fullblks = 0;
    n_ptblks = 0;
    total_copied = 0;
    //WHILE LOOP: Get blocks of data until no more data left OR end of input file reached
    while (toCopy > 0 && !feof(fin)){
        // printf("#"); fflush(stdout);
        size_t cur_toCopy = min(toCopy, blk_size);
        size_t bytes_read;
        // printf("cur to copy size: %zu\n", cur_toCopy);
        if ((bytes_read = fread(buffer, 1, cur_toCopy, fin)) > 0){ //there are still bytes to read 
            // printf("BUFFER: %s\n", buffer); fflush(stdout);
            fwrite(buffer, 1, bytes_read, fout);
            toCopy -= bytes_read;
            total_copied += bytes_read;
            if (bytes_read == blk_size) n_fullblks ++;
            else n_ptblks ++;
        }
        else{
            if (fin) fclose(fin); 
            if (fout) fclose(fout);
            break;
        }
    }
    // printf("\n");
    //Do end of copy status report:
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    print_status_report(n_fullblks, n_ptblks, n_fullblks, n_ptblks, 
                        total_copied, elapsed_time);
    if (infile) free(infile);
    if (outfile) free(outfile);
    return 0;
}