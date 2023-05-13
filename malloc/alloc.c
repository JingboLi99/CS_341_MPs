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
// /**
//  * Allocate memory block
//  *
//  * Allocates a block of size bytes of memory, returning a pointer to the
//  * beginning of the block.  The content of the newly allocated block of
//  * memory is not initialized, remaining with indeterminate values.
//  *
//  * @param size
//  *    Size of the memory block, in bytes.
//  *
//  * @return
//  *    On success, a pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a null pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
//  */
// void *malloc(size_t size) {
    
// }
// /**
//  * Allocate space for array in memory
//  *
//  * Allocates a block of memory for an array of num elements, each of them size
//  * bytes long, and initializes all its bits to zero. The effective result is
//  * the allocation of an zero-initialized memory block of (num * size) bytes.
//  *
//  * @param num
//  *    Number of elements to be allocated.
//  * @param size
//  *    Size of elements.
//  *
//  * @return
//  *    A pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory, a
//  *    NULL pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
//  */
// void *calloc(size_t num, size_t size) {

// }
// /**
//  * Deallocate space in memory
//  *
//  * A block of memory previously allocated using a call to malloc(),
//  * calloc() or realloc() is deallocated, making it available again for
//  * further allocations.
//  *
//  * Notice that this function leaves the value of ptr unchanged, hence
//  * it still points to the same (now invalid) location, and not to the
//  * null pointer.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(),
//  *    calloc() or realloc() to be deallocated.  If a null pointer is
//  *    passed as argument, no action occurs.
//  */
// void free(void *ptr) {
   
// }

// /**
//  * Reallocate memory block
//  *
//  * The size of the memory block pointed to by the ptr parameter is changed
//  * to the size bytes, expanding or reducing the amount of memory available
//  * in the block.
//  *
//  * The function may move the memory block to a new location, in which case
//  * the new location is returned. The content of the memory block is preserved
//  * up to the lesser of the new and old sizes, even if the block is moved. If
//  * the new size is larger, the value of the newly allocated portion is
//  * indeterminate.
//  *
//  * In case that ptr is NULL, the function behaves exactly as malloc, assigning
//  * a new block of size bytes and returning a pointer to the beginning of it.
//  *
//  * In case that the size is 0, the memory previously allocated in ptr is
//  * deallocated as if a call to free was made, and a NULL pointer is returned.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(), calloc()
//  *    or realloc() to be reallocated.
//  *
//  *    If this is NULL, a new block is allocated and a pointer to it is
//  *    returned by the function.
//  *
//  * @param size
//  *    New size for the memory block, in bytes.
//  *
//  *    If it is 0 and ptr points to an existing block of memory, the memory
//  *    block pointed by ptr is deallocated and a NULL pointer is returned.
//  *
//  * @return
//  *    A pointer to the reallocated memory block, which may be either the
//  *    same as the ptr argument or a new location.
//  *
//  *    The type of this pointer is void*, which can be cast to the desired
//  *    type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a NULL pointer is returned, and the memory block pointed to by
//  *    argument ptr is left unchanged.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
//  */
// void *realloc(void *ptr, size_t size) {
   
// }





// /**
//  * malloc
//  * CS 341 - Spring 2023
//  */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdbool.h>
// //**Block and Meta data structs**
// typedef struct blkhead{
//     size_t act_size;
//     bool isfree;
//     struct blkhead * nextf; // nextf goes from bottom to top
//     struct blkhead * prevf; // prevf goes from top to bottom
// } blkhead;

