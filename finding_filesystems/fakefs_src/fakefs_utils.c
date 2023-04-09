/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "dictionary.h"
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../minixfs.h"
#include "fakefs.h"
#include "fakefs_utils.h"
file_system *fs;
char *root_filename = NULL;
char *root_filepath = NULL;
int disable_hooks = 1;
dictionary *fd_map;
dictionary *dir_fd_map;
regex_t root_match;
int regex_init = 0;
int enable_logging = 0;

void free_cleanup(void **free_me) {
    free(*((void **)*free_me));
}

char *load_LD(char *so_name) {
    char *preload = getenv("LD_PRELOAD");

    char *custom_preload = NULL;
    char *real_path = realpath(so_name, NULL);
    if (preload) {
        asprintf(&custom_preload, "%s:%s", real_path, preload);
        free(real_path);
    } else
        custom_preload = real_path;

    int err = setenv("LD_PRELOAD", custom_preload, 1);
    if (err) {
        fakefs_log("Unable to set environment\n");
        exit(EXIT_FAILURE);
    }
    return custom_preload;
}

fakefs_path_t fakefs_realpath(const char *pathname) {
    if (!regex_init) {
        char *root_reg = NULL;
        auto_cleanup(root_reg);

        // TODO define root_filename vs root_filepath
        asprintf(&root_reg, "\\<%s\\>", root_filename);
        int error = regcomp(&root_match, root_reg, 0);

        if (error)
            exit(error);

        regex_init = 1;
    }
    char *resolved_path = realpath(pathname, NULL);
    if (resolved_path) {
        if (!strcmp(resolved_path, root_filepath)) {
            return (fakefs_path_t){resolved_path, strdup("/")};
        }
        return (fakefs_path_t){resolved_path, NULL};
    }

    // Need to test if this is a virtual file
    int regex_err = regexec(&root_match, pathname, 0, NULL, 0);
    if (regex_err)
        return fakefs_path_NULL;

    // We know that there is a directory with the same name as the root filename
    // somewhere in pathname, we're going to pop off directories until we have a
    // path on the real fs, make sure that path corresponds to the rootfs, then
    // add the remainder of the path to the fs, resolving .. and . on the way.
    char *my_pathname = strdup(pathname);
    auto_cleanup(my_pathname);

    size_t last_idx = strlen(pathname) - 1;
    int failure = 0;
    while (!(resolved_path = realpath(my_pathname, NULL))) {
        if (my_pathname[last_idx] == 0) {
            my_pathname[last_idx] = '/';
            last_idx--;
        }
        while (my_pathname[last_idx] != '/') {
            last_idx--;
            if (last_idx == 0) {
                failure = 1;
                break;
            }
        }
        if (failure)
            break;

        my_pathname[last_idx] = 0;
        fakefs_log("LOOKING FOR RESOLVEABLE PATH %s\n", my_pathname);
    }

    if (failure || !resolved_path)
        return fakefs_path_NULL;

    fakefs_log("FOUND RESOLVEABLE PATH %s\n", resolved_path);
    if (strcmp(resolved_path, root_filepath)) {
        // The root_filepath was not in the resolved path. This implies that the
        // original path never existed.
        free(resolved_path);
        return fakefs_path_NULL;
    }

    // Now we know that the path is a path into the virtual filesystem, time to
    // reconstruct the virtual path.
    char *orig_v_path = my_pathname + last_idx + 1;
    char *v_path = calloc(1, strlen(orig_v_path) + 2);
    v_path[0] = '/';
    size_t curr_idx = 0, v_idx = 1;
    int at_dir_boundry = 1;

    while (curr_idx < strlen(orig_v_path) && orig_v_path[curr_idx]) {
        // If we're processing a new directory name, check for the names '.' and
        // '..'
        if (at_dir_boundry) {
// Hacky way of defining a macro locally
// TODO make this a function or something
#define char_is_end(c) (c == '/' || c == 0)
            at_dir_boundry = 0;
            if (orig_v_path[curr_idx] == '.' &&
                char_is_end(orig_v_path[curr_idx + 1])) {
                // curr dir
                curr_idx += 2;
                at_dir_boundry = 1;
                continue;
            } else if (orig_v_path[curr_idx] == '.' &&
                       orig_v_path[curr_idx + 1] == '.' &&
                       char_is_end(orig_v_path[curr_idx + 2])) {
                // parent dir
                if (v_idx >= 2) {
                    v_idx -= 2;
                    while (v_idx && v_path[v_idx] != '/') {
                        v_path[v_idx] = 0;
                        v_idx--;
                    }
                } else {
                    if (strcmp("/", resolved_path)) {
                        size_t i = 0;
                        for (i = strlen(resolved_path) - 1;
                             i && resolved_path[i] != '/'; i--)
                            ;
                        if (i)
                            resolved_path[i] = 0;
                    }
                }
                if (v_path[v_idx] == '/')
                    v_idx++;

                curr_idx += 3;
                at_dir_boundry = 1;
                continue;
            }
#undef char_is_end
        }

        if (orig_v_path[curr_idx] == '/') {
            at_dir_boundry = 1;
            if (v_idx && v_path[v_idx - 1] == '/')
                v_idx -= 1;
        }
        v_path[v_idx] = orig_v_path[curr_idx];
        curr_idx++;
        v_idx++;
    }

    if (!strlen(v_path)) {
        free(v_path);
        v_path = strdup("/");
    }

    if (strlen(v_path) > 1 && v_path[strlen(v_path) - 1] == '/')
        v_path[strlen(v_path) - 1] = 0;

    // Trim excess memory
    resolved_path = realloc(resolved_path, strlen(resolved_path) + 1);
    v_path = realloc(v_path, strlen(v_path) + 1);

    return (fakefs_path_t){resolved_path, v_path};
}

void destroy_fakefile(fakefile *f) {
    free(f->path);
    free(f);
}

void destroy_fakedir(fakedir *d) {
    free(d->entry);
    free(d);
}
