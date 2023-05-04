/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"
//**GLOBALS:
double total_wait_time; // wait time: end_time - arrival_time - run_time
double total_turnard_time; //end_time - arrival_time
double total_resp_time; //start_time - arrival_time
double njobs;
/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
    double atime; //arrival time: time that job arrived into scheduler or is ready to be executed (if system is idle)
    double reqtime; // total running time
    double remtime; //remaining time
    double stime; // start time: cpu actually started working on it
    double priority; //from the initializer value
    double nquantas; //number of quantas ran
} job_info;

void scheduler_start_up(scheme_t s) {
    //Initialize the global priority queue:

    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // todo: Initialize globals
    total_wait_time = 0; // wait time: end_time - arrival_time - run_time
    total_turnard_time = 0;
    total_resp_time = 0;
    njobs = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    if (((job_info *)(((job *) a)->metadata))->atime < 
        ((job_info *)(((job *) b)->metadata))->atime){
            return -1;
        }
    else if (((job_info *)(((job *) a)->metadata))->atime > 
             ((job_info *)(((job *) b)->metadata))->atime){
            return 1;
        }
    return 0;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    if (((job_info *)(((job *) a)->metadata))->priority < 
        ((job_info *)(((job *) b)->metadata))->priority){
            return -1;
        }
    else if (((job_info *)(((job *) a)->metadata))->priority > 
             ((job_info *)(((job *) b)->metadata))->priority){
            return 1;
        }
    return break_tie(a, b); // tie breaker using fcfs
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    if (((job_info *)(((job *) a)->metadata))->remtime < 
        ((job_info *)(((job *) b)->metadata))->remtime){
            return -1;
        }
    else if (((job_info *)(((job *) a)->metadata))->remtime > 
             ((job_info *)(((job *) b)->metadata))->remtime){
            return 1;
        }
    return break_tie(a, b); // tie breaker using fcfs
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    if (((job_info *)(((job *) a)->metadata))->nquantas < 
        ((job_info *)(((job *) b)->metadata))->nquantas){
            return -1;
        }
    else if (((job_info *)(((job *) a)->metadata))->nquantas > 
             ((job_info *)(((job *) b)->metadata))->nquantas){
            return 1;
        }
    return break_tie(a, b); // tie breaker using fcfs
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    if (((job_info *)(((job *) a)->metadata))->reqtime < 
        ((job_info *)(((job *) b)->metadata))->reqtime){
            return -1;
        }
    else if (((job_info *)(((job *) a)->metadata))->reqtime > 
             ((job_info *)(((job *) b)->metadata))->reqtime){
            return 1;
        }
    return break_tie(a, b); // tie breaker using fcfs
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info * new_Jinfo = malloc(sizeof(job_info));
    new_Jinfo->id = job_number;
    new_Jinfo->atime = time;
    new_Jinfo->stime = -1; //Not initialized
    new_Jinfo->reqtime = sched_data->running_time;
    new_Jinfo->remtime = sched_data->running_time;
    new_Jinfo->priority = sched_data->priority;
    new_Jinfo->nquantas = 0; //Not initialized
    newjob->metadata = (void *) new_Jinfo;
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (job_evicted == NULL) return (job *) priqueue_peek(&pqueue);
    // Update metadata info:
    job_info * mtdt = (job_info *) job_evicted->metadata;
    if (mtdt->stime == -1){ //if start time has not been updated, find back from quanta
        mtdt->stime = time-1;
    }
    mtdt->remtime -= 1;
    mtdt->nquantas += 1;
    if (priqueue_size(&pqueue) >= 0) return NULL;
    // if scheme is non preemptive, continue with current job
    if (pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == RR || pqueue_scheme == SJF){
        return job_evicted;
    }
    // job * next_job = priqueue_poll(&pqueue);
    priqueue_offer(&pqueue, job_evicted);
    return priqueue_peek(&pqueue);
}

void scheduler_job_finished(job *job_done, double time) { //time is end time
    // TODO: Implement me!
    njobs ++;
    double cur_wait_tm = time - ((job_info *) job_done->metadata)->atime - ((job_info *) job_done->metadata)->reqtime;
    double cur_turnard_tm = time - ((job_info *) job_done->metadata)->atime;
    double cur_resp_tm = ((job_info *) job_done->metadata)->stime - ((job_info *) job_done->metadata)->atime;
    total_resp_time += cur_resp_tm;
    total_turnard_time += cur_turnard_tm;
    total_wait_time += cur_wait_tm;
    //clean up metadata:
    if (job_done->metadata) free(job_done->metadata);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return total_wait_time / njobs;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return total_turnard_time / njobs;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return total_resp_time / njobs;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
    printf("********* priority queue *********\n");
    for (int i = 0; i < priqueue_size(&pqueue); ++i) {
        job *curr = priqueue_poll(&pqueue);
        job_info* info = curr->metadata;
        printf("id: %d\n", info->id);
        printf("--priority: %f\n", info->priority);
        printf("--arrive time: %f\n", info->atime);
        printf("--starttime: %f\n", info->stime);
        printf("--required time: %f\n", info->reqtime);
        printf("--Number of quantas ran: %f\n", info->nquantas);
        printf("--remaining time: %f\n", info->remtime);
        priqueue_offer(&pqueue, curr);
    }

}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
