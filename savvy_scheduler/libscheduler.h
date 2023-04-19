/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#pragma once
#include "libpriqueue/libpriqueue.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Struct that holds information for creating a new job
 */
typedef struct {
    double running_time;
    double priority;
} scheduler_info;

/**
 * Context that represents a thread
 * If you want to know more, each of these variable names are the names of
 * registers that represent this process. Look up what these registers do in
 * x86.
 */
struct jobctx {
    uint64_t rsp;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
};

/**
 * State of a thread
 */
enum jobstate {
    Unused,
    Running,
    Ready,
};

/**
 * Struct that defines a thread.
 * In the code you write, you should NOT modify any field except metadata
 */
typedef struct {
    struct jobctx ctx;
    enum jobstate state;
    void (*work)(void);
    void *stack_start;
    void *metadata; // This is where job_info should go!
} job;

/**
 * Constants which represent the different scheduling algorithms
 */
typedef enum {
    FCFS,  // First Come First Served
    PPRI,  // Preemptive Priority
    PRI,   // Priority
    PSRTF, // Preemptive Least Remaining Time First
    RR,    // Round Robin
    SJF    // Shortest Job First
} scheme_t;

/**
 * Globals used in the scheduler library
 */
priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;

/*
 * The following comparers can return -1, 0, or 1 based on the following:
 *  -1 if 'a' comes before 'b' in the ordering.
 *  1 if 'b' comes before 'a' in the ordering.
 *  0 if break_tie() returns 0.
 *
 * Note: if 'a' and 'b' have the same ordering, then return whatever break_tie()
 * returns.
 *
 * Since you'll be pushing jobs on the queue, the void* parameters to the
 * comparers should be casted to job* so you can access the data.
 *
 * For the priority comparers, lower priority runs first
 *
 * Note that psrtf checks the remaining time for the job, vs shortest job
 * first which will always check the total running time.
 */

/**
 * Comparer for First Come First Serve
 */
int comparer_fcfs(const void *a, const void *b);

/**
 * Comparer for Premptive Priority
 */
int comparer_ppri(const void *a, const void *b);

/**
 * Comparer for Priority
 */
int comparer_pri(const void *a, const void *b);

/**
 * Comparer for Preemptive Shortest Remaining Time First
 */
int comparer_psrtf(const void *a, const void *b);

/**
 * Comparer for Round Robin
 */
int comparer_rr(const void *a, const void *b);

/**
 * Comparer for Shortest Job First
 */
int comparer_sjf(const void *a, const void *b);

/**
 * Initalizes the scheduler. This function has been implemented for you, but
 * you can extend this function and add any additional setup required.
 *
 * Notes:
 * - This function needs to be called first for any scheduler simulations.
 * - Calling this function multiple times result in undefined behavior.
 * - Calling this function with an invalid scheme_t results in undefined
 *   behavior.
 *
 * @param s the scheduling scheme that should be used. This value will be one
 *          of the six enum values of scheme_t defined above.
 */
void scheduler_start_up(scheme_t s);

/**
 * Called when a new job arrives. You will need to populate newjob->metadata and
 * add the job into the priority queue.
 * Hint: Assign the metadata to point to a struct.
 *
 * Notes:
 * - You will need to use the information in stats.
 * - You should not assign a pointer to point to stats, since stats may be
 *   allocated on the stack.

 * @param job a pointer to the job that is going to be added into the scheduler.
 * @param job_number a UID (unique identifier) for the job.
 * @param time the time that the job arrived into the scheduler.
 * @param stats scheduling attributes for the new job.
 */
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *stats);

/**
 * Called when a job has completed execution.
 *
 * This function should clean up the metadata and possibly collect information
 * from the thread. Do NOT free job_done.
 *
 * @param job_done pointer to the job that has recenty finished.
 * @param time the current time.
 */
void scheduler_job_finished(job *job_done, double time);

/**
 * This function is called when the quantum timer has expired. It will be called
 * for every scheme.
 *
 * If the last running thread has finished or there were no threads previously
 * running, job_evicted will be NULL.
 *
 * You should return the job* of the next thread to be run. If
 * - there are no waiting threads, return NULL.
 * - the current scheme is non-preemptive and job_evicted is not NULL, return
 *   job_evicted.
 * - the current scheme is preemptive and job_evicted is not NULL, place
 *   job_evicted back on the queue and return the next job that should be ran.
 *
 * @param job_evicted job that is currently running (NULL if no jobs are
 *                    running).
 * @param time the current time.
 * @return pointer to job that should be scheduled, NULL if there are no more
           jobs.
 */
job *scheduler_quantum_expired(job *job_evicted, double time);

/**
 * Returns the average waiting time of all jobs scheduled by your scheduler.
 *
 * Notes:
 * - This function will only be called after all jobs that have arrived have
 *   completed.
 *
 * @return the average waiting time of all jobs scheduled.
 */
double scheduler_average_waiting_time();

/**
 * Returns the average turnaround time of all jobs scheduled by your scheduler.
 *
 * Notes:
 * - This function will only be called after all jobs that have arrived have
 *   completed.
 *
 * @return the average turnaround time of all jobs scheduled.
 */
double scheduler_average_turnaround_time();

/**
 * Returns the average response time of all jobs scheduled by your scheduler.
 *
 * Notes:
 * - For preemptive scheduling algorithms, use the first time a job was started
 *   as the job's start time.
 * - This function will only be called after all jobs that have arrived have
 *   completed
 *
 * @return the average response time of all jobs scheduled.
 */
double scheduler_average_response_time();

/**
 * Free any memory associated with your scheduler.
 *
 * Assumptions:
 * - This function will be the last function called in your library.
 */
void scheduler_clean_up();

/**
 * This function may print out any debugging information you choose. This
 * function will be called by the simulator after every call the simulator
 * makes to your scheduler.
 *
 * In our provided output, we have implemented this function to list the jobs in
 * the order they are to be scheduled. Furthermore, we have also listed the
 * current state of the job (either running on the processor or idle). For
 * example, if we have a non-preemptive algorithm and job(id=4) has began
 * running, job(id=2) arrives with a higher priority, and job(id=1) arrives with
 * a lower priority, the output in our sample output will be:
 *
 *   2(-1) 4(0) 1(-1)
 *
 * This function is not required and will not be graded. You may leave it
 * blank if you do not find it useful.
 */
void scheduler_show_queue();
