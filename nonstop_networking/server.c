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
#include <sys/epoll.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "./includes/vector.h"

#include "common.h"
//** DEFINITIONS
#define MAX_EVENTS 64
#define BLOCK_SIZE 2048 // read and write in 2048 Bytes blocks

//** GLOBALS:
// static vector* files;
static int serverSocket;
static int epoll_fd;
static int endSession;
static const size_t MESSAGE_SIZE_DIGITS = 8;
static char * tdir; //need tro free
// ** HELPER FUNCTIONS
void sigpipe_handler() {
    // do nothing 
}

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
    ssize_t read_bytes = 
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
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
void close_client(int cfd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
    shutdown(cfd, SHUT_RDWR);
    close(cfd);

}
// signal handler to close server and clear all server memory
void close_server(){
    endSession = 1;
    //Close epoll instance
    close(epoll_fd);
    //Close server socket
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);
    //Remove temp directory
    DIR* d = opendir(tdir);
    if (d != NULL) {
        struct dirent* ptr;
        while ((ptr = readdir(d))) {
          // skip "." amd ".."
          if (!strcmp(ptr -> d_name, ".") || !strcmp(ptr -> d_name, "..")) continue;
          
          char path[strlen(tdir) + strlen(ptr -> d_name) + 1];
          sprintf(path, "%s/%s", tdir, ptr -> d_name);
          int result = unlink(path);
          if (result != 0) perror("remove file");
      }
      closedir(d);
    } else puts("fail");
    rmdir(tdir);
	exit(1);
}
void cleanup(){

}
// Early declare response functions:
void send_error_msg(int cfd, const char * msg);
int set_nonblocking(int sock){
    int flags = fcntl(sock, F_GETFL, 0); //get the oiginal flags of the socket
    if (flags == -1) {
        perror("Error getting socket flags");
        return -1;
    }
    flags |= O_NONBLOCK; //add the non blocking flag
    if (fcntl(sock, F_SETFL, flags) == -1) { //set the new flag as the socket's flag
        perror("Error setting socket to non-blocking");
        return -1;
    }

    return 0;
}
//Read client input until first \n character
char * get_firstline(int client_fd){
    char * fline = malloc(1024); //max header size is 1024 bytes long. Need to free
    strcpy(fline, "");
    ssize_t bytes_read = 0;
    char buffer[2];
    while (bytes_read < 1024){
        ssize_t cur_read;
        if ((cur_read = read_all_from_socket(client_fd, buffer, 1)) == 1){
            buffer[1] = '\0';
            if (buffer[0] == '\n'){
                break; //we have reached the end of the line
            }else{
                strcat(fline, buffer);
                bytes_read += cur_read;
            }
        }
        else{
            // fprintf(stderr, "HEADER ERROR: Unable to read request header");
            return NULL;
        }
    }
    if (bytes_read >= 1024){
        // fprintf(stderr, "HEADER ERROR: header size larger than 1024");
        return NULL;
    }
    return fline;
}
//early declare of the 4 command handlers:
int list_handler(int cfd);
int get_handler(char * filename, int cfd);
int put_handler(char * filename, int cfd);
int del_handler(char * filename, int cfd);

