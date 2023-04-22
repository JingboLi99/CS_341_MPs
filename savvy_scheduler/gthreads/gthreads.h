/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#include "../libscheduler.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#define GTSLEEP_SIG 10

#define LOG_REG_PTR(fd, ptr, str, runtime, priority) \
    do {                                             \
        char _buff[3];                               \
        ptr_to_str((uintptr_t)ptr, _buff, 3);        \
        write(fd, str, strlen(str));                 \
        write(fd, _buff, 3);                         \
        dprintf(fd, " %f %f", runtime, priority);    \
        write(fd, " ", 1);                           \
        char _buff1[5];                              \
        double_to_str(100 * currtime(), _buff1, 5);  \
        write(fd, _buff1, 5);                        \
        write(fd, "\n", 1);                          \
    } while (false);

#define LOG_ADD_PTR(fd, ptr, str)                   \
    do {                                            \
        char _buff[3];                              \
        ptr_to_str((uintptr_t)ptr, _buff, 3);       \
        char _buff1[5];                             \
        double_to_str(100 * currtime(), _buff1, 5); \
        write(fd, str, strlen(str));                \
        write(fd, _buff, 3);                        \
        write(fd, " ", 1);                          \
        write(fd, _buff1, 5);                       \
        write(fd, "\n", 1);                         \
    } while (false);

#define SWITCH_STR "Switched to "
#define LOG_SWITCH_TO(fd, ptr) LOG_ADD_PTR(fd, ptr, SWITCH_STR)

#define REGISTER_STR "Registered "
#define LOG_REGISTER(fd, ptr, runtime, priority) \
    LOG_REG_PTR(fd, ptr, REGISTER_STR, runtime, priority)

#define ENDED_STR "Ended "
#define LOG_ENDED(fd, ptr) LOG_ADD_PTR(fd, ptr, ENDED_STR)

enum {
    StackSize = 0x400000,
};

typedef void (*gt_runner)(void);

/**
 * This function should be called before anything else in this libaray
 *
 * This function sets up the scheduler and a signal handler for SIGALRM. It is
 * undefined behavior to call any other function in gthread before this one, or
 * to set a handler for SIGALRM after calling this.
 *
 * @param s  a scheme_t detailing what scheduling algorithm to be used.
 */
void gtinit(scheme_t s);

/**
 * Start the scheduler and allow children threads to run
 */
void gtstart(void);

/**
 * Green thread analogue to pthread_create.
 *
 * It spawns a new green thread (won't start until it's actually scheduled -
 * after gtgo is called).
 *
 * @param f function to execute
 * @param sched_data scheduler_info\* to get it's scheduler attributes.
 */
int gtgo(void (*f)(void), scheduler_info *sched_data);

/**
 * Returns current job
 */
void *gtcurrjob(void);

/**
 * Yield thread and allow the next job on the queue to run
 *
 * return whether or not a context switch occured
 */
bool gtdoyield(int sig);

/**
 * Sends current thread to sleep and calls gtyield
 *
 * @param sleep_time minimum amount of time for this thread to sleep
 */
void gtsleep(double sleep_time);

/**
 * Stop current thread and allow other threads to continue
 * @param ret exit status of this thread
 */
void gtret(int ret);
