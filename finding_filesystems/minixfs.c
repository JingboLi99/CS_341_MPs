/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define BLK_SIZE (16 * KILOBYTE)
// ** HELPER FUNCTIONS
int min(int a, int b) {
    return (a < b) ? a : b;
}
/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // TODO: Do I have to check if the current user can change the inode mode? if so how?
    // printf("Permission: %d\n", new_permissions);
    inode * in = get_inode(fs, path); //get the inode of path
    if (in){ //if the path exists
        uint16_t in_md = in->mode; //get the current mode int
        uint16_t reserved = in_md >> RWX_BITS_NUMBER;
        in->mode = new_permissions | (reserved << RWX_BITS_NUMBER);
        clock_gettime(CLOCK_REALTIME, &in->ctim); //change ctim
        return 0;
    }
    else{
        errno = ENOENT;
        return -1;
    }
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode * in = get_inode(fs, path);
    if (in){
        if (owner != (uid_t) -1){ //check if new uid to change to is valid
            in->uid = owner;
        }
        if (group != (gid_t) -1){ //check if new gid to change to is valid
            in->gid = group;
        }
        clock_gettime(CLOCK_REALTIME, &in->ctim); //change ctim
        return 0;
    }
    else{
        errno = ENOENT;
        return -1;
    }
}
char * getParentBlock(file_system *fs, inode *parent){
    int nblocks = floor(parent->size / BLK_SIZE);
    long offset = parent->size % BLK_SIZE;
    data_block_number st_blkno;
    if (!offset){
        st_blkno = add_data_block_to_inode(fs, parent);
        if (st_blkno == -1) return NULL;
    }
    else{
        if (nblocks < NUM_DIRECT_BLOCKS) st_blkno = nblocks;
        else{
            nblocks -= NUM_DIRECT_BLOCKS;
            data_block_number * ind_blkno_p = (data_block_number *) &fs->data_root[parent->indirect]; //start of the indirect block
            st_blkno = *(ind_blkno_p + nblocks);
        }
        
    }

    data_block * st_blk = &fs->data_root[st_blkno];
    return (char *) st_blk + offset;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode * in = get_inode(fs, path);
    if (in) return NULL; //file at filepath already exists
    inode_number new_in_no;
    if ((new_in_no = first_unused_inode(fs)) == -1){ // No availble inodes left
        fprintf(stderr, "CREATE INODE ERROR: No more inodes available\n");
    }
    inode * new_in = &fs->inode_root[new_in_no]; // get the unused inode 
    const char * newfilename = NULL;
    inode * parent = parent_directory(fs, path, &newfilename); //get parent and newfilename
    // check that parent and new filename are valid    
    if (!valid_filename(newfilename) || !parent) return NULL;
    init_inode(parent, new_in); //initialize this current new inode
    // create new directory entry for the new file
    minixfs_dirent new_dirent;
    new_dirent.inode_num = new_in_no; 
    new_dirent.name = (char *) newfilename;
    // find which <block + offset> to store new dirent in parent
    char * storage_loc = getParentBlock(fs, parent);
    if (!storage_loc) return NULL;
    memset(storage_loc, 0, FILE_NAME_ENTRY);
    make_string_from_dirent(storage_loc, new_dirent);
    
    parent->size += FILE_NAME_ENTRY;
    return new_in;
}