// typedef struct bt{
//     size_t act_size;
// } bt;
// //**Globals**
// static blkhead* fhead = NULL; // First free block (most bottom)
// static blkhead* ftail = NULL; // Last free block (most top)
// static void * firstblock; //Start position of the first block in the heap
// static void * lastblock; //End position of the last block in the heap.
// static const int BLKHEADSIZE = sizeof(blkhead); // size of block head metadata = 
// static const int BTSIZE = sizeof(bt); //size of boundary tag
// // static const int GROWTH_FACTOR = 2; //todo: used for reducing number of sbrk calls by calling more than required
// //**Helper functions**
// size_t get_new_capacity(size_t req_size) {
//     size_t init_cap = BLKHEADSIZE + BTSIZE;
//     size_t raw_cap = init_cap + req_size;
//     int n_units = (raw_cap + 15)/16; //number of size 16 units needed for current malloc request
//     return n_units * 16;
// }
// /**
//  * Given a free block, split it into two blocks: 1. allocated block as requested 2. remaining free space as a new block
//  * There are 2 situations: A. act_size if exact the same as required size. B. act_size > required_size + metadata tags sizes (both head and bt)
//  * @param blk Block to split. This block can fit the request size, and is chosen based on address order
//  * @param size Size of request 
//  * @return 
//  *      Pointer to user requested data
// **/
// void* splitFreeBlock(blkhead * blk, size_t size, int caller){
//     if (blk->isfree != true){ //Check that this block is labelled as free
//         fprintf(stderr, "*FREE BLOCK ERROR: Block is in free list but is marked not free\n %d", caller);
//         return NULL;
//     }
//     size_t ogblk_size = blk->act_size;
//     //1. Check if this is an exact fit (no splitting required)
//     if (ogblk_size > size){ //Splitting required: 4 steps (change size for all 4 metadata tags)
//         //place new boundary tag for the used block
//         bt * new_bt =  (bt *) (((char *) (blk + 1)) + size); //end of the user data segment of the allocated block. (before new bt)
//         new_bt->act_size = size; //NOTE: the size here is after alignement considerations
//         //place new metadata for newly created free block
//         blkhead * free_blkhead = (blkhead *) (new_bt + 1);
//         size_t free_size = ogblk_size - size - BLKHEADSIZE - BTSIZE;
//         free_blkhead->act_size = free_size;
//         free_blkhead->isfree = true;
//         free_blkhead->nextf = blk->nextf;
//         free_blkhead->prevf = blk->prevf;
//         //point prev amd next block (and fhead, ftail if applicable) to the new free block instead of old block
//         if (blk->prevf){
//             blk->prevf->nextf = free_blkhead;
//         }
//         else{ //is fhead
//             fhead = free_blkhead;
//         }
//         if (blk->nextf){
//             blk->nextf->prevf = free_blkhead;
//         }else{ // is ftail
//             ftail = free_blkhead;
//         }
//         //change bt size value for the freed block (which was inherited from the original block)
//         bt * free_bt = (bt *) (((char *) (free_blkhead + 1)) + free_size);
//         free_bt->act_size = free_size;
//         //change original metadata info:
//         blk->act_size = size;
//         blk->isfree = false;
//         blk->nextf = NULL;
//         blk->prevf = NULL;
//     }
//     else{ //reach here means no splitting required
//         //set isfree to false, and join prev free to next free
//         if (blk->prevf){
//             blk->prevf->nextf = blk->nextf;
//         }
//         else{ //block is fhead
//             fhead = blk->nextf;
//         }
//         if (blk->nextf){
//             blk->nextf->prevf = blk->prevf;
//         }
//         else{
//             ftail = blk->prevf;
//         }
//         blk->isfree = false;
//         blk->nextf = NULL; blk->prevf = NULL; // NOTE: USED BLOCKS ALWAYS HAVE next and prev SET AS NULL!
//     }
//     return (void *) (blk + 1);  //Return the start of the data for user.
// }
// //checks if coalescing will give enough space for realloc:
// bool checkIsEnough(blkhead *prev, blkhead *cur, blkhead *next, size_t req_size){
//     size_t new_size = cur->act_size;
//     if (prev){
//         new_size += (prev->act_size + BLKHEADSIZE + BTSIZE);
//     }
//     if (next){
//         new_size += (next->act_size + BLKHEADSIZE + BTSIZE);
//     }
//     return new_size == req_size || new_size > (req_size + BLKHEADSIZE + BTSIZE);
// }
// /**
//  * Gets the previous and next blocks IF THEY ARE ALSO FREE BLOCKS. 
//  *  @param cur Current block
//  *  @return 
//  *      - Returns pointer to the blkhead of the next or previous block.
//  *      - If either there is no previous block or no next block
//  *        or they are not free, both functions return NULL.
// */
// blkhead* getPrevBlk(blkhead * cur){
//     if ((void *) cur == firstblock) return NULL;//This is the first heap block! There is no prev block
//     bt * prev_bt = (bt *) (((char *) cur) - BTSIZE);
//     size_t prev_blksize = prev_bt->act_size;
//     blkhead * prev_blk = (blkhead *) (((char *) prev_bt) - BLKHEADSIZE - prev_blksize);
//     //check that next_blk is not larger than system break and that it is a free block
//     if (prev_blk->isfree == true){
//         return prev_blk;
//     }
//     return NULL;
// }
// blkhead* getNextBlk(blkhead * cur){
//     size_t cur_size = cur->act_size;
//     // void * cur_sysbrk = sbrk(0); // position of the system break
//     if ((void *) (((char *) cur) + cur_size + BLKHEADSIZE + BTSIZE) > lastblock){
//         fprintf(stderr, "\n***HEAP EXCEEDED: WRITING BEYOND END OF HEAP!!\n\n");
//     }
//     if ((void *) (((char *) cur) + cur_size + BLKHEADSIZE + BTSIZE) == lastblock) return NULL; //This is the last block. There is no next block
//     size_t cur_blksize = cur->act_size;
//     blkhead * next_blk = (blkhead *) (((char *) cur) + BLKHEADSIZE + cur_blksize + BTSIZE);
//     //check that next_blk is not larger than system break and that it is a free block
//     if (next_blk->isfree == true){
//         return next_blk;
//     }
//     return NULL;
// }
// /**
//  * Given an existing allocated block, we only keep the <newsize> portion of the block, and split the block
//  * to create a new freed block.
// */
// void splitUsedBlock(blkhead * blk, size_t newsize){
//     size_t ogblk_size = blk->act_size;
//     if (ogblk_size == newsize) return; //Size is exact, no splitting needed.
//     //place new boundary tag for the resized used block
//     bt * new_blk_bt = (bt *) (((char *) (blk + 1)) + newsize);
//     new_blk_bt->act_size = newsize;
//     //place new blkhead for new free block
//     size_t free_size = ogblk_size - newsize - BLKHEADSIZE - BTSIZE;
//     blkhead * free_blkhead = (blkhead *) (new_blk_bt + 1);
//     free_blkhead->act_size = free_size;
//     free_blkhead->isfree = false; //free() will change it for us.
//     free_blkhead->nextf = NULL; free_blkhead->prevf = NULL;
//     //change new free's bt:
//     bt * free_bt = (bt *) (((char *) (free_blkhead + 1)) + free_size);
//     free_bt->act_size = free_size;
//     //free the newly partitioned free block:
//     void * free_userdata = (void *) (free_blkhead + 1);
//     free(free_userdata);
//     //change original blkhead info:
//     blk->act_size = newsize;
// }

