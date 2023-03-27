/**
 * password_cracker
 * CS 341 - Spring 2023
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "thread_status.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "includes/queue.h"
#include <crypt.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>


#define MAX_IN 512
static queue * taskq = NULL;
//STRUCTS:
//values and data for password to crack
typedef struct toCrack{ //Need to free: each string in toCrack
    char * user;
    char *hash;
    char *prefix;
    int tocrack_len;
}toCrack;
//**HELPER FUNCTIONS
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

//**THREAD STARTING FUNCTION
void * cracker(void * tid){
    // //todo: need to free the task aftr cracking
    toCrack * cur_task = NULL; 
    int *successful_cracks = malloc(sizeof(int));
    *successful_cracks = 0;
    while ((cur_task = (toCrack *) queue_pull((queue *) taskq)) != NULL){ //check if there are any more to poll
        pthread_t cur_thread = *(int *) tid;
        v1_print_thread_start(cur_thread, cur_task->user); //print user
        double start_cpu_time = getCPUTime(); //cpu start time
        int tc = cur_task->tocrack_len; //length of the suffix to generate
        size_t psw_len = strlen(cur_task->prefix) + tc;
        //initialize suffix, include ending char
        char suf_chars[tc+1];
        // char *suf_chars = malloc(tc+1);
        // long total_iter = 1;
        suf_chars[tc] = '\0';
        for (int i = 0; i < tc; i++){
            suf_chars[i] = 'a';
            // total_iter *= 26;
        }
        
        //crypt_data bookkeeping //Question: should i initialize this for each thread or as a global variable?
        struct crypt_data cdata;
        cdata.initialized = 0;
        //iterate through all possible suffixes
        int hashCount = 0;
        bool cracked = false;
        while (suf_chars[tc-1] != '{'){
            // create full psw:
            char* psw_temp = malloc(psw_len+1); //need to free
            strcpy(psw_temp, cur_task->prefix);
            strcat(psw_temp, suf_chars);
            char hashed[13+1];
            strcpy(hashed, crypt_r( psw_temp, "xx", &cdata));
            hashCount ++;
            if (strcmp(hashed,  cur_task->hash) == 0){ //if this is the correct hash
                double total_cpu_time = getCPUTime() - start_cpu_time;
                v1_print_thread_result(cur_thread, cur_task->user, psw_temp, hashCount, total_cpu_time, 0);
                //free heap values
                free(psw_temp);
                free(cur_task->hash); free(cur_task->user); free(cur_task->prefix);
                if (cur_task) free(cur_task);
                cracked = true;
                (*successful_cracks) ++;
                break;
            }
            //Increment suffix by one character:
            suf_chars[0] = (char) (suf_chars[0] + 1);
            for (int j = 0; j < tc-1; j++){
                if (suf_chars[j] == '{'){
                    suf_chars[j] = 'a';
                    suf_chars[j+1]  = (char) (suf_chars[j+1] + 1);
                }
            }
            if (psw_temp) free(psw_temp);//free psw_temp
        }
        // for (int i = 0; i < total_iter; i++){
        //     // create full psw:
        //     char* psw_temp = malloc(psw_len+1); //need to free
        //     strcpy(psw_temp, cur_task->prefix);
        //     strcat(psw_temp, suf_chars);
        //     char hashed[13+1];
        //     strcpy(hashed, crypt_r( psw_temp, "xx", &cdata));
        //     hashCount ++;
        //     if (strcmp(hashed,  cur_task->hash) == 0){ //if this is the correct hash
        //         double total_cpu_time = getCPUTime() - start_cpu_time;
        //         v1_print_thread_result(cur_thread, cur_task->user, psw_temp, hashCount, total_cpu_time, 0);
        //         //free heap values
        //         free(psw_temp);
        //         free(cur_task->hash); free(cur_task->user); free(cur_task->prefix);
        //         if (cur_task) free(cur_task);
        //         cracked = true;
        //         (*successful_cracks) ++;
        //         break;
        //     }
        //     incrementString(suf_chars);
        // }
        // if (suf_chars) free(suf_chars);
        if (!cracked){
            double total_cpu_time = getCPUTime() - start_cpu_time;
            v1_print_thread_result(cur_thread, cur_task->user, NULL, hashCount, total_cpu_time, 1);
            free(cur_task->hash); free(cur_task->user); free(cur_task->prefix);
            if (cur_task) free(cur_task);
        }
    }
    // printf("**Thread %ld called, cracking: %d\n", pthread_self(), *successful_cracks);
    if (tid) free(tid);
    return (void *) successful_cracks;    
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    // CODE STARTS HERE
    //Initialize synchronization reqs
    taskq = queue_create(-1); //No max size set
    // Initialize all the threads:
    pthread_t threads[thread_count]; //array of thread ids
    for (size_t t = 0; t < thread_count; t++){
        int * tid = malloc(sizeof(int));
        *tid = ((int) t)+1;
        pthread_create(&threads[t], NULL, cracker, (void *) tid);
    }
    // run a loop for each line in stdin and run thread for each task
    size_t max_inlen = MAX_IN;
    ssize_t cur_readlen;
    char *curIn = NULL; 
    int n_requests = 0;
    while ((cur_readlen = getline(&curIn, &max_inlen, stdin)) != -1){
        if (curIn[cur_readlen-1] == '\n'){
            curIn[cur_readlen-1] = '\0';
        }else{
            curIn[cur_readlen] = '\0';
        }
        toCrack * cur_task = malloc(sizeof(toCrack)); //struct storing all the elements to crack
        char *fspace = strchr(curIn, ' '); // pointer to first space
        int user_len = fspace - curIn; 
        cur_task->user = getSubString(curIn, 0, user_len);
        cur_task->hash = getSubString(curIn, user_len+1, 13);
        int raw_psw_len = strlen(curIn) - (user_len + 13 + 1 + 1);
        char *raw_psw = getSubString(curIn, user_len+1+13+1, raw_psw_len);
        char * just_pref = get_prefix(raw_psw);
        int suf_len = strlen(raw_psw) - strlen(just_pref);
        if (raw_psw) free(raw_psw);
        cur_task->prefix = just_pref;
        cur_task->tocrack_len = suf_len;
        queue_push(taskq, (void *) cur_task); //Add task to thread-safe task queue
        n_requests ++;
        
    }
    if (curIn) free(curIn);
    //at the end, add an ending element for each thread to taskq:
    for (size_t n = 0; n < thread_count+1; n++){
        queue_push(taskq, NULL);
    }
    // join all worker threads:
    int res[thread_count];
    int suc_cracks = 0;
    for (size_t j = 0; j < thread_count; j++){
        void * cur_res = NULL;
        pthread_join(threads[j], &cur_res);
        res[j] = *(int *) cur_res;
        suc_cracks += res[j];
        if (cur_res) free(cur_res);
    }
    v1_print_summary(suc_cracks, (n_requests - suc_cracks));
    queue_destroy(taskq);
    return 0; 
}
