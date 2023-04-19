/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#pragma once

/**
 * comparer_t is a function pointer to comparator functions. Comparator
 * functions take two const void * and return an int. Suppose we had
 * comparer_t *comparer. Calling comparer(a,b) will return
 * - a negative value if a comes before b
 * - a positive value if b comes before a
 * - 0 otherwise.
 */
typedef int (*comparer_t)(const void *, const void *);

/**
 * An entry (node) in the priority queue.
 */
typedef struct _entry {
    void *value;         // The value stored in the entry.
    struct _entry *next; // A pointer to the next entry.
} entry;

/**
 * The priority queue data structure.
 */
typedef struct _priqueue_t {
    comparer_t comparer; // The comparator function.
    int size;            // The number of entries in the queue.
    entry *head;         // The entry with the highest priority.
} priqueue_t;

/**
 * Initializes the priqueue_t data structure with the provided comparer function
 *
 * Notes:
 * - You will need to call this function first for every instance of priqueue_t.
 * - Calling this function multiple times on an instance of priqueue_t is
 *   undefined behavior.
 *
 * @param q a pointer to an instance of the priqueue_t data structure.
 * @param comparer a function pointer to a comparator function.
 */
void priqueue_init(priqueue_t *q, comparer_t comparer);

/**
 * Inserts the element ptr into the priority queue.
 *
 * @param q a pointer to an instance of the priqueue_t data structure.
 * @param ptr a pointer to the data to be inserted into the priority queue.
 * @return The zero-based index where ptr is stored in the priority queue, where
 *         0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr);

/**
 * Retrieves, but does not remove, the head of this queue.
 *
 * @param q a pointer to an instance of the priqueue_t data structure.
 * @return pointer to element at the head of the queue, NULL if the queue is
 *         empty.
 */
void *priqueue_peek(priqueue_t *q);

/**
 * Retrieves and removes the head of this queue.
 *
 * @param q a pointer to an instance of the priqueue_t data structure.
 * @return the head of the queue, NULL if this queue is empty.
 */
void *priqueue_poll(priqueue_t *q);

/**
 * Returns the number of elements in the queue.
 *
 * @param q a pointer to an instance of the priqueue_t data structure.
 * @return the number of elements in the queue.
 */
int priqueue_size(priqueue_t *q);

/**
 * Destroys and frees all the memory associated with q.
 *
 * @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q);
