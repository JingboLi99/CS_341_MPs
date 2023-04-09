/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
int fsync(int fd) {
    FD_UNIMPLEMENTED(fd, 0);
    return orig_fsync(fd);
}

int fdatasync(int fd) {
    FD_UNIMPLEMENTED(fd, 0);
    return orig_fdatasync(fd);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
    FD_UNIMPLEMENTED(fd, NULL);
    return orig_mmap(addr, length, prot, flags, fd, offset);
}
