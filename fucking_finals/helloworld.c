//author: jingbo2

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
// ** CASE FUNCTIONS:
int encrypt(char * infile, char * maskfile, char * cryptfile);
int decrypt(char * outfile, char * maskfile, char * crpytfile);
int helloworld();
// ** Main input function
int main(int argc, char ** argv){
    // fprintf(stderr, "n args: %d, %s, %s\n", argc, argv[0], argv[1]); fflush(stderr);
    if (argc != 4) helloworld();
    if (!strcmp(argv[0], "./encrypt")){
        return encrypt(argv[1], argv[2], argv[3]);
    }
    else if (!strcmp(argv[0], "./decrypt")){
        return decrypt(argv[1], argv[2], argv[3]);
    }
    else{
        helloworld();
    }
}

int helloworld(){
    fprintf(stdout, "Hello World");
    exit(0);
}
int encrypt(char * infile, char * maskfile, char * cryptfile){
    //check if the infile is valid, if not, print hello world and exit
    struct stat stat_buf;
    if (stat(infile, &stat_buf) != 0) helloworld(); // if file does not exist, stat return -1
    size_t inSize = stat_buf.st_size; // size of input file
    // Open files as file descriptors:
    FILE * maskfp = fopen(maskfile, "w"); //open mask file for writing to
    FILE * cryptfp = fopen(cryptfile, "w"); //open crypt file to write after masking
    int infd = open(infile, O_RDWR); //open the infile for reading and writing
    char * inmap = mmap(NULL, inSize, PROT_READ | PROT_WRITE, MAP_SHARED, infd, 0);
    if (inmap == MAP_FAILED) {
        fprintf(stderr, "Encrypt Error: could not map input file into memory\n");
        exit(1);
    }

    int randfd = open("/dev/urandom", O_RDONLY); // Open the kernel random file
    // size_t toRead = inSize; //keeps track of how much data is left to read
    //Read from input file till there is nothing left to read

    for (size_t i = 0; i < inSize; i++){
        char maskByte;
        read(randfd, &maskByte, 1); //read a single random character
        char inputByte = inmap[i];
        //Process data:
        char maskedByte = inputByte ^ maskByte;
        fputc(maskByte, maskfp); // write mask bute to maskfile
        fputc(maskedByte, cryptfp); //write masked input to cryptfile
        inmap[i] = 0;
    }
    //close the files:
    fclose(maskfp); fclose (cryptfp); close(randfd);
    // Unmap the memory region
    if (munmap(inmap, inSize) == -1) {
        fprintf(stderr, "Encrypt Error: could not unmap memory region\n");
        exit(1);
    }
    // Close the input file
    close(infd);
    return 0;
}
int decrypt(char * outfile, char * maskfile, char * crpytfile){
    //Check that the 2 files bin1 and bin2 exists (using stat) and of same size
    struct stat stat_buf_mask;
    if (stat(maskfile, &stat_buf_mask) != 0) helloworld(); // if file does not exist, stat return -1
    size_t maskfileSize = stat_buf_mask.st_size; // size of mask file
    struct stat stat_buf_crypt;
    if (stat(crpytfile, &stat_buf_crypt) != 0) helloworld(); // if file does not exist, stat return -1
    size_t cryptfileSize = stat_buf_crypt.st_size; // size of crypt file
    if (maskfileSize != cryptfileSize) helloworld(); //Compare size of 2 input files
    //open the 2 files with mmap, and open the outfile with fopen
    int maskfd = open(maskfile, O_RDWR);
    char * mask_map = mmap(NULL, maskfileSize, PROT_READ | PROT_WRITE, MAP_SHARED, maskfd, 0);
    if (mask_map == MAP_FAILED) {
        fprintf(stderr, "Decrypt Error: could not map mask file into memory\n");
        return 1;
    }
    int cryptfd = open(crpytfile, O_RDWR);
    char * crypt_map = mmap(NULL, cryptfileSize, PROT_READ | PROT_WRITE, MAP_SHARED, cryptfd, 0);
    if (crypt_map == MAP_FAILED) {
        fprintf(stderr, "Decrypt Error: could not map crypt file into memory\n");
        return 1;
    }
    FILE * outfp = fopen(outfile, "w+");
    // Read byte by byte from the 2 inputs, xor them, and write (fputc) the ouput to outfile (for loop)
    for (size_t i = 0; i < maskfileSize; i++){
        char maskbyte = mask_map[i];
        char cryptbyte = crypt_map[i];
        char unmaskedByte = maskbyte ^ cryptbyte; //xored
        fputc(unmaskedByte, outfp);
    }
    // unmap and delete the 2 input files
    if (munmap(mask_map, maskfileSize) == -1) {
        fprintf(stderr, "Decrypt Error: could not unmap mask memory region\n");
        return 1;
    }
    if (munmap(crypt_map, cryptfileSize) == -1) {
        fprintf(stderr, "Decrypt Error: could not unmap crypt memory region\n");
        return 1;
    }
    close(maskfd); close(cryptfd);
    unlink(maskfile);
    unlink(crpytfile);
    return 0; 
}
