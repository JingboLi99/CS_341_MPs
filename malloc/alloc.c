/**
 * malloc
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// ** Metadata struct
typedef struct metadata{
    size_t usize; //user size
    bool isFree;
    struct metadata * prev; //previous block
    struct metadata * next; //next block
} metadata;
// ** GLOBALS:
static metadata * head = NULL;
static size_t total_memory_srbk = 0;
static size_t total_memory_requested = 0;
static const size_t METASIZE = sizeof(metadata);
// ** HELPERS
void split_block(metadata * blk, size_t req_size); //Splits a free block
metadata * coalesce(metadata * tofree, int side);


// ** Allocs:
void *malloc(size_t size) {
    if (size <= 0) return NULL;
    metadata * cur = head;
    //Check for a free block to allocate:
    if (total_memory_srbk - total_memory_requested >= size) {
        while (cur){
            if (cur->isFree && cur->usize >= size){    
                if (cur->usize - size >= 1024){
                    split_block(cur, size);
                    total_memory_requested += sizeof(metadata); 
                } 
                // if (cur->usize > size ) split_block(cur, size);
                cur->isFree = false; //since we are using cur, it is no longer free.
                // fprintf(stderr, "/");
                total_memory_requested += cur->usize;
                return (void *) (cur + 1);
            }
            cur = cur->next;
        }
    }
    // Reach here means no free blocks are suitable
    size_t size_to_add = METASIZE + size;
    void* new_mem = sbrk(size_to_add); //sbrk new chunk of heap memory
    if (new_mem == (void*) -1) return NULL; //Check if sbrk was successful: If sbrk fails return NULL
    metadata * new_blk = (metadata *) new_mem;
    // Update head:
    new_blk->isFree = false;
    new_blk->next = head;
    new_blk->prev = NULL;
    if (head) head->prev = new_blk;
    new_blk->usize = size;
    head = new_blk;
    // fprintf(stderr, "*");
    total_memory_requested += sizeof(metadata) + size;
    total_memory_srbk += sizeof(metadata) + size;
    return (void *)(new_blk + 1);
}

void *calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void * data = malloc(total_size);
    memset(data, 0, total_size);
    return data;
}

void free(void *ptr) {
    if (!ptr) return;
   metadata * tofree = (metadata *) (((char *) ptr) -METASIZE);
   if (tofree->isFree) return; // If this is already a free block
   total_memory_requested -= tofree->usize;
   if (tofree->next && tofree->next->isFree) tofree = coalesce(tofree, 0);
   if (tofree->prev && tofree->prev->isFree) coalesce(tofree, 1);
   tofree->isFree = true;
}

void *realloc(void *ptr, size_t size) {
    
    if (size == 0) free(ptr);
    // fprintf(stderr, "***OUT\n");
    if (!ptr) return malloc(size);
    metadata * blk = (metadata *) (((char *) ptr) -METASIZE);
    
    if (blk->usize >= size){
        if (blk->usize - size >= 1024) {
            split_block(blk, size);
            total_memory_requested -= blk->prev->usize;
        }
        // if (blk->usize > size ) split_block(blk, size);
        // fprintf(stderr, "***HERE\n");
        return ptr;
    }
    // REACH HERE means current block is not big enough:
    if (blk->prev && blk->prev->isFree && (blk->usize + blk->prev->usize) >= size){
        total_memory_requested += blk->prev->usize;
        coalesce(blk, 1); //coalesce previous
        return ptr;
    }
    else if (blk->next && blk->next->isFree && (blk->usize + blk->next->usize) >= size){
        total_memory_requested += blk->next->usize;
        metadata * newblk = coalesce(blk, 0); //coalesce w next blk
        memcpy((newblk+1), ptr, blk->usize); //copy over the data to the next blk
        newblk->isFree = false;
        return (void *) (newblk + 1);
    }
    else if (blk->prev && blk->prev->isFree &&// if both prev and next are free
             blk->next && blk->next->isFree &&
             (blk->usize + blk->next->usize + blk->prev->usize) >= size){ // and their combined size is enough for newsize
                total_memory_requested += blk->prev->usize;
                total_memory_requested += blk->next->usize;
                coalesce(blk, 1); //coalesce previous
                metadata * newblk = coalesce(blk, 0); //coalesce w next blk
                memcpy((newblk+1), ptr, blk->usize); //copy over the data to the next blk
                newblk->isFree = false;
                return (void *) (newblk + 1);
             }
    else{ // unable to coalesce-> find new memory
        void * newdata = malloc(size);
        memcpy(newdata, ptr, blk->usize);
        free(ptr);
        return newdata;
    }
}

// ** HELPER FUNCTIONS:
void split_block(metadata * blk, size_t req_usize){ //NOTE: req_usize is the user size that we require
    metadata * new_blk = (metadata *) (((char *) (blk + 1)) + req_usize);
    size_t new_size = blk->usize - req_usize - METASIZE; //new blk user size
    blk->usize = req_usize; //update the size of the original block
    new_blk->isFree = true; //new blk is free 
    new_blk->usize = new_size; 
    new_blk->next = blk; new_blk->prev = blk->prev;
    blk->prev = new_blk;
    if (new_blk->prev) new_blk->prev->next = new_blk; //old blk's prev blk now points to new_blk
    if (head == blk) head = new_blk; //if head used to point to old block, it now points to new blk
}
//coalesce is only called when we know we can coalesce
metadata * coalesce(metadata * tofree, int side){ //NOTE: 0: coalesce next 1: coalesce previous
    if (side == 0){ //coalesce with the next block
        metadata * nextblk = tofree->next;
        nextblk->prev = tofree->prev; //next blk inherits prev pointer of cur blk
        if (nextblk->prev) nextblk->prev->next = nextblk; //tofree's prev now points to next
        if (head == tofree) head = nextblk;
        //Increase size of next blk:
        nextblk->usize += (METASIZE + tofree->usize);
        // fprintf(stderr, "-");
        total_memory_requested -= sizeof(metadata);
        return nextblk; //return the combined freed block (to help coalesce with prev blk if any)
    }else{ //coalesce with previous block
        metadata * prevblk = tofree->prev;
        tofree->prev = prevblk->prev;
        if (tofree->prev) tofree->prev->next = tofree;
        if (head == prevblk) head = tofree;
        tofree->usize += (METASIZE + prevblk->usize);
        // fprintf(stderr, "+");
        total_memory_requested -= sizeof(metadata);
        return tofree;
    }
}