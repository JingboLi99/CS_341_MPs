/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#pragma once

#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define KILOBYTE (1024)

#define INODES_NUMBER (1024)
#define DATA_NUMBER (2 * KILOBYTE)
#define FILE_NAME_LENGTH (248)
#define FILE_NAME_ENTRY (FILE_NAME_LENGTH + 8)
#define UNASSIGNED_NODE (-1)
#define MAX_DIR_NAME_LEN 256
// Checks if a flag is included in a set
#define TEST(set, flag) ((set & flag) == flag)

typedef struct {
    int fd;
    size_t offset;
    size_t filesize;
} inode_info;

// enum that represents the possible filetypes
// stored inside the mode of a file
enum FILE_TYPE {
    TYPE_FILE = 1,
    TYPE_DIRECTORY = 2,
    TYPE_SINGLE_INDIRECT = 3,
    TYPE_PIPE = 4,
};

#define NUM_DIRECT_BLOCKS 11
#define RWX_BITS_NUMBER 9
#define INODE_SIZE 8

typedef int data_block_number;
typedef int inode_number;
#define NUM_INDIRECT_BLOCKS (sizeof(data_block) / sizeof(data_block_number))

typedef struct {
    uint64_t size;         /* Size of the mmapped region in bytes*/
    uint64_t inode_count;  /* Multiple of 64 */
    uint64_t dblock_count; /* Multiple of 64 */
} superblock;

/**
 * The data map records used blocks and is stored at the end of the filesystem.
 * It is an array that stores 1 if a particular block is used and 0 otherwise.
 * The data map is stored at the end so that the filesystem can be easily
 * resized in the future - currently this behavior is not supported
 */
#define GET_DATA_MAP(superblk_ptr)                        \
    ((char *)((void *)superblk_ptr + superblk_ptr->size - \
              superblk_ptr->dblock_count))
/**
 * An inode is set if the num_hard_links is 0.
 *
 * After that block we have dblock_count bytes, byte i
 * is 0 if that corresponding data block is free
 *
 * The superblock is exactly 1 kilobyte in size maximum
 *
 * You _do not_ need to worry about this implementation
 * this is abstracted away from you!
 */

/**
 * This struct represents an inode
 * It is padded to be perfectly aligned to a 64 byte boundary
 */
typedef struct {
    uid_t uid; /* user ID of owner*/
    gid_t gid; /* group ID of owner */

    /**
     *  Permissions
     *  Bits 9-0 are rwxrwxrwx
     *  Bits 11-9 are one of directory<d>, file<f>,
     *    character<c>, and pipe<p> in the provided enum
     *    the character and pipe aren't used in this lab
     *    but are there for realism and whatnot
     *
     *  HINT: if you want to change mode you should be careful
     *   not to corrupt the type.
     *   We've defined the number of permission bits in RWX_BITS_NUMBER
     *   Thus, you can get the type using the following code:
     *    int type = mode >> RWX_BITS_NUMBER
     *   And set the type using:
     *    mode |= type << RWX_BITS_NUMBER
     *
     */
    uint16_t mode; /* <d,f,c,p>rwxrwxrwx */

    /**
     *   Hard link count
     *  The number of links to the file/directory equals the number
     *    of times it appears in a directory. The inode is considered
     *    a free inode once this count equals 0
     */
    uint32_t nlink; /* reference count, when hit 0 */

    /**
     *  Updated any time any time this inode is read from
     *    For a directory: Every time you've ls'ed or cat'ed directory
     *    For a file: Every time you've cat'ed a file
     */
    struct timespec atim; /* time of last access */

    /**
     *  Updated any data change
     *    For directories: (mkdir, touch)
     *    For files: cp, other mutation functions
     */
    struct timespec mtim; /* time of last modification */

    /**
     *  Updated any metadata change (links, chmod, ...)
     */
    struct timespec ctim; /* time of last status change */

    uint64_t size; /* total size, in bytes */

    /**
     *  Holds the index of the direct data_block
     *
     *  The number of set blocks is continuous
     *    and relates to the size. ceil(size/sizeof(data_block))
     */
    data_block_number direct[NUM_DIRECT_BLOCKS];
    data_block_number indirect; /* points to a singly indirect block */
} inode;

typedef struct {
    char data[16 * KILOBYTE];
} data_block;

typedef struct {
    superblock *meta;  /* Stores the information about system */
    inode *inode_root; /* Pointer to the beginning of an array of inodes */
    data_block
        *data_root; /* Pointer to the beginning of an array of data_blocks */
} file_system;

typedef struct {
    char *name;
    inode_number inode_num;
} minixfs_dirent;

