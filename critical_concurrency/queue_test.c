/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

typedef struct args {
  queue* q;
  int num;
} args;

void *pull_runner(void *var) {
    args* ag = (args*)var;
    for(int i = 0; i < ag->num; i++) {
      sleep(i);
      int* ret = queue_pull(ag->q);
      printf("%d: %d\n",i, *ret);
    }
    return NULL;
}

void *push_runner(void *var) {
    args* ag = (args*)var;
    for(int i = 0; i < ag->num; i++) {
      sleep(i);
      int* num = malloc(sizeof(int));
      *num = i;
      queue_push(ag->q,num);
    }
    return NULL;
}

int test1() {
  // test push 1 and pull one in single thread
  queue *q = queue_create(2);
  int num1 = 1;
  int num2 = 2;
  queue_push(q, &num1);
  queue_push(q, &num2);
  int *ret1 = queue_pull(q);
  printf("ret %d\n", *ret1);

  int *ret2 = queue_pull(q);
  printf("ret %d\n", *ret2);
  queue_destroy(q);
  return 0;
}

int test2() {
  // test push 1 and pull one in two threads
  pthread_t pids[2];
  queue* q = queue_create(5);

  args* push_args = (args*)malloc(sizeof(args));
  push_args->q = q;
  push_args->num = 10;

  args* pull_args = (args*)malloc(sizeof(args));
  pull_args->q = q;
  pull_args->num = 11;

  pthread_create(pids, NULL, &push_runner, (void*)push_args);
  pthread_create(pids+1, NULL, &pull_runner, (void*)pull_args);

  pthread_join(pids[0], NULL);
  pthread_join(pids[1], NULL);

  return 0;
}

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("usage: %s test_number\n", argv[0]);
        exit(1);
    }

    test1();
    test2();
    return 0;
}
