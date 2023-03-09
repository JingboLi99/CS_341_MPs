/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    struct queue *new_q = malloc(sizeof(struct queue));
    new_q->head = NULL; new_q->tail = NULL;
    new_q->size = 0; new_q->max_size = max_size;

    pthread_cond_init(&new_q->cv, NULL);
    pthread_mutex_init(&new_q->m, NULL);
    return new_q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    if (!this) return;
    // //lock to free nodes
    // pthread_mutex_lock(&this->m);
    //free all the nodes in the queue
    queue_node * cur_node = this->head;
    while (cur_node != NULL) {
        queue_node * temp = cur_node;
        cur_node = temp->next;
        free(temp);
    }
    // pthread_mutex_unlock(&this->m);
    //destroy cond and mutex
    pthread_cond_destroy(&(this->cv));
    pthread_mutex_destroy(&(this->m));
    free(this);

}

void queue_push(queue *this, void *data) {
    /* Your code here */
    if (!this) return;
    //lock to add node
    pthread_mutex_lock(&this->m);
    //check wait condition: make sure number of nodes did not exceed limit
    while ((this->max_size > 0) && (this->size >= this->max_size)){
        pthread_cond_wait(&this->cv, &this->m);
    }
    queue_node *new_node = malloc(sizeof(queue_node));
    new_node->data = data;
    new_node->next = NULL;
    //if queue is empty
    if (this->size > 0){
        queue_node * cur_tail = this->tail;
        cur_tail->next = new_node;
        this->tail = new_node;
    }
    else{
        this->head = new_node;
        this->tail = new_node;
    }
    this->size++;
    pthread_cond_broadcast(&this->cv);
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    if (!this) return NULL;
    pthread_mutex_lock(&this->m);
    //make sure there are elements in queue
    while (this->size <= 0) {
        pthread_cond_wait(&this->cv, &this->m);
    }
    queue_node * to_ret_node = this->head;
    this->head = this->head->next;
    //head and tail are the same element
    if (this->size == 1){
        this->tail = NULL;
    }
    this->size--;

    if (this->size > 0 && this->size < this->max_size) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&this->m);
    void * to_ret = to_ret_node->data;
    free(to_ret_node);
    return to_ret;
}