extern char *minixfs_virtual_path_names[];
extern int minixfs_virtual_path_count;

//----------------------------------
// FUNCTIONS YOU NEED TO IMPLEMENT:
//----------------------------------

/**
 *  Creates a file located at path, return NULL if inode already exists or
 * cannot be created.
 *
 * This will also need to write a dirent to disk. See make_string_from_dirent.
 * Note that if the file does not exist, you will need to find its parent inode,
 * add a new dirent to the parent and then increase the parent's size.
 *
 * You will also need to check that the path is a valid pathname, (see
 * valid_filename)
 */
inode *minixfs_create_inode_for_path(file_system *fs, const char *path);

/**
 *  chmod takes the inode at path and a set of permissions
 *  and sets the permissions.
 *
 *  Note that you should only overwrite the bottom RWX_BITS_NUMBER bytes.
 *  The top 7 bits are reserved for the type of the file.
 *
 *  This function should update the node's ctim
 *
 *  Return value:
 *    On success return 0
 *    On failure return -1
 *      If this functions fails because path doesn't exist,
 *      set errno to ENOENT
 *
 */
int minixfs_chmod(file_system *fs, char *path, int new_permissions);

/**
 *  chown takes the inode at path and a new owner and group,
 *  and sets the uid and gid with the supplied values.
 *
 *  If owner is ((uid_t)-1), then don't change the node's uid.
 *  If group is ((gid_t)-1), then don't change the node's gid.
 *
 *  This function should update the node's ctim
 *
 *  Return value:
 *    On success return 0
 *    On failure return -1
 *      If this function fails because path doesn't exist,
 *      set errno to ENOENT
 *
 */
int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group);

/**
 *  This function allows you to write to a file. On success `count`
 *  bytes will be copied from buff to the inode at an offset of `*off`.
 *
 *  After writing successfully, *off should be incremented by the number of
 *  bytes written. If the file size has changed because of the write, the node's
 *  size should be updated.
 *
 *  Note: You are responsible for allocating blocks of memory for the nodes. You
 *  might find the following functions useful:
 *    minixfs_min_blockcount
 *    add_data_block_to_inode
 *    add_single_indirect_block
 *    add_data_block_to_indirect_block
 *
 *  This function should update the node's mtim and atim
 *
 *  Return value:
 *    On success return the number of bytes written (should equal to `count`)
 *    On failure return -1
 *      If this function is requested to write more bytes than the maximum
 *      possible filesize (calculate this from NUM_DIRECT_BLOCKS and
 *      NUM_INDIRECT_BLOCKS) fail without writing any data and set errno to
 * ENOSPC
 *
 *      If this function fails because no more data blocks can be allocated,
 *      set errno to ENOSPC
 *
 *      Note that this function cannot fail with ENOENT, since it should just
 *      create the file if it doesn't exist
 *
 *  Example:
 *  Suppse the file '/text' contains the following:
 *    Hello.
 *  And has a size of 6 bytes (that is the file only contains the characters
 *  'H','e','l','l','o','.')
 *  Say, we then execute the following code:
 *    char *str = " World!"
 *    off_t off = 5;
 *    mininxfs_write(fs, "/text", str, 7, &off);
 *  The file will then contain:
 *    Hello World!
 *  And have a size of 12. The value of `*off` will also be 12.
 *  Note that in this example, there is no trailing null byte in the file. This
 *  is because files just contain binary data and keep track of how many bytes
 *  the file has in the inode metadata.
 *
 * Additionally, `touch` is provided to create new files when testing.
 */
ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off);
/**
 *  This function allows you to read a file. On success
 *  `min(count, size_of_file - *off)` bytes will be copied from the file into
 *  buf to the inode after an offset of `*off`.
 *
 *  After reading successfully, *off should be incremented by the number of
 * bytes read.
 *
 *  Note: reading less than count bytes becuase the end of the file is reached
 *  counts as a successful read. If *off is greater than the end of the file,
 *  do nothing and return 0 to indicate end of file.
 *
 *  This function should update the node's atim
 *
 *  Return value:
 *    On success return the number of bytes read
 *    On failure return -1
 *      If this function fails because the path does not exist,
 *      set errno to ENOENT
 *
 *  Example:
 *  Suppse the file '/text' contains the following:
 *    Hello World!
 *  And has a size of 12 bytes. Say, we then execute the following code:
 *    char *str = malloc(6);
 *    off_t off = 6;
 *    mininxfs_read(fs, "/text", str, 5, &off);
 *    str[5] = 0;
 *    puts(str);
 *  The program will then print:
 *    World
 *  and `*off` will be 11.
 *  If the program then executed the following:
 *    memset(str, 0, 6);
 *    mininxfs_read(fs, "/text", str, 5, &off);
 *    puts(str);
 *  The program will the print:
 *    !
 *  and any subsequent reads will return 0. Make sure your implementation of
 *  read returns the right number of bytes
 */
ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off);