int write_helper(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off, int actual_offset) {

    inode* in = get_inode(fs, path);

    // Check if there is enough space
    // I guess we should actually incorporate the "count" in here as well
    int blocks_needed = ceil((actual_offset + (int) count) / ((int) (16 * KILOBYTE)));

    int min_blk_ct = minixfs_min_blockcount(fs, path, blocks_needed);
    if (min_blk_ct == -1) {
        errno = ENOSPC;
        return -1;
    }
    // Use offset to determine which block we should write to
    // For example, if off == 32800, then we skip first 2 blocks and start writing at 32nd byte of third block
    int nblks_to_skip = actual_offset / BLK_SIZE;

    data_block_number block_num_to_start_write;
    if (nblks_to_skip < NUM_DIRECT_BLOCKS) { //block to write is a direct block
        block_num_to_start_write = in->direct[nblks_to_skip];
    } else { //block to write is a indirect block
        nblks_to_skip -= NUM_DIRECT_BLOCKS;
        data_block_number* start_of_indirect_block = (data_block_number *) &(fs->data_root)[in->indirect];
        block_num_to_start_write = start_of_indirect_block[nblks_to_skip];
    }
    data_block* st_blk = &fs->data_root[block_num_to_start_write];

    int remaining_offset = actual_offset % BLK_SIZE;
    void* part_of_block = ((void*) st_blk) + remaining_offset;

    int cur_blk_space = BLK_SIZE - remaining_offset;
    int toWrite = (int) count;

    int hasWritten = 0;
    // Use this to track how many blocks we have used (i.e., if we've used block number at idx 9, and 10, and 11 of direct[] array)
    int prev_block_number_idx = nblks_to_skip;
    while (toWrite > 0) {
        int cur_toWrite = min(cur_blk_space, toWrite);
        memcpy(part_of_block, buf, cur_toWrite);
        // Update for next iteration
        buf += cur_toWrite;
        cur_blk_space -= cur_toWrite;
        toWrite -= cur_toWrite;
        hasWritten += cur_toWrite;
        if (toWrite > 0) {
            // move pointer to next block; check if direct or indirect
            data_block_number nxt_blk = 0;
            int block_number_idx = prev_block_number_idx + 1;
            prev_block_number_idx++;
            if (block_number_idx < NUM_DIRECT_BLOCKS) {
                nxt_blk = in->direct[block_number_idx];
            } else {
                data_block_number* ind_blk_st = (data_block_number *) &(fs->data_root)[in->indirect];
                block_number_idx -= NUM_DIRECT_BLOCKS;
                nxt_blk = ind_blk_st[block_number_idx];
            }

            // Now that we have nxt_blk, we actually get the ptr to that block by adding from the root
            data_block* cont_blk = &fs->data_root[nxt_blk];
            part_of_block = ((void*) cont_blk);

            // Update stuff
            cur_blk_space = BLK_SIZE;
        }
    }
    // Update filesize
    int ptr_diff = in->size - actual_offset;
    int extra_bytes_written = hasWritten - ptr_diff;
    if (extra_bytes_written > 0) {
        in->size += extra_bytes_written;
    }
    // Update mtim and atim
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    in->mtim = ts;
    in->atim = ts;
    *off += hasWritten;

    return hasWritten;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    if (count > ((NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS) * 16 * KILOBYTE)) {
        errno = ENOSPC;
        return -1;
    }

    inode* in = get_inode(fs, path);
    if (in == NULL) { // if we need to create a new file
        inode* new_in = minixfs_create_inode_for_path(fs, path);
        if (new_in == NULL) { // failed to create new file
            return -1;
        }
        return write_helper(fs, path, buf, count, off, (int) *off);
    }
    else if (is_directory(in)) { // Write to an existing directory
        return write_helper(fs, path, buf, count, off, (int) in->size);
    }
    else if (is_file(in)) { //write to an existing file
        return write_helper(fs, path, buf, count, off, (int) *off);
    } 

    return -1;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        //NOTE: virtual path does not actually "exist" on the filesystem, so do not do get_inode check
        superblock * sup_blk_ptr = fs->meta; //get the superblock of filesystem
        uint64_t total_blocks = sup_blk_ptr->dblock_count; //total blocks in fs
        char * datamap = GET_DATA_MAP(sup_blk_ptr); 
        uint64_t used_blocks = 0;
        uint64_t unused_blocks = 0;
        // iterate through the datamap, incrementing used_blocks each time one is found
        for (uint64_t i = 0; i < total_blocks; i++){
            if (datamap[i] == 1) used_blocks ++;
            else unused_blocks ++;
        }
        if (total_blocks != (used_blocks + unused_blocks)){
            fprintf(stderr, "**SOMETHING IS FUCKING WRONG!! Number of blocks do not align\n");
        }
        char * infostr = malloc(64);
        sprintf(infostr, "Free blocks: %ld\nUsed blocks: %ld\n", unused_blocks, used_blocks);
        //copy string into buffer
        size_t infolen = strlen(infostr);
        int toRead = min(count, infolen - *off); //number of bytes to read
        if (toRead > 0){
            char * str_st = infostr + *off;
            memcpy(buf, str_st, toRead);
            *off += toRead;
        }
        if (infostr) free(infostr);
        //TODO: How do I edit the atim?
        // clock_gettime(CLOCK_REALTIME, &in->atim);
        return toRead;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    fflush(stdout);
    if (virtual_path){
        // printf("Getting virtual path!\n"); fflush(stdout);
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    }
    //check that filepath exsits an it is not a directory:
    inode * in = get_inode(fs, path);
    if (!in){ //if this file path does not exist
        errno = ENOENT;
        return -1;
    }    
    // We want to read "count" bytes of data, unless count > filesize - off
    size_t toRead = min(count, in->size - *off); //number of bytes to read
    size_t hasRead = 0;
    void * tbuf  = buf; //points to the tail of the buffer (aka start writing from here)
    long toskip = * off;
    // read from direct blocks
    for (int i = 0; i < NUM_DIRECT_BLOCKS; i++){
        if (toRead <= 0) break; //we do not need to read anymore data
        if (toskip >= BLK_SIZE){ //we want to skip this entire block
            toskip -= BLK_SIZE;
            continue;
        }else{ //we want to skip part (or none) of this block
            int cur_toRead = min(BLK_SIZE - toskip, toRead); //how much to rea from this block
            data_block_number cur_blk_no = in->direct[i];
            data_block* cur_blk = &fs->data_root[cur_blk_no];
            void * st_add = (void *) cur_blk + toskip; //location to start copying from for this block
            memcpy(tbuf, st_add, cur_toRead);
            tbuf += cur_toRead;
            toskip = 0;
            toRead -= cur_toRead;
            hasRead += cur_toRead;
        }
    }
    // read from indirect blocks
    data_block_number * ind_blkno_p = (data_block_number *) &fs->data_root[in->indirect]; //start of the indirect block
    while (*ind_blkno_p != -1){ //while we have not reached the end of the indirect blocks (sentinel node -1)
        if (toRead <= 0) break; //we do not need to read anymore data
        if (toskip >= BLK_SIZE){ //we want to skip this entire block
            toskip -= BLK_SIZE;
        }else{ //we want to skip part (or none) of this block
            int cur_toRead = min(BLK_SIZE - toskip, toRead); //how much to rea from this block
            data_block_number cur_blk_no;
            memcpy(&cur_blk_no, ind_blkno_p, sizeof(data_block_number)); //get cur indirect data block number from indirect block
            data_block* cur_blk = &fs->data_root[cur_blk_no];
            void * st_add = (void *) cur_blk + toskip; //location to start copying from for this block
            memcpy(tbuf, st_add, cur_toRead);
            tbuf += cur_toRead;
            toskip = 0;
            toRead -= cur_toRead;
            hasRead += cur_toRead;
        }
        ind_blkno_p++;
    }
    if (hasRead) clock_gettime(CLOCK_REALTIME, &in->atim); //change atim
    // NOTE: There shouldn't be a case where we reach the end of the indir blk before reading all of toRead, as that will mean 
    // size of inode is wrong
    fprintf(stderr, "og offset: %ld\n", *off);
    *off += hasRead; //incrememnt offset by amount read
    fprintf(stderr, "buffer info read to: %s, new offset: %ld\n", buf, *off);
    return hasRead;
}
