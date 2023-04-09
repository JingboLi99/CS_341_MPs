/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

file_system *open_fs(const char *path) {
    if (!path) {
        return NULL;
    }
    struct stat file_stat;
    int open_flags = O_RDWR;
    int mmap_flags = PROT_READ | PROT_WRITE;

    int fd = open(path, open_flags);
    if (fd == -1) {
        printf("Failed to open file: %s\n", path);
        exit(1);
    }
    if (fstat(fd, &file_stat) != 0) {
        printf("Failed to stat file: %s\n", path);
        exit(1);
    }
    char *file =
        mmap(NULL, (size_t)file_stat.st_size, mmap_flags, MAP_SHARED, fd, 0);
    if (file == (void *)-1) {
        printf("Failed to map: %s\n", path);
        exit(1);
    }
    close(fd);
    superblock *metadata = (void *)file;
    file_system *my_fs = malloc(sizeof(*my_fs));
    my_fs->meta = (void *)file;
    my_fs->inode_root = (void *)(file + sizeof(superblock));
    my_fs->data_root = (void *)(my_fs->inode_root + metadata->inode_count);

    return my_fs;
}

void close_fs(file_system **fs_pointer) {
    assert(fs_pointer);
    assert(*fs_pointer);
    superblock *meta = (*fs_pointer)->meta;
    munmap(meta, meta->size);
    free(*fs_pointer);
    *fs_pointer = NULL;
}

void free_inode(file_system *fs_pointer, inode *node) {
    node->nlink = 0;
    data_block_number *block_array = node->direct;
    for (int i = 0; i < NUM_DIRECT_BLOCKS; ++i)
        if (block_array[i] != UNASSIGNED_NODE)
            free_data_block(fs_pointer, block_array[i]);

    if (node->indirect != UNASSIGNED_NODE) {
        block_array =
            (data_block_number *)(fs_pointer->data_root + node->indirect);
        for (int i = 0; i < NUM_DIRECT_BLOCKS; ++i)
            if (block_array[i] != UNASSIGNED_NODE)
                free_data_block(fs_pointer, block_array[i]);
    }
    free_data_block(fs_pointer, node->indirect);
}

void free_data_block(file_system *fs_pointer, data_block_number data_number) {
    set_data_used(fs_pointer, data_number, 0);
}

void set_data_used(file_system *fs_pointer, data_block_number data_number,
                   int used) {
    used = !!(used); // Gives a hard 1 or 0 prediction
    if (data_number < 0 ||
        (uint64_t)data_number >= fs_pointer->meta->dblock_count) {
        return;
    }
    GET_DATA_MAP(fs_pointer->meta)[data_number] = used;
}

data_block_number get_data_used(file_system *fs_pointer, int data_number) {
    if (data_number < 0 ||
        (uint64_t)data_number >= fs_pointer->meta->dblock_count) {
        return UNASSIGNED_NODE;
    }
    return GET_DATA_MAP(fs_pointer->meta)[data_number];
}

inode_number first_unused_inode(file_system *fs_pointer) {
    assert(fs_pointer);

    uint64_t i;
    for (i = 1; i < INODES_NUMBER; ++i) {
        int used = fs_pointer->inode_root[i].nlink;
        if (used == 0) {
            return (inode_number)i;
        }
    }
    return -1;
}

data_block_number first_unused_data(file_system *fs_pointer) {
    assert(fs_pointer);

    uint64_t i;
    uint64_t filled = 0;
    uint64_t dblock_count = fs_pointer->meta->dblock_count;
    while (filled < dblock_count) {
        filled = 0;
        for (i = 0; i < dblock_count; ++i) {
            int used = get_data_used(fs_pointer, i);
            if (!used && rand() % 2) {
                return (data_block_number)i;
            } else if (used) {
                filled++;
            }
        }
    }
    return -1;
}

inode *parent_directory(file_system *fs, const char *path,
                        const char **filename) {
    assert(fs);
    assert(path);

    int len = (int)strlen(path);
    const char *endptr = path + len;
    while (*endptr != '/') {
        endptr--;
    }

    if (filename) {
        *filename = endptr + 1;
    }
    char *parent_path = malloc(endptr - path + strlen("/") + 1);
    strncpy(parent_path, path, endptr - path + 1);
    parent_path[endptr - path + 1] = '\0';
    inode *nody = get_inode(fs, parent_path);
    free(parent_path);
    return nody;
}

int valid_filename(const char *filename) {
    assert(filename);

    if (*filename == '\0') {
        return 0;
    }
    while (*filename) {
        if (*filename == '/') {
            return 0;
        }
        filename++;
    }
    return 1;
}

void init_inode(inode *parent, inode *init) {
    assert(parent);
    assert(init);

    init->uid = parent->uid;
    init->gid = parent->gid;
    init->mode = (TYPE_FILE << RWX_BITS_NUMBER) | (parent->mode & 0777);
    init->nlink = 1;
    clock_gettime(CLOCK_REALTIME, &init->atim);
    clock_gettime(CLOCK_REALTIME, &init->mtim);
    clock_gettime(CLOCK_REALTIME, &init->ctim);
    init->size = 0;
    init->indirect = UNASSIGNED_NODE;
    int i;
    for (i = 0; i < NUM_DIRECT_BLOCKS; ++i)
        init->direct[i] = UNASSIGNED_NODE;
}