// blkhead * coalesce(blkhead * prev_blk, blkhead * cur_blk, blkhead * next_blk){
//     //coalesce current block with prev/ next if any
//     // If there is either prev or curr free blocks, we essentially follow their position within the freed list
//     if (prev_blk){ //previous block is a free block
//         size_t add_size = BLKHEADSIZE + BTSIZE + cur_blk->act_size;
//         prev_blk->act_size += add_size;
//         size_t new_prev_sz = prev_blk->act_size;
//         // memset(prev_blk+1, 0, new_prev_sz); //todo: Is this necessary? Do we have to wipe out the data at all?
//         bt * new_prev_bt = (bt *) (((char *)(prev_blk + 1)) + new_prev_sz);
//         new_prev_bt->act_size = new_prev_sz;
//         cur_blk = prev_blk; // combine prev and current to both be cur (naming covention so we can join with next with the same code)
//     }
//     if (next_blk){ //next block is a freed block
//         //Inherit the nextf pointer of the next block. Change nextf's prevf to cur_blk (instad of next_blk)
//         cur_blk->nextf = next_blk->nextf;
//         if (cur_blk->nextf){ //this is not ftail
//             cur_blk->nextf->prevf = cur_blk;
//         }
//         else{ // this is ftail
//             ftail = cur_blk;
//         }
//         if (!prev_blk){ //if previous didnt exist, we need to also inherit the prevf pointer of the next block
//             cur_blk->prevf = next_blk->prevf;
//             if (cur_blk->prevf){
//                 cur_blk->prevf->nextf = cur_blk;
//             }
//             else{
//                 fhead = cur_blk;
//             }
//         }
//         cur_blk->isfree = true; //Set the current block to be free
//         size_t add_size = BLKHEADSIZE + BTSIZE + next_blk->act_size; //size to act to current block
//         cur_blk->act_size += add_size;
//         size_t new_cur_sz = cur_blk->act_size;
//         // memset(cur_blk+1, 0, new_cur_sz); //set all values in new freed block to 0
//         bt * new_cur_bt = (bt *) (((char *) (cur_blk + 1)) + new_cur_sz);
//         new_cur_bt->act_size = new_cur_sz;
//     }
//     // If curr freed is a standalone free block, insert the new free blocks into the free list in address order (todo: do log(n) skip list? How?)
//     if (!prev_blk && !next_blk){ //standalone freed block: Insert in address order.
//         cur_blk->isfree = true; // set as free. size doesnt change. nextf and prevf should both still be NULL
//         //iterate through free list, to find the first block that is "above" cur_blk in address
//         blkhead * new_next = fhead;
//         while (new_next && new_next < cur_blk){
//             new_next = new_next->nextf;
//         }
//         //NOTE: Reaching here means we have found our the "next" freed block to our current block
//         if (!new_next){// cur block will be new ftail
//             blkhead * new_prev = ftail;
//             //check if ftail (and fhead) even exists
//             if (!new_prev){ //no ftail: free list has no elements
//                 ftail = cur_blk;
//                 fhead = cur_blk;
//             }
//             else{ //ftail exists
//                 ftail = cur_blk;
//                 cur_blk->prevf = new_prev;
//                 new_prev->nextf = cur_blk;
//             }
            
