/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#ifdef FAKEFS_UTILS
#else
#define FAKEFS_UTILS
#include "dictionary.h"

extern file_system *fs;
extern char *root_filename;
extern char *root_filepath;
extern int disable_hooks;
extern dictionary *fd_map;
extern dictionary *dir_fd_map;
extern regex_t root_match;
extern int regex_init;

extern int enable_logging;

typedef struct {
    /**
     * For a normal path, base_path represents the full path
     * For a path inside the virtual filesystem, base_path will be the path of
     * the root file
     */
    char *base_path;
    /**
     * For virtual paths, virtual_path will be the path relative to the root of
     * the virtual filesystem
     */
    char *virtual_path;
} fakefs_path_t;

/**
 * Set up LD_PRELOAD
 */
char *load_LD(char *so_name);

/**
 * Translate pathname into a fakefs_path_t as described above
 */
fakefs_path_t fakefs_realpath(const char *pathname);

/**
 * Destructor methods
 */
void destroy_fakefile(fakefile *f);
void destroy_fakedir(fakedir *d);

#define fakefs_path_NULL \
    (fakefs_path_t) {    \
        NULL, NULL       \
    }

void free_cleanup(void **free_me);

#define _TOKENPASTE(x, y) x##y
#define TOKENPASTE(x, y) _TOKENPASTE(x, y)
#define auto_cleanup(var)                          \
    void *TOKENPASTE(__auto_cleanup_, __COUNTER__) \
        __attribute__((unused, cleanup(free_cleanup))) = &(var);

#define auto_cleanup_path(paths)   \
    auto_cleanup(paths.base_path); \
    auto_cleanup(paths.virtual_path);

#define HOOK(fname) orig_##fname = dlsym(RTLD_NEXT, #fname)
#define HOOK2(fname, realname) orig_##fname = dlsym(RTLD_NEXT, #realname)

#define FD_UNIMPLEMENTED(fd, error)                           \
    if (!disable_hooks && dictionary_contains(fd_map, &fd)) { \
        return error;                                         \
    }

#define PATH_UNIMPLEMENTED(path, error)                                 \
    {                                                                   \
        fakefs_path_t PATH_UNIMPLEMENTED_paths = fakefs_realpath(path); \
        auto_cleanup_paths(PATH_UNIMPLEMENTED_paths);                   \
        if (PATH_UNIMPLEMENTED_paths.base_path &&                       \
            PATH_UNIMPLEMENTED_paths.virtual_path) {                    \
            return error;                                               \
        }                                                               \
    }

#define fakefs_log(args...)        \
    {                              \
        if (enable_logging)        \
            fprintf(stderr, args); \
    }
#endif
