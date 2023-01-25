/**
 * perilous_pointers
 * CS 341 - Spring 2023
 */
#pragma once

void one(const char *grade);
void two();
void three(const int *x, const int *y);
float *four(const int *x);
void five(const char *a);
void six(const char *s);
void seven();
void eight(int a);
void nine(const char *s);
void ten(const int d);
void clear_bits(long int value, long int flag);
void little_automaton(int (*transition_function)(int, char), const char *input);
