/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
/* START STAT */

static int fstat_common(int fd, struct stat *buf, int *retval) {
    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        *retval = minixfs_stat(fs, file->path, buf);
        return 1;
    }
    return 0;
}
int __xstat64(int ver, const char *pathname, struct stat64 *buf) {
    return __lxstat64(ver, pathname, buf);
}
int __lxstat64(int ver, const char *pathname, struct stat64 *buf) {
    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int fd = open(pathname, O_RDONLY);
        if (fd == -1)
            return -1;
        int retval = __fxstat64(ver, fd, buf);
        close(fd);
        return retval;
    }
    return orig_lstat64(ver, pathname, buf);
}
int __fxstat64(int ver, int fd, struct stat64 *buf) {
    if (disable_hooks)
        return orig_fstat64(ver, fd, buf);
    int retval;
    int hooked = fstat_common(fd, (struct stat *)buf, &retval);
    if (hooked)
        return retval;

    return orig_fstat64(ver, fd, buf);
}

int __fxstatat64(int ver, int dirfd, const char *pathname, struct stat64 *buf,
                 int flags) {
    if (disable_hooks)
        return orig_fstatat64(ver, dirfd, pathname, buf, flags);

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int fd = open(pathname, O_RDONLY);
        if (fd == -1)
            return -1;
        int retval = __fxstat64(ver, fd, buf);
        close(fd);
        return retval;
    }
    return orig_fstatat64(ver, dirfd, pathname, buf, flags);
}

int __xstat(int ver, const char *pathname, struct stat *buf) {
    return __lxstat(ver, pathname, buf);
}

int __lxstat(int ver, const char *pathname, struct stat *buf) {
    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
        return -1;
    int retval = __fxstat(ver, fd, buf);
    close(fd);
    return retval;
}
int __fxstat(int ver, int fd, struct stat *buf) {
    if (disable_hooks)
        return orig_fstat(0, fd, buf);
    int retval;
    int hooked = fstat_common(fd, buf, &retval);
    if (hooked)
        return retval;
    return orig_fstat(ver, fd, buf);
}

int __fxstatat(int ver, int dirfd, const char *pathname, struct stat *buf,
               int flags) {
    if (disable_hooks)
        return orig_fstatat(ver, dirfd, pathname, buf, flags);

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int fd = open(pathname, O_RDONLY);
        if (fd == -1)
            return -1;
        int retval = __fxstat(ver, fd, buf);
        close(fd);
        return retval;
    }

    return orig_fstatat(ver, dirfd, pathname, buf, flags);
}

int lstat(const char *pathname, struct stat *buf) {
    return __lxstat(0, pathname, buf);
}
int stat(const char *pathname, struct stat *buf) {
    return __xstat(0, pathname, buf);
}

int fstat(int fd, struct stat *buf) {
    return __fxstat(0, fd, buf);
}
/* END STAT */
