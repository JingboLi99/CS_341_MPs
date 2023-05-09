/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>

#include "common.h"
#define BLOCK_SIZE 2048 // read and write in 2048 Bytes blocks


char **parse_args(int argc, char **argv);
verb check_args(char **args);
// ** GLOBALS:
static volatile int serverSocket; //Stores the file descriptor for the server socket
static struct addrinfo *add_structs; // Array of address structs that represent our server socket, used to call connect
static const size_t MESSAGE_SIZE_DIGITS = 8;

// **HELPER FUNCTIONS
size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    ssize_t b_read = 0;
    while (b_read < (ssize_t) count){
        ssize_t cur_read = read(socket, buffer + b_read, count - b_read);
        if (cur_read == 0){
            break;
        }
        else if (cur_read > 0){
            b_read += cur_read;
        }
        else if (cur_read == -1 && errno == EINTR){
            continue;
        }
        else{
            return -1;
        }
    }
    return b_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t b_wrote = 0;
    while (b_wrote < (ssize_t) count){
        ssize_t cur_wrote = write(socket, buffer + b_wrote, count - b_wrote);
        if (cur_wrote == 0){
            break;
        }
        else if (cur_wrote > 0){
            b_wrote += cur_wrote;
        }
        else if (cur_wrote == -1 && errno == EINTR){
            continue;
        }
        else{
            return -1;
        }
    }
    return b_wrote;
}
// Read 8 bytes from the socket FD and return it as a ssize_t format
ssize_t get_message_size(int socket) {
    int64_t size;
    // char * testsize = malloc(128);
    ssize_t read_bytes = read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;
    // printf("read bytes: %zu, Read Message size: %ld\n", read_bytes, size);
    // printf("size as string: %s", testsize);
    // exit(1);
    return (ssize_t) size;
}
ssize_t write_message_size(size_t size, int socket) {
    ssize_t fixed_size = size;
    return write_all_to_socket(socket, (char*)&fixed_size , MESSAGE_SIZE_DIGITS);
}
// Get a single line of server response:
int get_oneLine(char * buf, int maxSize){
    char ch;// current character being read
    int idx = 0;
    while (idx < maxSize && read(serverSocket, &ch, 1) == 1) { // read one character at a time
        if (ch == '\n') { // if newline character is read
            buf[idx] = '\0'; // add null terminator to the buffer
            return idx;
        }
        else {
            buf[idx] = ch; // store the character in the buffer and increment the index
            idx++;
        }
    }
    return -1;
}
//Parse header and checks for 1. malformed format 2. response type 3. print error message 
int check_header_error(void){
    char responseStat[6]; //max size for the first line is ERROR\0 -> 6 
    int bRead_resp = get_oneLine(responseStat, 6); // get the response status
    if (bRead_resp < 0){
        print_invalid_response(); // Response first line is wrong
        printf("**HEADER ERROR: Response status malformed! First line issue\n");
        return -1;
    }
    if (strcmp(responseStat, "OK") == 0){
        return 0;
    }
    else if (strcmp(responseStat, "ERROR") == 0){
        char errMsg[1024];
        int bRead_resp_err = get_oneLine(errMsg, 1024);
        if (bRead_resp_err < 0){
            print_invalid_response(); // Error message line is wrong
            printf("**HEADER ERROR: Error message malformed!\n");
            return -1;       
        }
        else print_error_message(errMsg);
        return 1;
    }
    else{ // Status is neither OK or ERROR
        print_invalid_response();
        printf("**HEADER ERROR: Response status is invalid! Not OK or ERROR\n");
        return -1;
    }
}
// ** PROGRAM FUNCTIONS:
// Main SIGINT handler entry point
// 1. Free address structs for server, used during connection 
// 2. Close the file descriptor for the server socket
void close_program(int signal) {
    if (signal == SIGINT) {
        if (add_structs){
            freeaddrinfo(add_structs);
            add_structs = NULL;
        }
        shutdown(serverSocket, SHUT_RD);
        close(serverSocket);
    }
}
// Program used to connect to server, given the host DNS
int connect_to_server(const char *host, const char *port) {
    struct addrinfo hints; //hints is the constraints for the returned address structures, and results will point to the head of the linked list
    int sockfd;
    //Set constraints: Allow only IPv4, TCP
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_protocol = 0;          /* Any protocol */
    //get address structs for the server hostname
    int getAd_res;
    if ((getAd_res = getaddrinfo(host, port, &hints, &add_structs)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getAd_res));
        if (add_structs) freeaddrinfo(add_structs);
        exit(EXIT_FAILURE);
    }
    //Create client socket:
    if ((sockfd = socket(hints.ai_family , hints.ai_socktype, hints.ai_protocol)) < 0){
        perror("**CLIENT ERROR: Cannot create client socket");
        if (add_structs) freeaddrinfo(add_structs);
        exit(EXIT_FAILURE);
    }
    //make connection by looping through each addr struct until one connects (if any)
    struct addrinfo *temp = NULL; //current addr struct we are trying to connect to
    for (temp = add_structs; temp != NULL; temp = temp->ai_next){
        if (connect(sockfd, temp->ai_addr, temp->ai_addrlen) != -1){ //successful connection: we do not need to try to connect to any other adr structs
            break;
        }
    }
    //Check that connection was successful
    if (temp == NULL) {
        perror("**CLIENT ERROR: Could not connect to server");
        if (add_structs) freeaddrinfo(add_structs);
        exit(EXIT_FAILURE);
    }

    if (add_structs){
        freeaddrinfo(add_structs);
        add_structs = NULL;
    }

    return sockfd; //NOTE: CHECK FOR SEGFAULT FOR THIS (returning non heap memory)
}

