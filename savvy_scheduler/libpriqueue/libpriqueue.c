/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>

#include "libpriqueue.h"

void priqueue_init(priqueue_t *q, int (*comparer)(const void *, const void *)) {
    q->comparer = comparer;
    q->size = 0;
    q->head = NULL;
}

int priqueue_offer(priqueue_t *q, void *ptr) {
    entry *newentry = malloc(sizeof(entry));
    newentry->value = ptr;
    newentry->next = NULL;
    q->size++;

    if (q->head == NULL) {
        q->head = newentry;
        return 0;
    }
    // before head
    if (q->comparer(newentry->value, q->head->value) < 0) {
        newentry->next = q->head;
        q->head = newentry;
        return 0;
    }
    int retval = 1;
    entry *leftentry = q->head;
    while (leftentry != NULL) {
        //---tail
        int cmpA = q->comparer(leftentry->value, newentry->value);
        if (leftentry->next == NULL) {
            leftentry->next = newentry;
            break;
        }
        //---inbetween
        int cmpB = q->comparer(newentry->value, leftentry->next->value);
        if (cmpA <= 0 && cmpB < 0) {
            newentry->next = leftentry->next;
            leftentry->next = newentry;
            break;
        }
        leftentry = leftentry->next;
        retval++;
    }
    return retval;
}

void *priqueue_peek(priqueue_t *q) {
    entry *head = q->head;
    return (head == NULL) ? NULL : head->value;
}

void *priqueue_poll(priqueue_t *q) {
    if (q->head == NULL)
        return NULL;
    entry *prevhead = q->head;
    void *retval = prevhead->value;
    q->head = prevhead->next;
    free(prevhead);
    q->size = q->size - 1;
    return retval;
}

int priqueue_size(priqueue_t *q) {
    return q->size;
}

void priqueue_destroy(priqueue_t *q) {
    while (priqueue_poll(q) != NULL)
        ;
}
