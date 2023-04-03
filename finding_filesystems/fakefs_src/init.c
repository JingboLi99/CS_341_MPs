/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
void __attribute__((destructor)) fs_fini(void) {
    // fakefs_log("Goodbye!\n");
    if (regex_init)
        regfree(&root_match);

    if (root_filepath) {
        vector *keys = dictionary_keys(fd_map);
        VECTOR_FOR_EACH(keys, elem, { close(*(int *)elem); });
        vector_destroy(keys);
        // dictionary_destroy(fd_map);

        keys = dictionary_keys(fd_map);
        VECTOR_FOR_EACH(keys, elem, { closedir(*(DIR **)elem); });
        vector_destroy(keys);
        // dictionary_destroy(dir_fd_map);
        disable_hooks = 1;
        // close_fs(&fs);
    }
    free(root_filepath);
    free(root_filename);
}

static char *get_filename(char *full_path) {
    size_t idx = strlen(full_path) - 1;
    while (idx && full_path[idx] != '/')
        idx--;
    if (full_path[idx] != '/')
        return strdup(full_path);

    return strdup(full_path + idx + 1);
}

void __attribute__((constructor)) fs_init(void) {
    HOOK(access);
    HOOK(creat);
    HOOK(open);
    HOOK(read);
    HOOK(write);
    HOOK(close);
    HOOK(lseek);
    HOOK(unlink);
    HOOK(mkdir);
    HOOK(fchmod);
    HOOK(fchmodat);
    HOOK(fchown);
    HOOK(fchownat);

    HOOK(fsync);
    HOOK(fdatasync);

    HOOK(dup);
    HOOK(dup2);

    HOOK2(stat, __xstat);
    HOOK2(lstat, __lxstat);
    HOOK2(fstat, __fxstat);
    HOOK2(fstatat, __fxstatat);
    HOOK2(stat64, __xstat64);
    HOOK2(lstat64, __lxstat64);
    HOOK2(fstat64, __fxstat64);
    HOOK2(fstatat64, __fxstatat64);

    HOOK(fdopendir);
    HOOK(opendir);
    HOOK(readdir);
    HOOK(readdir64);
    HOOK(closedir);

    HOOK(mmap);

    HOOK(chdir);

    char *root = getenv("MINIX_ROOT");
    if (root) {
        root_filepath = realpath(root, NULL);
        if (root_filepath)
            root_filename = get_filename(root_filepath);

        fs = open_fs(root);
        fd_map = int_to_shallow_dictionary_create();
        dir_fd_map = shallow_to_shallow_dictionary_create();
    }

    char *logging = getenv("FAKEFS_DEBUG");
    if (logging)
        enable_logging = 1;

    fakefs_log("Logging enabled!");

    disable_hooks = 0;
}
