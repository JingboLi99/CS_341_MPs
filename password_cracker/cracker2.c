/**
 * password_cracker
 * CS 341 - Spring 2023
 */
// #define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "thread_status.h"
#include "includes/queue.h"
// **GLOBALS
#define MAX_IN 512
pthread_barrier_t bar;
pthread_mutex_t mtx;
bool found = false;
//STRUCTS:
typedef struct toCrack{
    int local_tid;
    char * user;
    char * pref;
    char * hash;
    char * st_suf;
    long count;
    long stidx;
    
}toCrack;
typedef struct toRet{
    char * foundPsw;
    long hashes;
}toRet;
// **HELPER FUNCTIONS
//s is start index (inclusive), len is length from start index
char* getSubString(char *in, int s, int len){
    int i = 0;
    char *dest = malloc(len+1);
    dest[len] = '\0';
    for (;i < len; i++){
        dest[i] = in[i+s];
    }
    return dest;
}
//Given a known prefix with periods, return prefix string
char * get_prefix(char *raw){
    int slen = strlen(raw);
    int i = 0;
    for (; i < slen; i++){
        if (raw[i] == '.'){
            break;
        }
    }
    char *pref = getSubString(raw, 0, i); //get the known prefix (without...s)
    return pref;
}   
// **THREAD FUNCTION
void * cracker(void * args){
    //crypt_data bookkeeping //Question: should i initialize this for each thread or as a global variable?
    struct crypt_data cdata;
    cdata.initialized = 0;
    //get subtask
    toCrack *stask = (toCrack *) args;   
    pthread_t cur_thread = stask->local_tid;
    // printf("\n**REACHED HERE!\n");
    char *cur_psw = malloc(8+1); //need to free: Only not freed in thread if returned (successful crack)
    strcpy(cur_psw, stask->pref);
    strcat(cur_psw, stask->st_suf);
    free(stask->st_suf);// st_suf no longer needed: freed at each subtask
    v2_print_thread_start(cur_thread, stask->user, stask->stidx, cur_psw);
    toRet * reVal = NULL;
    bool completed = false;
    int i = 0;
    for (; i < stask->count; i++){
        if (found){
            v2_print_thread_result(cur_thread, i, 1);
            free(cur_psw);
            reVal = malloc(sizeof(toRet));
            reVal->foundPsw = NULL;
            reVal->hashes = i;
            completed = true;
            break;
            // return (void *) reVal;
        }
        char hashed[13+1];
        strcpy(hashed, crypt_r( cur_psw, "xx", &cdata));
        if (strcmp(hashed, stask->hash) == 0){ //Found correct password
            v2_print_thread_result(cur_thread, i+1, 0);
            pthread_mutex_lock(&mtx);
            found = true;
            pthread_mutex_unlock(&mtx);
            reVal = malloc(sizeof(toRet));
            reVal->foundPsw = cur_psw;
            reVal->hashes = i+1;
            completed = true;
            break;
            // return (void *) reVal;
        }
        int inc_res = incrementString(cur_psw);
        //todo: check if common prefix part kenna change
        if (!inc_res) break;
    }
    if (!completed){
        free(cur_psw);
        v2_print_thread_result(cur_thread, i+1, 2);
        reVal = malloc(sizeof(toRet));
        reVal->foundPsw = NULL;
        reVal->hashes = i+1;
    }
    pthread_barrier_wait(&bar);
    return (void *) reVal;
}
int start(size_t thread_count) {
    //Initialize syncs
    pthread_mutex_init(&mtx, NULL);
    pthread_barrier_init(&bar, NULL, thread_count+1); // barrier for all worker + main threads
    // run a loop for each line in stdin and run thread for each task
    size_t max_inlen = MAX_IN;
    ssize_t cur_readlen;
    char *curIn = NULL; 
    while ((cur_readlen = getline(&curIn, &max_inlen, stdin)) != -1){
        if (curIn[cur_readlen-1] == '\n'){
            curIn[cur_readlen-1] = '\0';
        }else{
            curIn[cur_readlen] = '\0';
        }
        char *fspace = strchr(curIn, ' '); // pointer to first space
        int user_len = fspace - curIn; 
        char* cur_user = getSubString(curIn, 0, user_len); //Need to free
        v2_print_start_user(cur_user); //format.h: PRINT USER
        double start_time = getTime();
        double start_cpu_time = getCPUTime(); //cpu start time
        char * cur_hash = getSubString(curIn, user_len+1, 13); //Need to free
        int raw_psw_len = strlen(curIn) - (user_len + 13 + 1 + 1);
        char *raw_psw = getSubString(curIn, user_len+1+13+1, raw_psw_len); //Need to free
        char * just_pref = get_prefix(raw_psw); //Need to free
        int suf_len = strlen(raw_psw) - strlen(just_pref);
        char *raw_suf = getSubString(raw_psw, raw_psw_len-suf_len,suf_len); //need to free
        if (raw_psw) free(raw_psw);
        //Split task into thread_count:
        long * st_idx = malloc(sizeof(long)); //starting index of iteration for the suffix
        *st_idx = 0;
        long * cur_ct = malloc(sizeof(long)); //number of iterations to try
        *cur_ct = 0;
        //For current task, get the starting suffix, and the count of how many
        found = false; //Reset flag
        //iterations of new suffix to try for current thread
        pthread_t threads[thread_count]; //array of thread ids
        toCrack * tasklist[thread_count];
        
        for (size_t i = 0; i < thread_count; i++){
            getSubrange(suf_len, thread_count, i+1, st_idx, cur_ct);
            setStringPosition(raw_suf, *st_idx); 
            toCrack * subtask = malloc(sizeof(toCrack)); //need to free everything here
            tasklist[i] = subtask;
            subtask->user = cur_user; //THIS IS SHARED!!
            subtask->hash = cur_hash; //THIS IS SHARED!!
            subtask->count = *cur_ct; //number of iterations this thread should try
            subtask->pref = just_pref;//THIS IS SHARED!!
            subtask->st_suf = malloc(suf_len); //Free for each subtask
            subtask->stidx = *st_idx;
            subtask->local_tid = ((int) i)+1;
            strcpy(subtask->st_suf, raw_suf);
            pthread_create(&threads[i], NULL, cracker, (void *) subtask);
        }
        pthread_barrier_wait(&bar); //main thread wait for all workers to finish
        char * psw_found = NULL;
        long total_hashes = 0;
        int suc_crack = 1; //1 means failed. 0 means successful crack
        //join finished threads:
        for (size_t i = 0; i < thread_count; i++){
            void * res = NULL;
            pthread_join(threads[i], &res);
            if (!res){
                printf("***RETURNED A NULL FOR SOME REASON!\n");
            }
            toRet * ret_res = (toRet *) res;
            total_hashes += ret_res->hashes;
            if (ret_res->foundPsw){
                suc_crack = 0;
                psw_found = ret_res->foundPsw;
            }
            if (ret_res) {free(ret_res); ret_res = NULL;}
            if (tasklist[i]) {free(tasklist[i]); tasklist[i] = NULL;} //free each toCrack
        }
        
        double total_time = getTime() - start_time;
        double total_cpu_time = getCPUTime() - start_cpu_time;
        v2_print_summary(cur_user, psw_found, total_hashes, total_time, total_cpu_time, suc_crack);
        //free shared heap values
        if (psw_found) {free(psw_found); psw_found = NULL;}
        if (raw_suf) {free(raw_suf); raw_suf = NULL;}
        //free all the heap shared by all the tasks:
        if (cur_user) {free(cur_user); cur_user = NULL;}
        if (cur_hash) {free(cur_hash); cur_hash = NULL;}
        if (just_pref) {free(just_pref); just_pref = NULL;}


    }
    if (curIn) free(curIn);

    pthread_mutex_destroy(&mtx);
    pthread_barrier_destroy(&bar);
    return 0; 
}
