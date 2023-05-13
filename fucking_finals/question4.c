#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

void* worker_func(void * args){
    (void)args;
    printf("Current executing thread: %lu\n", pthread_self());
    int * rval = malloc(sizeof(int));
    *rval = rand() % 100;
    pthread_exit((void *) rval);
}
int main(){
    pthread_t workers[2];
    // void * results[2];

    for (int i = 0; i < 2; i++){
        pthread_create(&workers[i], NULL, worker_func, NULL);
    }
    // for (int i = 0; i < 2; i++){
    //     pthread_join(workers[i], &results[i]);
    // }

    printf("Doing some other tasks\n");
    sleep(1);
    printf("Final tasks\n");
    return 0;
}