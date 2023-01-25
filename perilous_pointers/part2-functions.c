/**
 * perilous_pointers
 * CS 341 - Spring 2023
 */
#include "part2-functions.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

void first_step(int value) {
    if (value == 81) {
        printf("1: Illinois\n");
    } else {
        printf("first_step failed\n");
    }
}

void second_step(int *value) {
    if (!value) {
        printf("second_step failed\n");
    } else if (*value == 132) {
        printf("2: Illinois\n");
    } else {
        printf("second_step failed\n");
    }
}

void double_step(int **value) {
    if (!value || !value[0]) {
        printf("double_step failed\n");
    } else if (*value[0] == 8942) {
        printf("3: Illinois\n");
    } else {
        printf("double_step failed\n");
    }
}

void strange_step(char *value) {
    if (!value) {
        printf("strange_step failed\n");
    } else if (*(int *)(value + 5) == 15) {
        printf("4: Illinois\n");
    } else {
        printf("strange_step failed\n");
    }
}

void empty_step(void *value) {
    if (!value) {
        printf("empty_step failed\n");
    } else if (((char *)value)[3] == 0) {
        printf("5: Illinois\n");
    } else {
        printf("empty_step failed\n");
    }
}

void two_step(void *s, char *s2) {
    if (!s2) {
        printf("two_step failed\n");
    } else if (s == s2 && s2[3] == 'u') {
        printf("6: Illinois\n");
    } else {
        printf("two_step failed\n");
    }
}

void three_step(char *first, char *second, char *third) {
    if (!first || !second || !third) {
        printf("three_step failed\n");
    } else if (third == second + 2 && second == first + 2) {
        printf("7: Illinois\n");
    } else {
        printf("three_step failed\n");
    }
}

void step_step_step(char *first, char *second, char *third) {
    if (!first || !second || !third) {
        printf("step_step_step failed\n");
    } else if (third[3] == second[2] + 8 && second[2] == first[1] + 8) {
        printf("8: Illinois\n");
    } else {
        printf("step_step_step failed\n");
    }
}

void it_may_be_odd(char *a, int b) {
    if (!a) {
        printf("it_may_be_odd failed\n");
    } else if (*a == b && b > 0) {
        printf("9: Illinois\n");
    } else {
        printf("it_may_be_odd failed\n");
    }
}

void tok_step(char *str) {
    if (!str) {
        printf("tok_step failed\n");
        return;
    }
    char *a = strtok(str, ",");
    a = strtok(NULL, ",");
    if (strcmp(a, "CS241") == 0) {
        printf("10: Illinois\n");
    } else {
        printf("tok_step failed\n");
    }
}

void the_end(void *orange, void *blue) {
    if (!orange || !blue) {
        printf("the_end failed\n");
    } else if (orange != NULL && orange == blue && ((char *)blue)[0] == 1 &&
               *((int *)orange) % 3 == 0) {
        printf("11: Illinois\n");
    } else {
        printf("the_end failed\n");
    }
}
