/**
 * deadlock_demolition
 * CS 341 - Spring 2023
 */
#ifndef __LIBDRS_H__
#define __LIBDRS_H__

#include <pthread.h>

// typedef struct { pthread_mutex_t m; } drm_t;
struct drm_t;
typedef struct drm_t drm_t;

/**
 * Initialize a deadlock-resistant mutex (drm) lock.
 * The new lock should be added to a Resource Allocation Graph.
 * Attempting to initialize a drm that has already been initialized is undefined
 * behavior.
 * This function should return a pointer to allocated heap memory.
 **/
drm_t *drm_init();

/**
 * Unlocks the given drm.
 * Remove the appropriate edge from your Resource Allocation Graph.
 *
 * @return :
 *  0 if the specified thread is not able to unlock the given drm.
 *  1 if the specified thread is able to unlock the given drm.
 **/
int drm_post(drm_t *drm, pthread_t *thread_id);

/**
 * Attempts to lock the given drm with the specified thread.
 * To make this attempt, create the appropriate edge in your Resource Allocation
 *Graph and
 * check for a cycle. If no cycle exists, allow the thread to wait on the drm,
 *otherwise remove
 * the newly created edge.
 *
 * @return :
 *   0 if attempting to lock this drm with the specified thread would cause
 *deadlock.
 *   1 if the drm can be locked by the specified thread.
 **/
int drm_wait(drm_t *drm, pthread_t *thread_id);

/**
 * Destroy any resources of the given drm.
 * Remove the vertex representing this drm from the Resource Allocation Graph.
 **/
void drm_destroy(drm_t *drm);

#endif
