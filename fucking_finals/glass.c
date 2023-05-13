#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MILES (10)
#define DONG (11)
#define KLEAR (12)
#define ONION (13)
#define IDEA "AI for Dogs"
// compile flags: -pthread -O0 -Wall -Wextra -g -std=c99 -D_GNU_SOURCE -DDEBUG
static int stage = 1; // Complete all 4 puzzles
static int completed = 0;
static int monalisa = MILES;
static int barrier = 0;
static char* secret;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void birdie(char* mesg) {
    printf("%s\nYou failed at stage %d.\n", mesg, stage);
    exit(1);
}
static void check_completed() {
    if(! completed) {
        puts("\nYour attempt was incomplete. Try again?");
    } else {
        puts("Unbelievable. You, *subject name here,* must be the pride of *subject hometown here.*");
    }
}
static void congrats(int new_stage, char* description) {
    printf("\nDONG!\nCongrats - stage complete!\nMoving on to stage %d of 5:\n%s\n", new_stage,
    description);
    stage = new_stage;
    sleep(1);
}
static void trace(char* mesg) {
    write(1, mesg, strlen(mesg));
    write(1, "\n", 1);
}
static void check_stage(int expected) {
    if( stage != expected) {
        printf("\nThis is puzzle #%d; you need to solve puzzle #%d first\n", expected, stage);
        exit(1);
    }
}
static char* receive_fax(int fd) {
    printf("\nChecking fd %d ...\n", fd);
    FILE* f = fdopen(fd, "r");
    if( ! f) {
        birdie("Could not open fax machine; is that file descriptor valid?");
    }
    char* buffer = NULL;
    size_t capacity = 0;
    printf("Found fax machine. Waiting for a complete Fax message...\n");
    ssize_t result = getline(&buffer, &capacity, f);
    // printf("RESULT IS: %zd", result);
    fclose(f);
    if( result < 0 ) { return NULL; }
    return buffer;
}
static void check_fax() {
    trace("check_fax");
    check_stage(2);
    char* fax_message = receive_fax(ONION);
    if( ! fax_message) {
        perror("Failed to read fax idea");
        birdie("Did you even send a fax?");
    }
    printf("Fax received: '%s'\n", fax_message);
    if( strncmp(IDEA, fax_message, strlen(IDEA)) != 0) {
        birdie("... which is a bad idea. We need genius-level ideas.");
    }
    free( fax_message);
    congrats(3, "Let's make some plans - Wait for my signal then act quickly.");
    // check_timing...
    check_stage(3);
    monalisa = MILES;
    usleep(rand() % 200000);
    monalisa = DONG;
    trace("Sending signal...");
    raise(SIGQUIT); // enact plans here
    usleep(5000);
    if( monalisa != KLEAR) {
        birdie("Wrong sequence of events! We need a real detective here to handle these subtle signals.");
    }
    congrats(4, "Let's have a smashing time.");
}
// public API
void open_box(char* secret) {
    trace("open_box");
    check_stage(1);
    if( strcmp(secret,"East") && strcmp(secret, "West") && strcmp(secret, "South") ) {
        congrats(2,"Let's check the fax machine (fd 103) for Genius ideas...");
        atexit(check_completed);
        check_fax();
    }
}

void enact_sneaky_plans() {
    trace( "Enact_secret_plans");
    check_stage(3);
    if( monalisa == DONG) {
        monalisa = KLEAR;
    } else {
        birdie("Only act on my signal!");
    }
}
void* smash_artwork(void* arg) {
    check_stage(4);
    if( arg != (void*) 0x341 ) { birdie("This is not going to fly"); }
    pthread_mutex_lock(&mutex);
    int success = (++barrier) == 100;
    pthread_mutex_unlock(&mutex);
    if( ! success) {
        write(2,".",1);
        usleep(1000);
        pthread_exit(NULL);
    }
    asprintf(&secret,"You solved the Glass Onion! (%x)", rand());
    congrats(5,"Call finish with the secret message to win!");
    pthread_exit((void*) secret);
}
void finish(char* mesg) {
    trace("finish");
    check_stage(5);
    if( strcmp(secret,mesg) == 0) {
        completed = 1;
        puts( mesg );
        puts( "Puzzle Complete - thanks for playing." );
        exit(EXIT_SUCCESS);
    }
    birdie("Incorrect message");
}