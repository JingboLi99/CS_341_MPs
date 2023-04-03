/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

data_block_number add_data_block_to_inode(file_system *fs_pointer,
                                          inode *node) {
    assert(fs_pointer);
    assert(node);

    int i;
    for (i = 0; i < NUM_DIRECT_BLOCKS; ++i) {
        if (node->direct[i] == -1) {
            data_block_number first_data = first_unused_data(fs_pointer);
            if (first_data == -1) {
                return -1;
            }
            node->direct[i] = first_data;
            set_data_used(fs_pointer, first_data, 1);
            return first_data;
        }
    }
    return 0;
}

data_block_number add_data_block_to_indirect_block(file_system *fs_pointer,
                                                   data_block_number *blocks) {
    assert(fs_pointer);
    assert(blocks);

    size_t i;
    for (i = 0; i < NUM_INDIRECT_BLOCKS; ++i) {
        if (blocks[i] == UNASSIGNED_NODE) {
            data_block_number first_data = first_unused_data(fs_pointer);
            if (first_data == -1) {
                return -1;
            }
            blocks[i] = first_data;
            set_data_used(fs_pointer, first_data, 1);
            return first_data;
        }
    }
    return 0;
}

inode_number add_single_indirect_block(file_system *fs_pointer, inode *node) {
    assert(fs_pointer);
    assert(node);

    if (node->indirect != UNASSIGNED_NODE)
        return 0;
    data_block_number first_data = first_unused_data(fs_pointer);
    if (first_data == -1) {
        return -1;
    }
    node->indirect = first_data;
    set_data_used(fs_pointer, first_data, 1);
    node->nlink = 1;

    size_t i;
    data_block_number *block_array =
        (data_block_number *)(fs_pointer->data_root + first_data);
    for (i = 0; i < NUM_INDIRECT_BLOCKS; ++i) {
        block_array[i] = UNASSIGNED_NODE;
    }
    return 0;
}

int minixfs_min_blockcount(file_system *fs, const char *path, int block_count) {
    inode *nody = get_inode(fs, path);
    if (!nody) {
        nody = minixfs_create_inode_for_path(fs, path);
        if (!nody)
            return -1;
    }

    data_block_number *block_array = nody->direct;
    int err = 0;
    if (block_count < NUM_DIRECT_BLOCKS) {
        block_array = nody->direct;
        for (int i = 0; i < block_count; i++) {
            if (block_array[i] == -1) {
                err = add_data_block_to_inode(fs, nody);
                if (err == -1)
                    return -1;
                memset(fs->data_root + block_array[i], 0, sizeof(data_block));
            }
        }
    } else {
        for (int i = 0; i < NUM_DIRECT_BLOCKS; i++) {
            if (block_array[i] == -1) {
                err = add_data_block_to_inode(fs, nody);
                if (err == -1)
                    return -1;
                memset(fs->data_root + block_array[i], 0, sizeof(data_block));
            }
        }
        err = add_single_indirect_block(fs, nody);
        if (err == -1)
            return -1;
        block_array = (data_block_number *)(fs->data_root + nody->indirect);
        block_count -= NUM_DIRECT_BLOCKS;
        for (int i = 0; i < block_count; i++) {
            if (block_array[i] == -1) {
                err = add_data_block_to_indirect_block(fs, block_array);
                if (err == -1)
                    return -1;
                memset(fs->data_root + block_array[i], 0, sizeof(data_block));
            }
        }
    }
    return 0;
}

int minixfs_open(file_system *fs, const char *path, int flags) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path) {
        if (!strlen(virtual_path)) {
            return 0;
        }
        for (int i = 0; i < minixfs_virtual_path_count; i++) {
            if (!strcmp(virtual_path, minixfs_virtual_path_names[i])) {
                return 0;
            }
        }

        return 1;
    }

    inode *node = get_inode(fs, path);
    if (!node) {
        if (TEST(flags, O_CREAT)) {
            (void)flags;
            inode *new_node = minixfs_create_inode_for_path(fs, path);
            if (new_node)
                return 0;
        }
        errno = ENOENT;
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &node->atim);
    return 0;
}

int minixfs_access(file_system *fs, const char *path, int mode) {
    inode *node = get_inode(fs, path);
    if (!node)
        return -1;

    if (mode == F_OK)
        return 0;

    int orig_mode = node->mode & 0777;
    int retval = 0;

    if (TEST(mode, R_OK))
        if (!TEST(orig_mode, 0400))
            retval = -1;

    if (TEST(mode, W_OK))
        if (!TEST(orig_mode, 0200))
            retval = -1;

    if (TEST(mode, X_OK))
        if (!TEST(orig_mode, 0100))
            retval = -1;

    return retval;
}

int minixfs_mkdir(file_system *fs, char *path, int mode) {
    inode *node = minixfs_create_inode_for_path(fs, path);
    if (!node) {
        return -1;
    }

    int oldbits = node->mode & 0777;
    node->mode = ((TYPE_DIRECTORY) << RWX_BITS_NUMBER);
    node->mode |= oldbits;
    node->mode |= mode;
    return 0;
}

