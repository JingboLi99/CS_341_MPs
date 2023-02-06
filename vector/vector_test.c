/**
 * vector
 * CS 341 - Spring 2023
 */
#include "vector.h"
#include <stdio.h>
// #include <assert.h>
// int main(int argc, char *argv[]) {

int main() {
    // Write your test cases here
    printf("starting unit tests\n");

    //1. Test for shallow copies:

    //2. Test resize
    // vector* vector = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    
    // size_t newCapacity = 22;
    // vector_resize(vector, newCapacity);
    // printf("new size: %zu\n", vector_size(vector));
    // if (vector_size(vector) == newCapacity) {
    //     printf("passed test 1\n");
    // } else {
    //     printf("failed test 1\n");
    // }

    // vector_destroy(vector);
    

    //3. Test Pushback, pop back, 
    vector *vec3 = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    vector_reserve(vec3, 200);
    printf("Vec size after resize: %zu, %zu\n", vector_size(vec3), vector_capacity(vec3));
    int a = 0;
    for (; a < 20;){
        vector_push_back(vec3, &a);
        a+=2;
    }
    printf("Vec size after push back: %zu, %zu\n", vector_size(vec3), vector_capacity(vec3));
    
    int i = 0;
    for (;i < 7; i++){
        printf("Item %d is: %d\n",i, *((int *) vector_get(vec3, i)));
    }
    // int new_first = 500000;
    // vector_set(vec3, 0, &new_first);
    // vector_pop_back(vec3);
    // i = 0;
    // for (;i < 10; i++){
    //     printf("Item %d is: %d\n",i, *((int *) vector_get(vec3, i)));
    // }
    // printf("Vec size after set and popback: %zu\n", vector_size(vec3));


    vector_destroy(vec3); //WARNING: Losing 2 bytes here

    //4. Test insert, erase
}    