int command_handler(char * cmd, char * filename, int client_fd){

    if (strcmp(cmd, "LIST") == 0) {
        return list_handler(client_fd);
    }
    else if (strcmp(cmd, "GET") == 0 && filename) {
        return get_handler(filename, client_fd);
    }
    else if (strcmp(cmd, "PUT") == 0 && filename){
        return put_handler(filename, client_fd);
    }
    else if (strcmp(cmd, "DELETE") == 0 && filename){
        return del_handler(filename, client_fd);
    }
    else{
        print_invalid_response();
        send_error_msg(client_fd, err_bad_request);
        return -1;
    }

}
// Given a valid connection with the client_fd, process the request and do required actions:
int process_client(int client_fd){
    char * fline = get_firstline(client_fd);// need to free fline
    if (!fline || strlen(fline) < 4){
        print_invalid_response();
        send_error_msg(client_fd, err_bad_request);
        return -1;
    }
    //Find the space, if any:
    char * space = strchr(fline, ' ');
    if (space == NULL){ //no space -> command should be list
        if (strcmp(fline, "LIST") == 0){
            return command_handler("LIST", NULL, client_fd);
        }else{
            print_invalid_response();
            send_error_msg(client_fd, err_bad_request);
            if (fline) free(fline);
            return -1;
        }
    }
    else{ // get command (GET, PUT, DELETE) and filename
        int space_idx = (int) (space - fline);
        int left_len = space_idx; // length of the command
        // int right_len = strlen(fline) - space_idx - 1; // length of the filename
        char* filename = malloc(1024); //need to free
        char* command = malloc(8); //need to free
        //copy the strng left of space:
        strncpy(command, fline, left_len);
        command[left_len] = '\0'; // have to add the ending char
        //copy string right of space:
        strcpy(filename, space+1); // already includes ending char
        command_handler(command, filename, client_fd);
        //free malloced
        if (filename) free(filename);
        if (command) free(command);
    }  
    if (fline) free(fline);
    return 0;
}
void run_server(char * port){
    // Create a server socket with the right configurations:
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        // fprintf(stderr, "**CLIENT ERROR: Cannot create client socket\n");
        exit(EXIT_FAILURE);
    }
    // Set socket options to SO_REUSEADDR and SO_REUSEPORT
    int optval = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
        perror("**SERVER ERROR- setsockopt SO_REUSEPORT failed: ");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("**SERVER ERROR- setsockopt SO_REUSEADDR failed: ");
        exit(EXIT_FAILURE);
    }
    // Set socket to non-blocking mode
    if (set_nonblocking(serverSocket) == -1) {
        perror("**SERVER ERROR- cannot set non blocking: ");
        exit(EXIT_FAILURE);
    }
    // Bind: Binding an abstract socket to a port and network interface
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s;
    if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0) {
        freeaddrinfo(result);
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        close_server();
        exit(EXIT_FAILURE);
    }
    // Bind the socket to the server address and port
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("**SERVER ERROR- bind failed: ");
        close_server();
        exit(EXIT_FAILURE);
    }
    //Listen:
    if (listen(serverSocket, MAX_EVENTS) < 0){
        perror("**SERVER ERROR- listen failed: ");
        close_server();
        exit(EXIT_FAILURE);
    }
    // Epoll config: 
    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("Error creating epoll instance");
        close_server();
        exit(EXIT_FAILURE);
    }
    // Add server socket to epoll instance
    struct epoll_event event;
    event.events = EPOLLIN; //We are interested in incoming connections to the EPOLL
    event.data.fd = serverSocket; //Associate a fd that triggers the event returned by epoll_wait
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) < 0) {
        perror("Error adding server socket to epoll instance: ");
        exit(1);
    }
    //declare a epoll_event array to store all incoming events
    //When an event occurs on one of the file descriptors being monitored (serverSocket), 
    // information about the event is placed into one of these structures.
    struct epoll_event inevents[MAX_EVENTS];
    struct epoll_event events_client[MAX_EVENTS]; int num_clients = 0;// added
    //Client information
    struct sockaddr_storage clientaddr; 
    clientaddr.ss_family = AF_INET;
    socklen_t clientaddrsize = sizeof(clientaddr);
    //constant running while loop
    while(endSession == 0){
        //wait for events:
        int n_active_events = epoll_wait(epoll_fd, inevents, MAX_EVENTS, -1); //current blocks indefinitely
        if (n_active_events < 0){ //Something went wrong with epoll_wait
            perror("Error in epoll_wait(): ");
            exit(EXIT_FAILURE);
        }
        // Go through each active event (events with something happening)
        // and process data from the connection.
        for (int i = 0; i < n_active_events; i++){
            if (inevents[i].data.fd == serverSocket){ // New client socket found: add to epoll set
                //Retrieve an active client file descriptor
                int new_client_fd = accept(serverSocket, (struct sockaddr *) &clientaddr, &clientaddrsize);
                if (new_client_fd < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("Error accepting connection");
                    }
                    continue;
                }
                // Set client to non-blocking:
                if (set_nonblocking(new_client_fd) < 0){
                    close(new_client_fd);
                    // fprintf(stderr, "Unable to set new client as non-blocking\n");
                    continue;
                }
                // Add client to epoll set:
                events_client[num_clients].events = EPOLLIN | EPOLLET;
                events_client[num_clients].data.fd = new_client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &events_client[num_clients]) < 0) {
                    perror("Error adding client socket to epoll instance");
                    close(new_client_fd);
                    continue;
                }
                num_clients++;
            }
            else{ //Client information received: process info
                // Handle client socket
                int client_fd = inevents[i].data.fd;
                int res = process_client(client_fd);
                close_client(client_fd);
                if (res == -1){
                    // fprintf(stderr, "***Client process error\n");
                }
            }
        }
    }
    close_server();
}

