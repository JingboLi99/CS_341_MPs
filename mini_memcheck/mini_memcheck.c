/**
 * mini_memcheck
 * CS 341 - Spring 2023
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//defining global variables
meta_data *head;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;

//**NOTE: new data is inserted at the head of the linked list!!
void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // your code here
    if (request_size == 0){
        return NULL;
    }
    // Allocate memory on heap and initialize metadata ptr and actual data ptr
    struct _meta_data * metadata = malloc(sizeof(struct _meta_data) + request_size); //this pointer points to the start of the metadata
    // printf("**Size of allocated node: %zu\n", sizeof(*metadata));
    //If malloc failed, return NULL: When malloc fails, it return NULL pointer
    if (!metadata){
        return NULL;
    }
    // void * act_data = metadata + 1; //this pointer points to the actual memory
    //increment total memory requested:
    total_memory_requested += request_size;
    // put items into metadata:
    metadata->request_size = request_size;
    metadata->filename = filename;
    metadata->instruction = instruction;
    //change head pointer and curr metadata next pointer
    metadata->next = head;
    head = metadata;
    //return pointer to actual data
    return metadata + 1;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // your code here
    //if trying to allocate 0 bytes of memory
    if (num_elements == 0){
        return NULL;
    }
    //calculate actual size in bytes needed for the request
    size_t request_size = num_elements * element_size;
    // Allocate memory on heap and initialize metadata ptr and actual data ptr
    struct _meta_data * metadata = malloc(sizeof(struct _meta_data) + request_size); //this pointer points to the start of the metadata
    // printf("**Size of actual data: %zu\n", sizeof(*metadata));
    //If malloc failed, return NULL: When malloc fails, it return NULL pointer
    if (!metadata){
        return NULL;
    }
    void * act_data = metadata + 1; //this pointer points to the actual memory
    // act_data = malloc(request_size);
    //increment total memory requested:
    total_memory_requested += request_size;
    // put items into metadata:
    metadata->request_size = request_size;
    metadata->filename = filename;
    metadata->instruction = instruction;
    //change head pointer and curr metadata next pointer
    metadata->next = head;
    head = metadata;
    //return pointer to actual data
    return act_data;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // your code here
    // If no current payload exist, realloc basically acts as malloc does
    if (!payload){
        return mini_malloc(request_size, filename, instruction);
    }
    //If request size is 0, we basically want non of the existing memory in payload: free
    if (request_size == 0){
        mini_free(payload);
        return NULL;
    }

    struct _meta_data * iter = head;
    //check head:
    void *head_act_data = iter + 1;
    if (head_act_data == payload){
        // a. document size change in global variables and local variables
        int size_diff = request_size - iter->request_size;
        if (size_diff > 0){
            total_memory_requested += size_diff;
        }
        else{
            total_memory_freed += size_diff;
        }
        iter->request_size = request_size; //Modify request size
        //b. do realloc
        size_t new_size = sizeof(struct _meta_data) + request_size;
        iter = realloc(iter, new_size);
        if (!iter){ //if failed to realloc properly
            return NULL;
        }
        //set head to new pointer for this node:
        head = iter;
        return iter + 1; //return the user memory
    }
    struct _meta_data * iter_next = iter->next;
    //We iterate through our entire linked list to find the position of the payload
    //We compare the act_memory address of the current node with payload ptr.
    //If same, we modify it w realloc
    //If no node in LL has same mem adr, it is an invalid address
    while (iter_next){
        void *cur_act_data = iter_next + 1; //pointer to the actual data in this node
        if (cur_act_data != payload){ //compare the 2 memory addresses: true = differnt
            iter = iter_next;
            iter_next = iter_next->next;
            continue;
        }
        //Reaching here means we found the payload node:
        // a. document size change in global variables and local variables
        int size_diff = request_size - iter_next->request_size;
        if (size_diff > 0){
            total_memory_requested += size_diff;
        }
        else{
            total_memory_freed += size_diff;
        }
        iter_next->request_size = request_size; //Modify request size
        //b. do realloc
        //1. realloc from the start of the entire node (metadata pointer)
        //2. repoint the next for the previous node, to the new pointer given by realloc
        size_t new_size = sizeof(struct _meta_data) + request_size;
        iter_next = realloc(iter_next, new_size);
        if (!iter_next){ //if failed to realloc properly
            return NULL;
        }
        iter->next = iter_next;
        return iter_next+1;
    }
    //Reaching here means that payload address was not in our LL, aka invalid address
    invalid_addresses ++;
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    // If a NULL pointer is passed, no action occurs, and it does not count as a bad free.
    if (!payload){
        return;
    }
    struct _meta_data * iter = head;
    //check head:
    void *head_act_data = iter + 1;
    if (head_act_data == payload){
        head = iter->next;
        total_memory_freed += iter->request_size;
        // printf("**Size at this ptr: %zu: metadata= %zu\n", sizeof(*iter), sizeof(struct _meta_data));
        free(iter);
        return;
    }
    meta_data * curr = head;
    meta_data * prev = NULL;
    meta_data * next = curr ->next;
    while (curr){
        next = curr->next;
        void *cur_act_data = curr + 1; //pointer to the actual data in this node
        if (cur_act_data == payload){ //compare the 2 memory addresses
            //Reaching here means we found the payload node:
            //Remove both meta and actual data from this node.
            prev->next = next; //connect parent to child
            total_memory_freed += curr->request_size;
            free(curr);
            return;
        }
        prev = curr;
        curr = next;       
    }
    //Reaching here means that payload address was not in our LL, aka invalid address
    invalid_addresses ++;
}

// meta_data *find_prev_instance(meta_data * head, void * to_find) {
//     meta_data * curr_head = head;
//     meta_data * prev = NULL;
//     meta_data * next = curr_head->next;

//     while (curr_head) {
//         next = curr_head->next;
//         // Get memory location of curr_head
//         void * memory_loc = ((void *) curr_head) + sizeof(meta_data);
//         // If found to_find, return the prev pointer 
//         if (to_find == memory_loc) return prev;
//         // Iterate to the next pointer in linked list
//         prev = curr_head;
//         curr_head = next;
//     }
//     // to_find wasn't found
//     return NULL;
// }

// void mini_free(void *payload) {
//     // If invalid, nothing happens
//     if (!payload) return;

//     struct _meta_data * iter = head;
//     //check head:
//     void *head_act_data = iter + 1;
//     if (head_act_data == payload){
//         head = iter->next;
//         total_memory_freed += iter->request_size;
//         // printf("**Size at this ptr: %zu: metadata= %zu\n", sizeof(*iter), sizeof(struct _meta_data));
//         free(iter);
//         return;
//     }

//     // Once you know it's not the head
//     meta_data * prev_to_found = find_prev_instance(head, payload);
//     if (prev_to_found) {
//         meta_data * temp = prev_to_found->next;
//         prev_to_found->next = temp->next;
//         total_memory_freed += temp->request_size;
//         free(temp);
//         return;
//     }
//     invalid_addresses++;
// }