// **REQUEST FUNCTIONS: GET PUT DELETE LIST
int get_handler(char * remote_file, char * local_file){
    // WRITE REQUEST TO SERVER:
    char headerMsg[1024];
    sprintf(headerMsg, "GET %s\n", remote_file);// NOTE: sprintf write the nul character to the end
    size_t headerLen = strlen(headerMsg);
    ssize_t bRead_header = write_all_to_socket(serverSocket, headerMsg, headerLen); //write to socket
    if (bRead_header == -1){
        // printf("**GET ERROR: Unable to write header info to server\n"); fflush(stdout);
        return -1;
    }
    // perform half close
    shutdown(serverSocket, SHUT_WR);
    // HANDLE SERVER RESPONSE
    int resp_res = check_header_error();
    if (resp_res != 0){
        return -1; // Malformed response or GET error
    }
    // Reach here means GET is successful
    ssize_t toReadSize = get_message_size(serverSocket); //Number of bytes to read from the socket
    size_t ogReadSize = toReadSize;
    if (toReadSize == -1){
        // printf("**GET ERROR: Unable to read size of file\n");
        return -1;
    }
    // todo: You should create the file with all permissions set (r-w-x for all users groups).
    FILE * lf = fopen(local_file, "w+"); //open the local file for writing
    if (!lf){ // Check if the local file exists or successfully created
        // printf("**GET ERROR: Local file cannot be created\n");
        exit(EXIT_FAILURE);
    }
    size_t hasWritten = 0;
    char buffer[BLOCK_SIZE];
    bool success = true; //will be false if there is too little or too much data
    while (1){
        // fprintf(stderr, "*");
        if (toReadSize > 0){ //still have data we have to read
            size_t cur_toRead = min(toReadSize, BLOCK_SIZE);
            ssize_t bRead_cur; 
            if ((bRead_cur = read_all_from_socket(serverSocket, buffer, cur_toRead)) > 0){ // Read success
                ssize_t act_wrote = fwrite(buffer, 1, bRead_cur, lf);
                if (bRead_cur != act_wrote) printf("**GET ERROR: Write to local file failed\n");
                toReadSize -= bRead_cur;
                hasWritten += bRead_cur;
            }
            else {
                // printf("**GET ERROR: Read from server failed\n");
                success = false;
                break;
            }
        }else{ //No more data to read, do read one more time to see if we can get more data than specified
            ssize_t try_more = read_all_from_socket(serverSocket, buffer, 1);
            if (try_more > 0){
                print_received_too_much_data();
                success = false;
            }
            break;
        }
    }
    if (lf) fclose(lf);
    // fprintf(stderr, "toRead: %zu hasWritten; %zu", ogReadSize, hasWritten);
    if (hasWritten < ogReadSize){
        print_too_little_data();
    }
    if (success){ // we read just nice the amount of data we wanted to read.
        return 0;
    }
    return -1;
}

