/**
 * vector
 * CS 341 - Spring 2023
 */
/**
 * Vector Lab
 * CS 241 - Spring 2019
 */

#include "vector.h"
#pragma once
/**
 * C is notorious for having relatively annoying string manipulation interfaces
 * as compared to other languages such as C++ and python. In this MP, we're
 * going to define and implement sstring, a wrapper around C strings that
 * provides a higher level interface to dealing with strings. You will actually
 * define the implementation of `sstring` in sstring.c. Since the struct
 * definition is only present in the `.c` file, that definition is essentially
 * 'private' and as a result, outside of the implementing `.c` file, the
 * internal members of sstring cannot be accessed. As a consequence, an
 * `sstring` can only be declared on the heap (at least, not without some
 * non-trivial refactoring).
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward declare the structure */
typedef struct sstring sstring;

/**
 * This function should take in a C-string, and return a pointer to an sstring,
 * allocating any memory required to create the sstring on the heap.
 */
sstring *cstr_to_sstring(const char *input);

/**
 * This function returns a C-string from an sstring. This function will allocate
 * the memory to
 * store the returned C-string on the heap.
 */
char *sstring_to_cstr(sstring *this);

/**
 * This function takes in two sstrings, appends the second to the first, and
 * returns the length of the first sstring after the append.
 * to the first.
 * Example:
 *
 * sstring *str1 = cstr_to_sstring("abc");
 * sstring *str2 = sstring_to_cstr("def");
 * int len = sstring_append(str1, str2); // len == 6
 * sstring_to_cstr(str1); // == "abcdef"
 */
int sstring_append(sstring *this, sstring *addition);

/**
 * Takes in an sstring and a character (the delimiter), and splits the sstring
 * into a vector of C-strings on the given delimiter.
 * This should be analogous to python3's split function on
 * strings. You can check what the output should be for a given source string,
 * INPUT and a delimiter D, by running `python3 -c 'print("INPUT".split('D'))'`
 * in a shell.
 *
 * Example:
 * sstring_split(cstr_to_sstring("abcdeefg"), 'e'); // == [ "abcd", "", "fg" ]);
 * sstring_split(cstr_to_sstring("This is a sentence."), ' ');
 * // == [ "This", "is", "a", "sentence." ]
 */
vector *sstring_split(sstring *this, char delimiter);

/**
 * This function should substitute one occurrence of target in this after offset
 * bytes with
 * substitution. If there are no occurrences of target after offset bytes,
 * return -1.
 * Otherwise, return 0.
 *
 * You can assume that `offset` is always less than the length of the data in
 * `this`.
 *
 * Example:
 *
 * sstring *replace_me = cstr_to_sstring("This is a {} day, {}!");
 * sstring_substitute(replace_me, 18, "{}", "friend");
 * sstring_to_cstr(replace_me); // == "This is a {} day, friend!"
 * sstring_substitute(replace_me, 0, "{}", "good");
 * sstring_to_cstr(replace_me); // == "This is a good day, friend!"
 */
int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution);

/**
 * This function takes in an sstring, a start index and end index.
 * It should return a C-string representing the bytes between start (inclusive)
 * and end (exclusive).
 * You can assume that start and end are always positive and satisfy the
 * following rule:
 *     0 <= start <= end <= number of bytes held by `this`
 *
 *
 * Example:
 * sstring *slice_me = cstr_to_sstring("1234567890");
 * sstring_slice(slice_me, 2, 5);
 * // == "345" (don't forget about the null byte!)
 */
char *sstring_slice(sstring *this, int start, int end);

/**
 * Destroys this sstring object, freeing all of its dynamic memory. `this`
 * shall be a valid sstring allocated on the heap.
 */
void sstring_destroy(sstring *this);
