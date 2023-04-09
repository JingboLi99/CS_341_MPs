/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
/* START DUP */

int dup2(int oldfd, int newfd) {
    if (disable_hooks || !dictionary_contains(fd_map, &oldfd))
        return orig_dup2(oldfd, newfd);

    fakefile *file = *(dictionary_at(fd_map, &oldfd).value);
    orig_dup2(oldfd, newfd);
    dictionary_set(fd_map, &newfd, file);
    return 1;
}

int dup(int oldfd) {
    if (disable_hooks || !dictionary_contains(fd_map, &oldfd))
        return orig_dup(oldfd);

    fakefile *file = *(dictionary_at(fd_map, &oldfd).value);
    int newfd = orig_open("/dev/null", file->flags);
    if (newfd > 0) {
        dictionary_set(fd_map, &newfd, file);
        return newfd;
    }
    return newfd;
}
/* END DUP */
