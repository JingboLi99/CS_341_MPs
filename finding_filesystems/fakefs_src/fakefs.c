/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#include "../minixfs.h"
#include "../minixfs_utils.h"
#include "dictionary.h"
#include "vector.h"

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fakefs.h"
#include "fakefs_utils.h"

/**
 * Ever seen this trick before? This file simply includes .c files. This is done
 * purely for organizational convinience.
 *
 * Essentially, #include tells the
 * compiler to go paste the contents of that file here. That's why you normally
 * don't put implementations in headers since that would cause a function to be
 * redefined.
 *
 * By including .c files we can break up one large file into a bunch of smaller
 * files. This can be a neat trick that cleans up your project, but be careful
 * with it! If you're writing a reusable component, it might be a better idea to
 * just have a header file and link in the implementation after it's been
 * compiled to an object.
 */

// Main function to run fakefs as an executable
#include "main.c"

// Shared object initialization code
#include "init.c"

// Include source of glibc wrappers
#include "core.c"
#include "cwd_mgmt.c"
#include "dir.c"
#include "dup.c"
#include "stat.c"
#include "unimplemented.c"
