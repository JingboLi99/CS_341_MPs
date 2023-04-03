/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
int main(int argc, char **argv) {
    if (argc == 3) {
        if (!strcmp(argv[1], "mkfs")) {
            disable_hooks = 1;
            minixfs_mkfs(argv[2]);
            return 0;
        }
    }
    if (argc < 3) {
        fakefs_log("%s <minix_file> <command> [command_args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fakefs_so = getenv("FAKEFS_SO");
    if (!fakefs_so)
        fakefs_so = "fakefs.so";
    char *custom_preload = load_LD(fakefs_so);
    char *root_path = realpath(argv[1], NULL);
    if (!root_path) {
        perror("Could not obtain realpath ");
        exit(1);
    }
    setenv("MINIX_ROOT", root_path, 1);
    free(root_path);

    execvp(argv[2], argv + 2);
    perror("Exec failed");

    free(custom_preload);
}
