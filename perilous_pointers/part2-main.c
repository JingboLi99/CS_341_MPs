/**
 * perilous_pointers
 * CS 341 - Spring 2023
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    int num = 132;
    int *val = &num;
    second_step(val);

    num = 8942;
    int *dsval = &num;
    int **dsaval = &dsval;
    double_step(dsaval);

    // strange step:
    char isarr[4] = {15,0,0,0};
    char *addy = isarr-5;
    //^ when typecasting into int pointer (int *) and then
    // dereferencing (*(int *)), we are now reading 4 bytes instead
    // of the usual 1 byte when dereferencing a char *.
    strange_step(addy);
    
    //empty step:
    char charls[5];
    empty_step(charls);
    
    // two_step:
    char *s2 = "uuuuu";
    char *s = s2;
    two_step(s, s2);
    
    //three steps:
    char *third = "bob";
    char *second = third-2;
    char *first = second-2;
    three_step(first, second, third);

    //stepstepstep:
    char *sthird = "zzzzzz";
    char *ssecond = "rrr";
    char *sfirst = "jj";
    step_step_step(sfirst, ssecond, sthird);

    //it might be odd:
    char *isa = "AA";
    int isb = 65;
    it_may_be_odd(isa, isb);

    //tok_step:
    char tok[] = "random,CS241,random";
    //^ We cannot use char *tok because it is a string literal
    // and cannot be modified (is const char *), instead,
    // we have to use a char array char tok[]
    tok_step(tok);

    //the end:
    //NOTE: void* pointers are pointers without specific cast
    char blue[4] = {1,2,3,3};
    char *orange = blue;
    the_end(orange, blue);
    return 0;
}
