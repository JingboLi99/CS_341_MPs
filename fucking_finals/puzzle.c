//author: jingbo2

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

void open_box(char* secret);
void enact_sneaky_plans();
void* smash_artwork(void* arg);
void finish(char* mesg);

void quit_handler(){
    enact_sneaky_plans(); // puzzle 3
}
int main(){
    const char *file_name = "ai_for_dogs.txt";
    int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR); //create new text file
    write(fd, "AI for Dogs", 12); //write required fax text
    fd = dup2(fd, 13);// Set 13 to also point to the fd
    lseek(fd, 0, SEEK_SET); //reposition the file offset to the front of the file
    close(3);
    signal(SIGQUIT, quit_handler); 
    open_box("North");
    //puzzle 4:
    pthread_t workers[99];
    for (int i = 0; i < 99; i++){
        pthread_create(&workers[i], NULL, smash_artwork, (void *) 0x341);
    }
    for (int i = 0; i < 99; i++){
        pthread_join(workers[i], NULL);
    }
    pthread_t last_worker;
    void * returned;
    pthread_create(&last_worker, NULL, smash_artwork, (void *) 0x341);
    pthread_join(last_worker, &returned);
    //puzzle 5
    char secret[512];
    strcpy(secret, (char *) returned);
    finish(secret);
}