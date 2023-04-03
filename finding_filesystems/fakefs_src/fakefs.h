/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#ifdef FAKEFS
#else
#define FAKEFS
#include <sys/types.h>
typedef int (*access_fn)(const char *pathname, int mode);

typedef int (*creat_fn)(const char *pathname, mode_t mode);
typedef int (*open_fn)(const char *pathname, int flags, ...);
typedef ssize_t (*read_fn)(int fd, void *buf, size_t count);
typedef ssize_t (*write_fn)(int fd, const void *buf, size_t count);
typedef int (*close_fn)(int fd);
typedef off_t (*lseek_fn)(int fd, off_t offset, int whence);
typedef int (*unlink_fn)(const char *pathname);
typedef int (*mkdir_fn)(const char *pathname, mode_t mode);
typedef int (*fchmod_fn)(int fd, mode_t mode);
typedef int (*fchmodat_fn)(int dirfd, const char *pathname, mode_t mode,
                           int flags);

typedef int (*fchown_fn)(int fd, uid_t owner, gid_t group);
typedef int (*fchownat_fn)(int dirfd, const char *pathname, uid_t owner,
                           gid_t group, int flags);

typedef int (*stat_fn)(int mode, const char *pathname, struct stat *buf);
typedef int (*fstat_fn)(int mode, int fd, struct stat *buf);
typedef int (*fstatat_fn)(int mode, int dirfd, const char *pathname,
                          struct stat *buf, int flags);
typedef int (*stat64_fn)(int mode, const char *pathname, struct stat64 *buf);
typedef int (*fstat64_fn)(int mode, int fd, struct stat64 *buf);
typedef int (*fstatat64_fn)(int mode, int dirfd, const char *pathname,
                            struct stat64 *buf, int flags);

typedef int (*fsync_fn)(int oldfd);

typedef int (*dup_fn)(int oldfd);
typedef int (*dup2_fn)(int oldfd, int newfd);

typedef DIR *(*opendir_fn)(const char *name);
typedef DIR *(*fdopendir_fn)(int fd);
typedef struct dirent64 *(*readdir64_fn)(DIR *dirp);
typedef struct dirent *(*readdir_fn)(DIR *dirp);
typedef int (*readdir_r_fn)(DIR *dirp, struct dirent *entry,
                            struct dirent **result);
typedef int (*closedir_fn)(DIR *dirp);

typedef void *(*mmap_fn)(void *addr, size_t length, int prot, int flags, int fd,
                         off_t offset);

typedef int (*chdir_fn)(const char *path);

typedef struct {
    int fd;
    int flags;
    char *path;
    size_t refcount;
    long offset;
} fakefile;

typedef struct {
    int fd;
    int entries_read;
    int max_entries;
    struct dirent *entry;
} fakedir;

#define ORIG_FN(name) static name##_fn orig_##name;
ORIG_FN(access);

ORIG_FN(creat);
ORIG_FN(open);
ORIG_FN(read);
ORIG_FN(write);
ORIG_FN(close);
ORIG_FN(lseek);

ORIG_FN(unlink);
ORIG_FN(mkdir);
ORIG_FN(fchmod);
ORIG_FN(fchmodat);
ORIG_FN(fchown);
ORIG_FN(fchownat);

ORIG_FN(fsync);
static fsync_fn orig_fdatasync;

ORIG_FN(dup);
ORIG_FN(dup2);

ORIG_FN(stat);
static stat_fn orig_lstat;
ORIG_FN(fstat);
ORIG_FN(fstatat);
ORIG_FN(stat64);
static stat64_fn orig_lstat64;
ORIG_FN(fstat64);
ORIG_FN(fstatat64);

ORIG_FN(fdopendir);
ORIG_FN(opendir);
ORIG_FN(readdir64);
ORIG_FN(readdir);
ORIG_FN(readdir_r);
ORIG_FN(closedir);

ORIG_FN(mmap);

ORIG_FN(chdir);
#undef ORIG_FN
#endif
