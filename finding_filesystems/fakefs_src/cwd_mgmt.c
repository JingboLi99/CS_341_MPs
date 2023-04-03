/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#ifdef _ENABLE_CWD_MGMT
/**
 * The functions in this file are still experimental and don't work as intended.
 * Using them can cause unintended consequences. Enable at your own risk
 */
static char *minixfs_cwd = NULL;

char *getcwd(char *buf, size_t size) {
    if (disable_hooks || !minixfs_cwd)
        return getcwd(buf, size);

    if (!buf)
        return strdup(minixfs_cwd);

    size = size > (strlen(minixfs_cwd) + 1) ? (strlen(minixfs_cwd) + 1) : size;
    strncpy(buf, minixfs_cwd, size);
    buf[size - 1] = 0;
    return buf;
}

int chdir(const char *path) {
    // TODO remove chdir
    fakefs_log("CHDIR WAS CALLED %s\n", path);
    fakefs_path_t paths = fakefs_realpath(path);
    auto_cleanup_path(paths);
    fakefs_log(" root_filename: %s; root_filepath: %s\n", root_filename,
               root_filepath);
    fakefs_log(" BASE_PATH: %s; V_PATH: %s\n", paths.base_path,
               paths.virtual_path);

    if (disable_hooks || !paths.virtual_path) {
        int err = orig_chdir(path);
        if (!err) {
            free(minixfs_cwd);
            minixfs_cwd = NULL;
        }
        return err;
    }

    // TODO check path is dir
    if (!access(path, F_OK)) {
        asprintf(&minixfs_cwd, "%s%s", paths.base_path, paths.virtual_path);
        fakefs_log("Setting PWD %s\n", minixfs_cwd);
        setenv("PWD", minixfs_cwd, 1);
        return 0;
    }

    // TODO set errno
    return -1;
}
#endif
