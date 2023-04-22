/**
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int _log(int a, int b) {
    int res = 0;
    while (a) {
        a /= b;
        res++;
    }
    return res;
}
int int_write(int fd, int arg) {
    int len = _log(arg, 10);
    char buffer[len + 1];
    buffer[len] = 0;
    for (int i = len - 1; i >= 0; i--) {
        buffer[i] = '0' + arg % 10;
        arg /= 10;
    }
    write(fd, buffer, strlen(buffer));
    return strlen(buffer);
}

int double_write(int fd, double arg) {
    int int_part = (int)arg;
    int written = int_write(fd, int_part);
    double remainder = arg - int_part;
    if (remainder > 0) {
        int decimals = (int)(remainder * 1000);
        if (written == 0) {
            write(fd, "0.", 2);
            written += 2;
        } else {
            write(fd, ".", 1);
            written += 1;
        }
        int bottom_half = int_write(fd, decimals);
        return written + bottom_half;
    }
    return written;
}
static int uint_write(int fd, uintptr_t ptr) {
    int len = 2 * _log(ptr, 16);
    if (len == 0) {
        write(fd, "NULL", 4);
        return 4;
    }

    char buff[len + 1];
    buff[len] = 0;
    for (int i = 0; i < len; i++) {
        char c;
        if (ptr % 16 < 10) {
            c = '0' + (ptr % 16);
        } else {
            c = 'a' + (ptr % 16) - 10;
        }
        buff[len - i - 1] = c;
        ptr /= 16;
    }
    write(fd, buff, strlen(buff));
    return strlen(buff);
}
int ptr_write(int fd, void *arg) {
    if (arg == NULL) {
        write(fd, "N", 1);
        return 1;
    }
    uintptr_t ptr = (uintptr_t)arg;
    write(fd, "0x", 2);
    return 2 + uint_write(fd, ptr);
}