//             // fprintf(stderr, "**REACHED HERE\n");
//         }
//         else{ // cur blk is not ftail
//             blkhead * new_prev = new_next->prevf;
//             cur_blk->nextf = new_next;
//             new_next->prevf = cur_blk;
//             if (new_prev){//current block is not fhead
//                 cur_blk->prevf = new_prev;
//                 new_prev->nextf = cur_blk;
//             }
//             else{ //current block is new fhead
//                 fhead = cur_blk;
//             }
//         }
//     }
//     if (cur_blk->isfree != true){
//         fprintf(stderr, "**FREE ERROR: Didnt manage to set free correctly\n");
//     }
//     return cur_blk;
// }
// /**
//  * Allocate memory block
//  *
//  * Allocates a block of size bytes of memory, returning a pointer to the
//  * beginning of the block.  The content of the newly allocated block of
//  * memory is not initialized, remaining with indeterminate values.
//  *
//  * @param size
//  *    Size of the memory block, in bytes.
//  *
//  * @return
//  *    On success, a pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a null pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
//  */
// void *malloc(size_t size) {
//     if (size == 0){
//         return NULL;
//     }
    
//     size_t size_to_add = get_new_capacity(size); //Total amount of data to reserve with sbrk, with alignment considerations
//     size_t act_size = size_to_add - BLKHEADSIZE - BTSIZE; //size of user data
//     //Check through list of free blocks: POLICY = address ordered
//     blkhead * currfblk = fhead; // start from bottom most position, check for large enough free blocks upwards
//     while (currfblk != NULL){ // check all free blocks between head and tail (fnext of tail will be NULL)
//         //check if curr free block is large enough:
//         if (currfblk->act_size > size_to_add || currfblk->act_size == act_size){ //current block large enough. Use it
//             void * currblk_act_data_st = splitFreeBlock(currfblk, act_size, 0);
//             fprintf(stderr, "^");
//             return currblk_act_data_st;
//         }
//         else{// current block too small to hold request (account for new metadata to be added to split block) Use next free block.
//             currfblk = currfblk->nextf;
//         }
//     }
//     //NOTE: Reaching here means no free blocks are big enough: **needs to sbrk a new block
//     // void * curr_heap_top = sbrk(0);
//     void* new_blk = sbrk(size_to_add);
//     //Check if sbrk was successful: If sbrk fails return NULL
//     if (new_blk == (void*) -1) return NULL;
//     //initialize heapstart on first malloc:
//     if (!firstblock) firstblock = new_blk;
//     //Initialize or update heapend each time new memory is added with sbrk:
//     lastblock = (void *)(((char *) new_blk) + size_to_add); 
//     //place head metadata into new block
//     blkhead * new_blkhead = (blkhead *) new_blk;
//     new_blkhead->act_size = act_size;
//     new_blkhead->isfree = false;
//     new_blkhead->nextf = NULL; new_blkhead->prevf = NULL;
//     //place boundary tag (bt) in block
//     bt * new_bt = (bt *) (((char *) (new_blkhead+1)) + act_size);
//     new_bt->act_size = act_size;     
//     fprintf(stderr, "$$");
//     return (void *) (new_blkhead+1);
// }
// /**
//  * Allocate space for array in memory
//  *
//  * Allocates a block of memory for an array of num elements, each of them size
//  * bytes long, and initializes all its bits to zero. The effective result is
//  * the allocation of an zero-initialized memory block of (num * size) bytes.
//  *
//  * @param num
//  *    Number of elements to be allocated.
//  * @param size
//  *    Size of elements.
//  *
//  * @return
//  *    A pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory, a
//  *    NULL pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
//  */
// void *calloc(size_t num, size_t size) {
//     // implement calloc!
//     size_t total_size = num * size;
//     void * user_data = malloc(total_size);
//     memset(user_data, 0, total_size);
//     return user_data;
// }
// /**
//  * Deallocate space in memory
//  *
//  * A block of memory previously allocated using a call to malloc(),
//  * calloc() or realloc() is deallocated, making it available again for
//  * further allocations.
//  *
//  * Notice that this function leaves the value of ptr unchanged, hence
//  * it still points to the same (now invalid) location, and not to the
//  * null pointer.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(),
//  *    calloc() or realloc() to be deallocated.  If a null pointer is
//  *    passed as argument, no action occurs.
//  */
// void free(void *ptr) {
//     if (!ptr) return;
//     blkhead * cur_blk =(blkhead *) (((char *) ptr) - BLKHEADSIZE);
//     if (cur_blk->isfree == true){
//         printf("FREE ERROR: Trying to free a block that is already free");
//     }
//     blkhead * prev_blk = getPrevBlk(cur_blk); //is NULL if not a freed block
//     blkhead * next_blk = getNextBlk(cur_blk); //is NULL if not a freed block
//     coalesce(prev_blk, cur_blk, next_blk);
// }

