/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
/* START DIR */
DIR *fdopendir(int fd) {
    if (dictionary_contains(fd_map, &fd)) {
        fakedir *dir = malloc(sizeof(fakedir));
        dir->fd = fd;
        dir->entries_read = 0;
        dir->max_entries = 0;
        dir->entry = NULL;

        fakefile *file = *(dictionary_at(fd_map, &fd).value);

        dir->max_entries = minixfs_readdir(fs, file->path, &(dir->entry));
        if (dir->max_entries < 0) {
            free(dir);
            return NULL;
        }

        DIR *realdir = orig_opendir("/dev/");
        dictionary_set(dir_fd_map, realdir, dir);

        return realdir;
    }
    return orig_fdopendir(fd);
}

DIR *opendir(const char *name) {
    int fd = open(name, O_DIRECTORY);
    DIR *dirp = fdopendir(fd);
    return dirp;
}

struct dirent64 *readdir64(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        return (struct dirent64 *)readdir(dirp);
    }
    return orig_readdir64(dirp);
}

struct dirent *readdir(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        if (dir->entries_read >= dir->max_entries) {
            return NULL;
        }
        struct dirent *entry = &(dir->entry[dir->entries_read]);
        dir->entries_read++;
        return entry;
    }
    return orig_readdir(dirp);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        if (dir->entries_read >= dir->max_entries) {
            *result = NULL;
            return 0;
        }
        struct dirent *entry = &(dir->entry[dir->entries_read]);
        dir->entries_read++;
        *result = entry;
        return 0;
    }
    return orig_readdir_r(dirp, entry, result);
}

int closedir(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        close(dir->fd);
        dictionary_remove(dir_fd_map, dirp);
        destroy_fakedir(dir);
        return 0;
    }
    return orig_closedir(dirp);
}
/* END DIR */
