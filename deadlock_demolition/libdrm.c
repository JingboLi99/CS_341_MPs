/**
 * deadlock_demolition
 * CS 341 - Spring 2023
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {};

drm_t *drm_init() {
    /* Your code here */
    return NULL;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    return 0;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    return;
}
