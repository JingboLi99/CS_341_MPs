/**
 * charming_chatroom
 * CS 341 - Spring 2023
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#include "utils.h"

#define MAX_CLIENTS 8
#define SO_REUSEPORT 15

void *process_client(void *p);

static volatile int serverSocket;
static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//**Globals:
// struct addrinfo *res; // possible addr structures for local address
/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
    clientsCount = 0;
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
    // Create a server socket with the right configurations:
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "**CLIENT ERROR: Cannot create client socket\n");
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
    // Bind: Binding an abstract socket to a port and network interface
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s;
    if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0) {
        freeaddrinfo(result);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    // Bind the socket to the server address and port
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        perror("**SERVER ERROR- bind failed: ");
        exit(EXIT_FAILURE);
    }
    //Listen:
    if (listen(serverSocket, 16) < 0){
        perror("**SERVER ERROR- listen failed: ");
        exit(EXIT_FAILURE);
    }
    //Accept incoming connections if any
    // NOTE: the accept call returns a new file descriptor. 
    // This file descriptor is specific to a particular client. We have to use this fd instead of
    // the original server socket descriptor for the server I/O
    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[i] = -1;
    }

    struct sockaddr_storage clientaddr; //Client information
    clientaddr.ss_family = AF_INET;
    socklen_t clientaddrsize = sizeof(clientaddr);
    //thread array for all each different worker thread to work on a client
    pthread_t threads[MAX_CLIENTS];
    while (endSession == 0){
        // pthread_mutex_lock(&mutex); //need to mutex lock to access global variables clients and clientsCount
        if (clientsCount < MAX_CLIENTS){ //Check if we can accept a new connection
            // int new_clientfd = accept(serverSocket, NULL, NULL);
            // int new_clientfd = accept(serverSocket, (struct sockaddr *) &clientaddr, &clientaddrsize);
            // if (new_clientfd == -1){
            //     perror("**SERVER ERROR- Accept failed: ");
            //     exit(EXIT_FAILURE);
            // }
            //Given a new connection can be made, find the first position in clients to store the new client fd
            for (int i = 0; i < MAX_CLIENTS; i++){
                if (clients[i] == -1){ //this means the cur position is available
                    clients[i] = accept(serverSocket, (struct sockaddr *) &clientaddr, &clientaddrsize);
                    if (clients[i] == -1){
                        perror("**SERVER ERROR- Accept failed: ");
                        exit(EXIT_FAILURE);
                    }
                    // printf("Waiting for connection...\n");
                    // clients[i] = new_clientfd;
                    // printf("Connection made: client_fd=%d\n", clients[i]);
                    pthread_create(&threads[i], NULL, process_client, (void *) (intptr_t) i);
                    clientsCount++;
                    break;
                }
            }
        }
        // pthread_mutex_unlock(&mutex);
    }
    freeaddrinfo(result);

    // for (int j = 0; j < MAX_CLIENTS; j++){
    //     pthread_join(threads[j], NULL);
    // }
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
