/**
 * vector
 * CS 341 - Spring 2023
 */
#include "sstring.h"

int main(int argc, char *argv[]) {
    // TODO create some tests
    

    const char * t = "This is a test string! LOL";
    //const char * u = "Another fking string.";
    sstring* strs = cstr_to_sstring(t);
    //sstring* stru = cstr_to_sstring(u);
    printf("%d\n", getSize(strs));
    // sstring_append(strs, stru);
    // char *first_s = sstring_to_cstr(strs);
    //printf("After append: %s\n", first_s);

}