// /**
//  * Reallocate memory block
//  *
//  * The size of the memory block pointed to by the ptr parameter is changed
//  * to the size bytes, expanding or reducing the amount of memory available
//  * in the block.
//  *
//  * The function may move the memory block to a new location, in which case
//  * the new location is returned. The content of the memory block is preserved
//  * up to the lesser of the new and old sizes, even if the block is moved. If
//  * the new size is larger, the value of the newly allocated portion is
//  * indeterminate.
//  *
//  * In case that ptr is NULL, the function behaves exactly as malloc, assigning
//  * a new block of size bytes and returning a pointer to the beginning of it.
//  *
//  * In case that the size is 0, the memory previously allocated in ptr is
//  * deallocated as if a call to free was made, and a NULL pointer is returned.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(), calloc()
//  *    or realloc() to be reallocated.
//  *
//  *    If this is NULL, a new block is allocated and a pointer to it is
//  *    returned by the function.
//  *
//  * @param size
//  *    New size for the memory block, in bytes.
//  *
//  *    If it is 0 and ptr points to an existing block of memory, the memory
//  *    block pointed by ptr is deallocated and a NULL pointer is returned.
//  *
//  * @return
//  *    A pointer to the reallocated memory block, which may be either the
//  *    same as the ptr argument or a new location.
//  *
//  *    The type of this pointer is void*, which can be cast to the desired
//  *    type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a NULL pointer is returned, and the memory block pointed to by
//  *    argument ptr is left unchanged.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
//  */
// void *realloc(void *ptr, size_t size) {
//     // Case 1: size if null - act as free
//     if (size == 0){
//         free(ptr);
//         return NULL;
//     }
//     // Case 2: Pointer is NULL - act as Malloc:
//     if (!ptr){
//         void * userdata_start = malloc(size);
//         return userdata_start;
//     }
//     blkhead *cur_blk = (blkhead *) (((char *) ptr) - BLKHEADSIZE);
//     size_t og_size = cur_blk->act_size;
//     // Case 3: No change in size
//     if (size == og_size){
//         return ptr;
//     }
//     // Case 4: new_size + metadata < og_size -> split block
//     if (og_size > size + BLKHEADSIZE + BTSIZE){
//         splitUsedBlock(cur_blk, size);
//         return ptr;
//     }
//     // Try to coalesce current block with adjacent blocks, and see if the new free block can accomodate the request
//     blkhead * prev_blk = getPrevBlk(cur_blk);
//     blkhead * next_blk = getNextBlk(cur_blk);
//     bool isEnough =  checkIsEnough(prev_blk, cur_blk, next_blk, size); //check if there is enough space after coalescing
//     //NOTE: This takes into account the need for splitting (includes metadata size): ie either exact size, or can also accomodate additional metadata
//     // Case 5: coalesced space is large enough
//     if (isEnough){
//         /**1. copy current blocks actual data over to a new tep block
//          * 2. coalesce current block (basically freeing)
//          * 3. write back the data into the new freed block
//          * 4. free the temp block 
//          * 5. Split the block if needed (recall the blocksize is either exact or enough for splitting)
//         */
//         void * temp_data = malloc(og_size);
//         memcpy(temp_data, ptr, og_size);
//         blkhead * new_freed_blk = coalesce(prev_blk, cur_blk, next_blk);
//         void * new_freed_data = splitFreeBlock(new_freed_blk, size, 1);
//         if (size < og_size){
//             memcpy(new_freed_data, temp_data, size);
//         }
//         else{
//             memcpy(new_freed_data, temp_data, og_size);
//         }
//         free(temp_data);
//         return new_freed_data;
//     }
//     // Case 6: 
//     else{
//         void * new_data_st = malloc(size);
//         if (!new_data_st) return NULL; // if malloc fails to allocate this much memory
//         memcpy(new_data_st, ptr, og_size);
//         free(ptr);
//         return new_data_st;
//     }
    
// }