inode *find_inode_named(file_system *fs, inode *root, const char *name) {
    assert(fs);
    assert(root);
    assert(name);

    uint64_t size = root->size;
    int count = 0;
    minixfs_dirent node;
    data_block_number *block_array = root->direct;
    while (size > 0 && root->direct[count] != UNASSIGNED_NODE) {
        char *block = (char *)(fs->data_root + block_array[count]);
        char *endptr = block + sizeof(data_block);
        while (size > 0 && block < endptr) {
            make_dirent_from_string(block, &node);
            if (strcmp(name, node.name) == 0) {
                return fs->inode_root + node.inode_num;
            }
            block += FILE_NAME_ENTRY;
            size -= FILE_NAME_ENTRY;
        }
        count++;
        if (count == NUM_DIRECT_BLOCKS) {
            if (root->indirect == UNASSIGNED_NODE) {
                break;
            }
            block_array = (data_block_number *)(fs->data_root + root->indirect);
            count = 0;
        }
    }
    return NULL;
}

inode *get_inode(file_system *fs, const char *path) {
    assert(fs);
    assert(path);

    if (*path == '\0') {
        return fs->inode_root;
    }
    if (*path != '/') {
        return NULL;
    }
    char *path_cpy = strdup(path);
    char *tok = strtok(path_cpy, "/");
    inode *node = fs->inode_root;
    while (node && tok && *tok != '\0') {
        if (!is_directory(node)) {
            node = NULL;
            continue;
        }
        node = find_inode_named(fs, node, tok);
        if (node) {
            tok = strtok(NULL, "/");
        }
    }
    free(path_cpy);
    return node;
}

int is_file(inode *node) {
    return (node->mode >> RWX_BITS_NUMBER) == TYPE_FILE;
}

int is_directory(inode *node) {
    return (node->mode >> RWX_BITS_NUMBER) == TYPE_DIRECTORY;
}

int make_dirent_from_string(char *block, minixfs_dirent *to_fill) {
    char inode_number[INODE_SIZE + 1];
    memcpy(inode_number, block + FILE_NAME_LENGTH, INODE_SIZE);
    inode_number[INODE_SIZE] = '\0';
    long offset = strtoll(inode_number, NULL, 16);
    if (offset == 0) {
        perror("strtoll:");
        return 0;
    }
    to_fill->inode_num = (int)offset;
    to_fill->name = block;
    return 1;
}

void make_string_from_dirent(char *block, const minixfs_dirent source) {
    char *cpy = calloc(1, FILE_NAME_LENGTH);
    strncpy(cpy, source.name, FILE_NAME_LENGTH);
    memcpy(block, cpy, FILE_NAME_LENGTH);
    sprintf(block + FILE_NAME_LENGTH, "%08zx", (size_t)source.inode_num);
    free(cpy);
}

void minixfs_mkfs(char *args) {
    size_t size = sizeof(superblock) + /* Size of data_map */ DATA_NUMBER;
    size += sizeof(inode) * INODES_NUMBER + sizeof(data_block) * DATA_NUMBER;
    FILE *pf = fopen(args, "w");
    if (!pf) {
        printf("Error, File could not be opened\n");
        exit(1);
    }
    fseek(pf, size, SEEK_SET);
    fputc('\0', pf); /* File is gaurenteed to be zeros at this point */
    fclose(pf);

    file_system *my_fs = open_fs(args);
    inode *root = my_fs->inode_root;
    // Initialize Metadata
    my_fs->meta->size = size;
    my_fs->meta->inode_count = INODES_NUMBER;
    my_fs->meta->dblock_count = DATA_NUMBER;

    // Set up root directory as 0
    // 0 is root
    root->uid = 0;
    root->gid = 0;
    root->mode = 0755 | (TYPE_DIRECTORY << RWX_BITS_NUMBER);
    root->indirect = UNASSIGNED_NODE;
    root->nlink = 1;
    int i;
    for (i = 0; i < NUM_DIRECT_BLOCKS; ++i)
        root->direct[i] = UNASSIGNED_NODE;
    clock_gettime(CLOCK_REALTIME, &root->atim);
    clock_gettime(CLOCK_REALTIME, &root->mtim);
    clock_gettime(CLOCK_REALTIME, &root->ctim);
    // Initialize it to one data block size as of right now
    root->size = 0;

    close_fs(&my_fs);
}

const char *is_virtual_path(const char *path) {
    if (!strcmp(path, "/virtual"))
        return path + strlen("/virtual");

    if (!strncmp(path, "/virtual", strlen("/virtual"))) {
        path += strlen("/virtual");
        if (*path == '/') {
            path++;
            return path;
        }
    }

    return NULL;
}
