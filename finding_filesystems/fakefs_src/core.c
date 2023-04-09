/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
/* START CORE */
int access(const char *pathname, int mode) {
    fakefs_log("called access %s\n", pathname);
    if (disable_hooks) {
        if (!orig_access) {
            /*
             * On Ubuntu 18, for some reason access seems to be called before
             * the __attribute__((constructor)) function that sets up the hooks.
             * As a result, the orig_* functions are still NULL, leading to a
             * segfault. This if statement checks for the above case, and
             * manually HOOKs the access function to avoid this case.
             */
            HOOK(access);
        }
        return orig_access(pathname, mode);
    }

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);

    fakefs_log("Accessing %s -> %s : %s\n", pathname, paths.base_path,
               paths.virtual_path);

    if (paths.base_path && paths.virtual_path) {
        fakefs_log("called vaccess %s\n", paths.virtual_path);
        return minixfs_access(fs, paths.virtual_path, mode);
    }

    return orig_access(pathname, mode);
}

int creat(const char *pathname, mode_t mode) {
    if (disable_hooks)
        return orig_creat(pathname, mode);

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        // minixfs doesn't have a creat so we just use open
        return open(pathname, O_CREAT, mode);
    }
    return orig_creat(pathname, mode);
}

int open(const char *pathname, int flags, ...) {
    va_list a_list;
    va_start(a_list, flags);
    mode_t m = 0;
    if ((flags & O_CREAT) == O_CREAT) {
        m = va_arg(a_list, mode_t);
    }
    va_end(a_list);

    if (disable_hooks) {
        if ((flags & O_CREAT) == O_CREAT)
            return orig_open(pathname, flags, m);
        return orig_open(pathname, flags);
    }

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int v_fd = -1;
        int error = minixfs_open(fs, paths.virtual_path, flags);
        if (error >= 0) {
            v_fd = orig_open("/dev/null", O_RDONLY);

            fakefile *file = malloc(sizeof(fakefile));
            file->fd = v_fd;
            file->path = strdup(paths.virtual_path);
            file->offset = 0;
            file->refcount = 1;
            file->flags = 0;

            dictionary_set(fd_map, &v_fd, file);
            return v_fd;
        } else
            return -1;
    }

    if ((flags & O_CREAT) == O_CREAT) {
        return orig_open(pathname, flags, m);
    }
    return orig_open(pathname, flags);
}

ssize_t read(int fd, void *buf, size_t count) {
    if (disable_hooks)
        return orig_read(fd, buf, count);

    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        ssize_t m_read =
            minixfs_read(fs, file->path, buf, count, &(file->offset));
        return m_read;
    }

    return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (disable_hooks)
        return orig_write(fd, buf, count);
    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        ssize_t res =
            minixfs_write(fs, file->path, buf, count, &(file->offset));
        return res;
    }
    return orig_write(fd, buf, count);
}
int close(int fd) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_close(fd);

    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    dictionary_remove(fd_map, &fd);
    file->refcount--;
    if (!file->refcount)
        destroy_fakefile(file);
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_lseek(fd, offset, whence);

    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    struct stat buf;

    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        if (fstat(fd, &buf) >= 0)
            file->offset = buf.st_size + offset;
        else
            return -1;
        break;
    default:
        break;
    }
    return file->offset;
}

int unlink(const char *pathname) {
    if (disable_hooks)
        return orig_unlink(pathname);

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int retval = minixfs_unlink(fs, paths.virtual_path);
        return retval;
    }
    return orig_unlink(pathname);
}

int mkdir(const char *pathname, mode_t mode) {
    if (disable_hooks)
        return orig_mkdir(pathname, mode);

    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int retval = minixfs_mkdir(fs, paths.virtual_path, (int)mode);
        return retval;
    }
    return orig_mkdir(pathname, mode);
}

int chmod(const char *pathname, mode_t mode) {
    int fd = open(pathname, O_WRONLY);
    int retval = fchmod(fd, mode);
    close(fd);
    return retval;
}

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);
    if (paths.base_path && paths.virtual_path) {
        int retval = chmod(pathname, mode);
        return retval;
    }

    return orig_fchmodat(dirfd, pathname, mode, flags);
}

int fchmod(int fd, mode_t mode) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_fchmod(fd, mode);
    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    // fakefs_log("[FCHMOD] %s\n", file->path);
    minixfs_chmod(fs, file->path, (int)mode);
    return 0;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    return lchown(pathname, owner, group);
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
    int fd = open(pathname, O_WRONLY);
    int retval = fchown(fd, owner, group);
    close(fd);
    return retval;
}

int fchown(int fd, uid_t owner, gid_t group) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_fchown(fd, owner, group);
    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    // fakefs_log("[FCHOWN] %s %d\n", file->path, (int)owner);
    minixfs_chown(fs, file->path, owner, group);
    return 0;
}

int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group,
             int flags) {
    fakefs_path_t paths = fakefs_realpath(pathname);
    auto_cleanup_path(paths);

    if (paths.base_path && paths.virtual_path)
        return chown(pathname, owner, group);

    return orig_fchownat(dirfd, pathname, owner, group, flags);
}

/* END CORE */
