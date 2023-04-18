/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

int main() {
    // Write tests here!
    file_system *fs = open_fs("test.fs");
     //TEST READ
    off_t off = 0;
    char buf[13];
    ssize_t bytes_read = minixfs_read(fs, "/goodies/hello.txt", buf, 13, &off);
    printf("Read: %sbytesread: %zu\n", buf, bytes_read);
    //open /goodies/hello.txt with open() or fopen() and read the contents way you normally would
    //TEST CHOWN
    //TEST CHMOD
    minixfs_chmod(fs, "/goodies/hello.txt", 1234);

    close_fs(&fs);
}
