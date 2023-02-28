/**
 * teaching_threads
 * CS 341 - Spring 2023
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct Reducee{
    /* data */
    int * list;
    size_t length;
    reducer reducer;
    int base_case;
} Reducee;

/* You should create a start routine for your threads. */
void * start_routine(void * to_red){
    Reducee * red = (Reducee *) to_red;
    int * res = malloc(sizeof(int));
    *res = reduce(red->list, red->length, red->reducer, red->base_case);
    free (red);
    return (void *)res;
}

int ** get_sublists(int * list, size_t nthreads, size_t list_len){
    int ** to_Ret = malloc(sizeof(int *) * nthreads);
    size_t n_chunks = list_len / nthreads; //size of each sublist
    size_t no_adds = nthreads - (list_len % nthreads); //first no_adds sublists that will not add 1 to the size of sublist, 
    int lidx = 0;
    for (size_t i = 0; i < nthreads; i++){
        int lsize = 0;
        //allocate sublist
        if (i < no_adds) lsize = n_chunks;
        else lsize = n_chunks + 1;
        int * cur_list = malloc(sizeof(int) * lsize);
        for (int j = 0; j < lsize; j++){
            cur_list[j] = list[lidx];
            lidx ++;
        }
        to_Ret[i] = cur_list;
    }
    return to_Ret;
}
int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    //split list into num_threads sizes of sublists:
    if (num_threads > list_len){
        num_threads = list_len;
    }
    int ** sublists = get_sublists(list, num_threads, list_len); //needs to free
    size_t n_chunks = list_len / num_threads; //size of each sublist
    size_t no_adds = num_threads - (list_len % num_threads); //first no_adds sublists that will not add 1 to the size of sublist, 

    pthread_t threads[num_threads]; //array to store all the threads to be created
    int results[num_threads];
    //run a loop to create all the threads
    for (size_t i = 0; i < num_threads; i++){
        //create new reducee struct
        Reducee * cur_red = malloc(sizeof(Reducee));
        cur_red->list = sublists[i];
        cur_red->reducer = reduce_func;
        cur_red->base_case = base_case;
        if (i < no_adds) cur_red->length = n_chunks;
        else cur_red->length = n_chunks + 1;
        pthread_create(&threads[i], 0, start_routine, (void *) cur_red);
    }

    for (size_t i = 0; i < num_threads; i++){
        void * res;
        pthread_join(threads[i], &res);
        results[i] = *(int *) res;
        if (res) free(res);
        if (sublists[i]) free(sublists[i]);
    }
    free(sublists);
    //do final calculation:
    int final_res = reduce(results, num_threads, reduce_func, base_case);
    return final_res;
}
