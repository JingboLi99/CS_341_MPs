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
    uint16_t new_bits = new_permissions & 0xFF;
    inode * in = get_inode(fs, path); //get the inode of path
    if (in){ //if the path exists
        uint16_t in_md = in->mode; //get the current mode int
        in_md = (in_md & 0xFE00) | new_bits; //clear the bottom 9 bits and set to the new 9 bits
        in->mode = in_md;
        clock_gettime(CLOCK_REALTIME, &in->ctim); //change ctim
    }
    else{
        errno = ENOENT;
        perror("CHMOD ERROR: ");
        return -1;
    }
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode * in = get_inode(fs, path);
    if (in){
        bool success = false; //checks if any modifications have been made
        if (owner != (uid_t) -1){ //check if new uid to change to is valid
            in->uid = owner;
            success = true;
        }
        if (group != (gid_t) -1){ //check if new gid to change to is valid
            in->gid = group;
            success = true;
        }
        if (success){
            clock_gettime(CLOCK_REALTIME, &in->ctim); //change ctim
            return 0;
        }
        return -1;
    }
    else{
        errno = ENOENT;
        perror("CHOWN ERROR: ");
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

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
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
        perror("READ ERROR: path does not exist- ");
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
