/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */
#include "format.h"

// Error message sent by the server when it receives a malformed request
const char *err_bad_request = "Bad request\n";

// Error message sent by the server when the client sends too little or too much
// data
const char *err_bad_file_size = "Bad file size\n";

// Error message sent by the sever when a client tries to GET or DELETE a non
// existent file
const char *err_no_such_file = "No such file\n";

void print_client_usage() {
    printf("./client <host>:<port> <method> [remote] [local]\n \
        <host>\t\tAddress to conenct to.\n \
        <port>\t\tPort to set up connection on.\n \
        <method>\tMethod of request to send.\n \
        [remote]\tOptional argument refering to remote filename\n \
        [local]\tOptional argument refering to file on local system\n \
        If <method> is any PUT, GET, or DELETE then [remote] and/or [local] are required\n");
}

void print_client_help() {
    print_client_usage();
    printf("Methods:\n \
        LIST\t\t\tRequests a list of files on the server.\n \
        PUT <remote> <local>\tUploads <local> file to serve as filename <remote>.\n \
        GET <remote>\t\tDownloads file named <remote> from server.\n \
        DELETE <remote>\tDeletes file named <remote> on server.\n");
}

void print_connection_closed() {
    printf("Connection closed\n");
}

void print_error_message(char *err) {
    printf("%s\n", err);
}

void print_invalid_response() {
    printf("Invalid response\n");
}

void print_received_too_much_data() {
    printf("Received too much data\n");
}

void print_too_little_data() {
    printf("Received too little data\n");
}

void print_success() {
    printf("DELETE/PUT successful\n");
}

void print_temp_directory(char *temp_directory) {
    fprintf(stdout, "%s\n", temp_directory);
}

void print_server_usage(void) {
    fprintf(stderr, "./server <port>\n");
}