//----------------------------------
// IMPORTANT PROVIDED FUNCTIONS:
//----------------------------------

/**
 * Open a file and perform any associated actions.
 * Return 0 on success, 1 otherwise.
 */
int minixfs_open(file_system *fs, const char *path, int flags);

/**
 * Decrement the reference count on a file. If the reference count is 0, remove
 * the file and free all associated blocks.
 */
int minixfs_unlink(file_system *fs, const char *path);

/**
 * Determine if a file can be accessed given the provided mode.
 */
int minixfs_access(file_system *fs, const char *path, int mode);

/**
 * Create a directory at path.
 */
int minixfs_mkdir(file_system *fs, char *path, int mode);

/**
 * This function will first check if `path` exists. If it does, it will ensure
 * that the inode that represents `path` has at least `block_count` many data
 * blocks including both direct and indirect data blocks.
 *
 * If that file already has `block_count` blocks, 0 will be retunred. If it
 * does not, it will allocate blocks for it.
 *
 * On error -1 will be returned, but errno will NOT be set.
 */
int minixfs_min_blockcount(file_system *fs, const char *path, int block_count);

/**
 *  This function is very similar to stat(2). It will populate the passed in
 * buffer
 *  with the file's attributes.
 */
int minixfs_stat(file_system *fs, char *paths, struct stat *buf);

/**
 *  Adds an indirect block if already not connected to another block
 *  Returns -1 if the number of data blocks is full, and 0 if already connected
 * to a block.
 */
inode_number add_single_indirect_block(file_system *fs_pointer, inode *node);

/**
 *  Adds a data block if not already full
 *  Returns -1 if the number of data_blocks is full already
 */
data_block_number add_data_block_to_inode(file_system *fs_pointer, inode *node);

/**
 *  Adds a data block (to an indirect block)
 *       if not already full
 *  Returns -1 on no more data_blocks or if
 *  the number of data_blocks is full already
 */
data_block_number add_data_block_to_indirect_block(file_system *fs_pointer,
                                                   data_block_number *blocks);

/**
 *  Returns the inode in the filesystem where
 *    path = '/path/to/file'
 *        '/path/to/dir/'
 *        '/path/to/dir'
 *    (forward slash on the directory optional)
 *  Returns NULL if inode doesn't exist
 */
inode *get_inode(file_system *fs, const char *path);

/**
 *  Returns true if the inode represents a file
 *  Undefined behavior if passed a single indirect block
 */
int is_file(inode *node);

/**
 *  Returns true if the inode represents a directory
 *  Undefined behavior if passed a single indirect block
 */
int is_directory(inode *node);

/**
 *  This function reads all directory entries from a directory and stores them
 * in
 *  an array. You can assume that `*entries == NULL`, and should allocate memory
 *  for the entries.
 *
 *  If the inode for the file denoted by `path` is a file and not a directory
 *  (you can test this with the is_file function implemented in
 *  minixfs_utils.c) then return -1 and set errno apprpriately (see below)
 *
 *  `struct dirent` is the real directory entry structure used by linux. If you
 *  see the man page for readdir(3) you will see the following definition:
 *
 *    struct dirent {
 *        ino_t          d_ino;
 *        off_t          d_off;
 *        unsigned short d_reclen;
 *        unsigned char  d_type;
 *        char           d_name[256];
 *    }
 *
 *  The only fields you need to populate are d_ino and d_name. To do this, you
 *  must read the entire file indicated by `path`. You should read the file
 *  FILE_NAME_ENTRY bytes at a time (where FILE_NAME_ENTRY is a macro defined
 *  for you in this header file). You can then use the make_dirent_from_string
 *  function (which you can find a prototype for in this file and an
 *  implementation in minixfs_utils.c) to make a dirent object (this is in terms
 *  of a struct defined in this header, not the real linux dirent) and copy
 *  over the inode number and name.
 *
 *  Note: The entries in the array you return must be in the same order in
 *  which they are present in the file (this should be sorted by inode number).
 *
 *  This function should update the node's atim
 *
 *  Hint: The number of entries you need will be inode->size/FILE_NAME_ENTRY.
 *
 *  Return value:
 *    On success return number of entries
 *    On failure return -1
 *      If this function fails because `path` is not a directory,
 *      set errno to ENOTDIR
 *      If this function fails because `path` does not exist,
 *      set errno to ENOENT
 */
int minixfs_readdir(file_system *fs, const char *path, struct dirent **entries);