int main(int argc, char **argv) {
    // good luck!
    if (argc != 2){ // server port not provided
        print_server_usage();
        return 1;
    }
    // initialize signal handlers
    signal(SIGPIPE, sigpipe_handler);

    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0){
        perror("sigaction: ");
        return 1;
    }
    // set up temp directory:
    tdir = malloc(8); //need to free
    strcpy(tdir, "XXXXXX");
    tdir = mkdtemp(tdir);
    print_temp_directory(tdir);
    // main server function:
    run_server(argv[1]);
    cleanup();
}

int list_handler(int cfd){
    char file_list[8192]; //char arr to contain all the "[filename]\n"
    strcpy(file_list, "");
    //append all the filenames to our list in a single string
    DIR * dir = opendir(tdir);
    if (!dir){
        perror("Error opening directory\n");
        return -1;
    }
    struct dirent * cfile;
    while ((cfile = readdir(dir)) != NULL){
        strcat(file_list, cfile->d_name);
        strcat(file_list, "\n");
    }
    ssize_t filels_size = strlen(file_list); //keep track of the size of cummulative sum of "[filename]\n" sizes 
    file_list[filels_size-1] = '\0'; // remove the last nextline character
    filels_size --;
    //send header: OK\n
    char *header = "OK\n";
    ssize_t header_written = write_all_to_socket(cfd, header, strlen(header));
    //send message length
    ssize_t msg_len_written = write_message_size(filels_size, cfd);
    if (header_written == -1 || msg_len_written == -1){
        // fprintf(stderr, "LIST ERROR: Unable to write header or msg size\n");
        return -1;
    }
    //send message body
    ssize_t sent_len = write_all_to_socket(cfd, file_list, filels_size);
    if (sent_len < filels_size){
        // fprintf(stderr, "LIST ERROR: Did not write entire message to client\n");
        return -1;
    }
    return 0;
}
int get_handler(char * filename, int cfd){
    //get the path of the file to get
    char path[strlen(filename) + strlen(tdir) + 2];
    sprintf(path, "%s/%s", tdir, filename);
    //Open stat file for info:
    struct stat lfstat;
    if (stat(path, &lfstat) == -1){
        send_error_msg(cfd, err_no_such_file);
        return -1;
    }
    size_t toReadSize = lfstat.st_size; //size of file
    // WRITE PART 1: header information: ie OK\n[size]
    char headerMsg[1024];
    sprintf(headerMsg, "OK\n");// NOTE: sprintf write the nul character to the end
    size_t headerLen = strlen(headerMsg);
    ssize_t bRead_header = write_all_to_socket(cfd, headerMsg, headerLen); //write to socket
    if (bRead_header == -1){
        // printf("**GET ERROR: Unable to write header info to client\n"); fflush(stdout);
        return -1;
    }
    write_message_size(toReadSize, cfd);
    // open file to read
    FILE* lf = fopen(path, "r");
    if (!lf){ //file does not exist
        send_error_msg(cfd, err_no_such_file);
        return -1;
    }
    size_t hasRead = 0; //keep track of total bytes read
    char buffer[BLOCK_SIZE]; //temp array to store information to be copied over: 1 block worth of info
    while (toReadSize > 0 && !feof(lf)){ // Run while loop till all local file data has been written 
        size_t cur_toRead = min(toReadSize, BLOCK_SIZE);
        ssize_t bRead_cur;
        if ((bRead_cur = fread(buffer, 1, cur_toRead, lf)) > 0){ // Still have data to read
            ssize_t act_wrote = write_all_to_socket(cfd, buffer, bRead_cur);
            if (act_wrote != bRead_cur){
                // printf("**GET ERROR: Write to client failed!\n"); 
                if (lf) fclose(lf);
                return -1;
            } 
            toReadSize -= bRead_cur;
            hasRead += bRead_cur;
        }
        else break; //Nothing left to read
    }
    if (lf) fclose(lf);
    return 0;
}
int put_handler(char * filename, int cfd){
    //get the path of the file to get
    char path[strlen(filename) + strlen(tdir) + 2];
    sprintf(path, "%s/%s", tdir, filename);
    //get file size from start of message (immediately after header)
    ssize_t toReadSize = get_message_size(cfd); //Number of bytes to read from the socket
    size_t ogReadSize = toReadSize;
    if (toReadSize == -1){
        // printf("**PUT ERROR: Unable to read size of incoming client file\n");
        return -1;
    }
    FILE * lf = fopen(path, "w+"); //open the local file for writing
    if (!lf){ // Check if the local file exists or successfully created
        send_error_msg(cfd, err_no_such_file);
        // fprintf(stderr, "THIS SHOULD NOT BE CALLED\n");
        return -1;
    }
    size_t hasWritten = 0;
    char buffer[BLOCK_SIZE];
    bool success = true; //will be false if there is too little or too much data
    while (1){
        // fprintf(stderr, "*");
        if (toReadSize > 0){ //still have data we have to read
            size_t cur_toRead = min(toReadSize, BLOCK_SIZE);
            ssize_t bRead_cur; 
            if ((bRead_cur = read_all_from_socket(cfd, buffer, cur_toRead)) > 0){ // Read success
                ssize_t act_wrote = fwrite(buffer, 1, bRead_cur, lf);
                if (bRead_cur != act_wrote) printf("**PUT ERROR: Write to server local file failed\n");
                toReadSize -= bRead_cur;
                hasWritten += bRead_cur;
            }
            else {
                // printf("**PUT ERROR: Read from client failed\n");
                success = false;
                break;
            }
        }else{ //No more data to read, do read one more time to see if we can get more data than specified
            ssize_t try_more = read_all_from_socket(cfd, buffer, 1);
            if (try_more > 0){
                send_error_msg(cfd, err_bad_file_size);
                success = false;
            }
            break;
        }
    }
    if (lf) fclose(lf);
    // fprintf(stderr, "toRead: %zu hasWritten; %zu", ogReadSize, hasWritten);
    if (hasWritten < ogReadSize){
        send_error_msg(cfd, err_bad_file_size);
    }
    if (success){ // we read just nice the amount of data we wanted to read.
        return 0;
    }
    return -1;
}
int del_handler(char * filename, int cfd){
    //get the path of the file to get
    char path[strlen(filename) + strlen(tdir) + 2];
    sprintf(path, "%s/%s", tdir, filename);
    // try to open the file to see if it even exists.
    FILE * lf = fopen(path, "r");
    if (!lf){ //file does not exist
        send_error_msg(cfd, err_no_such_file);
        return -1;
    }
    fclose(lf);
    // REACH HERE: File exists, can try to delete
    int unlink_res;
    if ((unlink_res = unlink(path)) != 0){
        perror("DELETE ERROR: Unable to delete file\n");
        return -1;
    }
    return 0;
}
// Response functions
void send_error_msg(int cfd, const char * msg){
    char resp[1024];
    sprintf(resp, "ERROR\n%s\n", msg);
    int resp_len = strlen(resp);
    write_all_to_socket(cfd, resp, resp_len);
}