int put_handler(char * remote_file, char * local_file){
    // WRITE REQUEST TO SERVER
    // Get size of local file to put
    struct stat lfstat;
    if (stat(local_file, &lfstat) == -1){
        printf("**PUT ERROR: Unable to open local stat file\n"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    size_t toReadSize = lfstat.st_size; //size of local file
    // WRITE PART 1: header information: ie PUT [filename]\n[size]
    char headerMsg[1024];
    sprintf(headerMsg, "PUT %s\n", remote_file);// NOTE: sprintf write the nul character to the end
    size_t headerLen = strlen(headerMsg);
    ssize_t bRead_header = write_all_to_socket(serverSocket, headerMsg, headerLen); //write to socket
    if (bRead_header == -1){
        printf("**PUT ERROR: Unable to write header info to server\n"); fflush(stdout);
        return -1;
    }
    write_message_size(toReadSize, serverSocket);
    FILE * lf = fopen(local_file, "r"); //open the local file
    if (!lf){ // Check if the local file exists
        printf("**PUT ERROR: Local file not found\n");
        exit(EXIT_FAILURE);
    }
    size_t hasRead = 0; //keep track of total bytes read
    char buffer[BLOCK_SIZE]; //temp array to store information to be copied over: 1 block worth of info
    while (toReadSize > 0 && !feof(lf)){ // Run while loop till all local file data has been written 
        size_t cur_toRead = min(toReadSize, BLOCK_SIZE);
        ssize_t bRead_cur;
        if ((bRead_cur = fread(buffer, 1, cur_toRead, lf)) > 0){ // Still have data to read
            // fprintf(stderr, "%s", buffer);
            ssize_t act_wrote = write_all_to_socket(serverSocket, buffer, bRead_cur);
            if (act_wrote != bRead_cur){
                printf("**PUT ERROR: Write to server failed!\n"); 
                if (lf) fclose(lf);
                return -1;
            } 
            toReadSize -= bRead_cur;
            hasRead += bRead_cur;
        }
        else break;// Nothing left to read
    }
    if (lf) fclose(lf);
    // perform half close
    shutdown(serverSocket, SHUT_WR);
    // HANDLE SERVER RESPONSE
    int resp_res = check_header_error();
    if (resp_res == 0){
        print_success(); // PUT is a success
        return hasRead;
    } 
    return -1; // Malformed response or PUT error
    
}

int delete_handler(char * remote_file){
    // WRITE REQUEST TO SERVER:
    char headerMsg[1024];
    sprintf(headerMsg, "DELETE %s\n", remote_file);// NOTE: sprintf write the nul character to the end
    size_t headerLen = strlen(headerMsg);
    ssize_t bRead_header = write_all_to_socket(serverSocket, headerMsg, headerLen); //write to socket
    if (bRead_header == -1){
        printf("**DELETE ERROR: Unable to write header info to server\n"); fflush(stdout);
        return -1;
    }
    // perform half close
    shutdown(serverSocket, SHUT_WR);
    // HANDLE SERVER RESPONSE
    int resp_res = check_header_error();
    if (resp_res != 0){
        return -1; // Malformed response or DELETE error
    }
    print_success();
    return 0;
}

int list_handler(void){
    // WRITE REQUEST TO SERVER:
    char headerMsg[] = "LIST\n";
    size_t headerLen = strlen(headerMsg);
    ssize_t bRead_header = write_all_to_socket(serverSocket, headerMsg, headerLen); //write to socket
    if (bRead_header == -1){
        printf("**LIST ERROR: Unable to write header info to server\n"); fflush(stdout);
        return -1;
    }
    // perform half close
    shutdown(serverSocket, SHUT_WR);
    // HANDLE SERVER RESPONSE
    int resp_res = check_header_error();
    if (resp_res != 0){
        return -1; // Malformed response or GET error
    }
    // Reach here means LIST is successful
    ssize_t toReadSize = get_message_size(serverSocket); //Number of bytes to read from the socket
    if (toReadSize == -1){
        printf("**LIST ERROR: Unable to read size of file\n");
        return -1;
    }
    size_t ogReadSize = toReadSize;
    // printf("read size: %zu\n", toReadSize);
    FILE * lf = stdout; // LIST writes to stdout
    size_t hasWritten = 0;
    char buffer[BLOCK_SIZE];
    bool success = true; //will be false if there is too little or too much data
    while (1){
        // fprintf(stderr, "*");
        if (toReadSize > 0){ //still have data we have to read
            size_t cur_toRead = min(toReadSize, BLOCK_SIZE);
            ssize_t bRead_cur; 
            if ((bRead_cur = read_all_from_socket(serverSocket, buffer, cur_toRead)) > 0){ // Read success
                ssize_t act_wrote = fwrite(buffer, 1, bRead_cur, lf);
                if (bRead_cur != act_wrote) printf("**LIST ERROR: Write to local file failed\n");
                toReadSize -= bRead_cur;
                hasWritten += bRead_cur;
            }
            else {
                // printf("**LIST ERROR: Read from server failed\n");
                success = false;
                break;
            }
        }else{ //No more data to read, do read one more time to see if we can get more data than specified
            ssize_t try_more = read_all_from_socket(serverSocket, buffer, 1);
            if (try_more > 0){
                print_received_too_much_data();
                success = false;
            }
            break;
        }
    }
    if (lf) fclose(lf);
    // fprintf(stderr, "toRead: %zu hasWritten; %zu\n", ogReadSize, hasWritten);
    if (hasWritten < ogReadSize){
        print_too_little_data();
    }
    if (success){ // we read just nice the amount of data we wanted to read.
        return 0;
    }
    return -1;
}

int main(int argc, char **argv) {
    // Good luck!
    char ** args = parse_args(argc, argv); //Need to free
    //Check args are valid:
    check_args(argv); //if action is returned, it has been checked to be valid 
    //Setup signal handler:
    signal(SIGINT, close_program);
    // Connect to server
    serverSocket = connect_to_server(args[0], args[1]);
    // Write request to server
    if (strcmp(args[2], "GET") == 0){
        get_handler(args[3], args[4]);
    }else if (strcmp(args[2], "PUT") == 0){
        put_handler(args[3], args[4]);
    }else if (strcmp(args[2], "DELETE") == 0){
        delete_handler(args[3]);
    }else if (strcmp(args[2], "LIST") == 0){
        list_handler();
    }else{
        print_client_help();
    }
    free(args);
    close_program(SIGINT);
    return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