int minixfs_unlink(file_system *fs, const char *path) {
    inode *node = get_inode(fs, path);
    if (!node)
        return -1;
    inode_number target_ino = (int)(node - fs->inode_root);
    const char *filename;
    inode *parent_node = parent_directory(fs, path, &filename);
    uint64_t size = parent_node->size;
    size_t count = 0;
    minixfs_dirent dirents;
    data_block_number *block_array = parent_node->direct;
    while (size > 0) {
        uint64_t temp = 0;
        data_block *blocky = fs->data_root + block_array[count];
        int should_break = 0;
        while (temp < sizeof(data_block) && temp < size) {
            make_dirent_from_string(((char *)blocky) + temp, &dirents);
            if (dirents.inode_num == target_ino) {
                memmove((char *)blocky + temp,
                        (char *)blocky + temp + FILE_NAME_ENTRY,
                        sizeof(data_block) - temp - FILE_NAME_ENTRY);
                should_break = 1;
                break;
            }

            temp += FILE_NAME_ENTRY;
        }
        if (should_break)
            break;
        count++;
        size -= temp;
        if (count == NUM_DIRECT_BLOCKS) {
            if (node->indirect == UNASSIGNED_NODE)
                break;
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            count = 0;
        }
    }
    parent_node->size -= FILE_NAME_ENTRY; // Add one directory to parent only
    free_inode(fs, node);
    return 0;
}

static int minixfs_stat_virtual(struct stat *buf) {
    memset(buf, 0, sizeof(struct stat));
    buf->st_mode = 0777;
    buf->st_mode |= S_IFDIR;
    return 0;
}

static int minixfs_stat_virtual_path(struct stat *buf) {
    memset(buf, 0, sizeof(struct stat));
    buf->st_mode = 0777;
    buf->st_mode |= S_IFREG;
    return 0;
}

int minixfs_stat(file_system *fs, char *path, struct stat *buf) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path) {
        if (!strlen(virtual_path))
            return minixfs_stat_virtual(buf);
        for (int i = 0; i < minixfs_virtual_path_count; i++) {
            if (!strcmp(virtual_path, minixfs_virtual_path_names[i])) {
                return minixfs_stat_virtual_path(buf);
            }
        }

        errno = ENOENT;
        return 1;
    }
    inode *node = get_inode(fs, path);
    if (!node)
        return -1;
    buf->st_size = node->size;
    buf->st_blksize = sizeof(data_block);
    buf->st_blocks = (node->size + sizeof(data_block) - 1) / sizeof(data_block);

    buf->st_nlink = (nlink_t)node->nlink;

    buf->st_mode = (mode_t)(node->mode & 0777);
    if (is_directory(node)) {
        buf->st_mode |= S_IFDIR;
    } else {
        buf->st_mode |= S_IFREG;
    }

    buf->st_uid = node->uid;
    buf->st_gid = node->gid;

    buf->st_atim = node->atim;
    buf->st_mtim = node->mtim;
    buf->st_ctim = node->ctim;

    buf->st_ino = node - fs->inode_root;
    buf->st_dev = 0;

    return 0;
}

static int minixfs_readdir_virtual(const char *virtual_path,
                                   struct dirent **entries) {
    if (strlen(virtual_path)) {
        // can't have subdirectories in virtual
        errno = ENOENT;
        return -1;
    }

    assert(*entries == NULL);
    *entries = malloc(sizeof(struct dirent) * minixfs_virtual_path_count);
    int num_entries = 0;
    for (int i = 0; i < minixfs_virtual_path_count; i++) {
        strcpy((*entries)[num_entries].d_name, minixfs_virtual_path_names[i]);
        (*entries)[num_entries].d_ino = 0;
        num_entries++;
    }
    return minixfs_virtual_path_count;
}

int minixfs_readdir(file_system *fs, const char *path,
                    struct dirent **entries) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path) {
        return minixfs_readdir_virtual(virtual_path, entries);
    }

    inode *node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &node->atim);
    if (is_file(node)) {
        errno = ENOTDIR;
        return -1;
    }
    assert(*entries == NULL);
    uint64_t size = node->size, max_entries = node->size / FILE_NAME_ENTRY;
    if (!strcmp("/", path))
        max_entries += 1; // adding entry for /virtual

    *entries = malloc(sizeof(struct dirent) * max_entries);
    int num_entries = 0;
    if (!strcmp("/", path)) {
        // adding entry for /virtual
        strcpy((*entries)[num_entries].d_name, "virtual");
        (*entries)[num_entries].d_ino = 0;
        num_entries++;
    }

    size_t count = 0;
    minixfs_dirent dirents;
    data_block_number *block_array = node->direct;
    while (size > 0) {
        uint64_t temp = 0;
        data_block *blocky = fs->data_root + block_array[count];
        while (temp < sizeof(data_block) && temp < size) {
            make_dirent_from_string(((char *)blocky) + temp, &dirents);

            size_t length = MIN(strlen(dirents.name) + 1, MAX_DIR_NAME_LEN);
            memcpy((*entries)[num_entries].d_name, dirents.name, length);
            (*entries)[num_entries].d_name[length] = '\0';
            (*entries)[num_entries].d_ino = dirents.inode_num;
            num_entries++;

            temp += FILE_NAME_ENTRY;
        }
        count++;
        size -= temp;
        if (count == NUM_DIRECT_BLOCKS) {
            if (node->indirect == UNASSIGNED_NODE)
                break;
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            count = 0;
        }
    }
    return num_entries;
}
