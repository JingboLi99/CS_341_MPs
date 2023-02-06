// /**
//  * vector
//  * CS 341 - Spring 2023
//  */
// #include "vector.h"
// #include <stdio.h>
// // #include <assert.h>
// // int main(int argc, char *argv[]) {

// int main() {
//     // Write your test cases here
//     printf("starting unit tests\n");

//     //1. Test for shallow copies:

//     //2. Test resize
//     // vector* vector = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    
//     // size_t newCapacity = 22;
//     // vector_resize(vector, newCapacity);
//     // printf("new size: %zu\n", vector_size(vector));
//     // if (vector_size(vector) == newCapacity) {
//     //     printf("passed test 1\n");
//     // } else {
//     //     printf("failed test 1\n");
//     // }

//     // vector_destroy(vector);
    

//     //3. Test Pushback, pop back,
//     vector *vec3 = vector_create(int_copy_constructor, int_destructor, int_default_constructor);
//     vector_reserve(vec3, 200);
//     printf("Vec size after resize: %zu, %zu\n", vector_size(vec3), vector_capacity(vec3));
//     int a = 0;
//     for (; a < 1000000;a++){
//         vector_push_back(vec3, &a);
//     }
//     printf("Vec size after push back: %zu, %zu\n", vector_size(vec3), vector_capacity(vec3));
    
//     int * front =(int *) vector_front(vec3);
//     int * back =(int *) vector_back(vec3);
//     int *front2 = (int *) vector_get(vec3, 0);
//     printf("front and back: %d, %d, %d\n", *front, *front2, *back);
    
//     // test clear:
//     // vector_clear(vec3);
//     // printf("Size after clear: %zu\n", vector_size(vec3));
//     //test vector erase for last element:
//     vector_erase(vec3, vector_size(vec3) - 1);
//     vector_erase(vec3, vector_size(vec3) - 1);
//     vector_erase(vec3, vector_size(vec3) - 1);
//     printf("Size after erase: %zu\n", vector_size(vec3));
//     vector_destroy(vec3); //WARNING: Losing 2 bytes here
//     //4. Test insert, erase
// }    



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

    //tests for a null vector

    vector* vectorNULL = vector_create(NULL, NULL, NULL);
    
    size_t newCapacity = 8;
    
    vector_resize(vectorNULL, newCapacity);
    // printf("new size: %zu\n", vector_size(vectorNULL));
    if (vector_size(vectorNULL) == newCapacity) {
        printf("passed test 1\n");
    } else {
        printf("failed test 1\n");
    }

    // vector_push_back(vectorNULL, NULL);

    vector_destroy(vectorNULL); //valgrind shows that everything is destroyed.

    //tests for vectors with a defined data type
    //char vector
    //char test create
    vector* charVect = vector_create(char_copy_constructor, char_destructor, char_default_constructor);
    
    char charA = 'a';
    vector_push_back(charVect, &charA);
    printf("test vector_get: %d\n", 'a'== *(char*)vector_get(charVect, 0));
    //resize upwards and downwards
    //resize upwards without changing capacity
    vector_resize(charVect, 8);
    printf("test vector_resize size: %d\n", vector_size(charVect)==8);
    printf("test vector_resize capacity: %d\n", vector_capacity(charVect)==8);

    //resize upwards with changing capacity
    vector_resize(charVect, 9);
    printf("test vector_resize size: %d\n", vector_size(charVect)==9);
    printf("test vector_resize capacity: %d\n", vector_capacity(charVect)==16);

    //resize downwards
    vector_resize(charVect, 1);
    printf("test vector_resize: %d\n", vector_size(charVect)==1);
    printf("test vector_get after resize: %d\n", 'a'== *(char*)vector_get(charVect, 0));
    //just trying to push back until we need a resize
    int count = 0;
    while (count < 8) {
        vector_push_back(charVect, &charA);
        count++;
    }
    printf("testing vector_push_back...\n");
    size_t vectIndex = 0;
    for (; vectIndex < vector_size(charVect) - 1; vectIndex++) {
        // printf("vect index: %zu\n", vectIndex);
        printf("checking for each element: %d\n", 'a' == *(char*)vector_get(charVect, vectIndex));
    }
    char charB = 'b';
    vector_set(charVect, 0, &charB);
    printf("test vector_get: %d\n", 'b'== *(char*)vector_get(charVect, 0));

    //testing vector_front
    printf("test vector_front: %d\n", 'b'== **(char**)vector_front(charVect));

    //testing vector_back
    char charC = 'c';
    vector_set(charVect, vector_size(charVect)-1, &charC);
    printf("test vector_back: %d\n", 'c'== **(char**)vector_back(charVect));

    //testing vector_empty with full vect
    vector_empty(charVect);
    printf("test vector_set non-empty vect: %d\n", !vector_empty(charVect)); //this should be false since we havent cleared it so we invert the false

    //testing vector_empty will empty vect
    vector_resize(charVect, 0);
    printf("test vector_empty empty vect: %d\n", vector_empty(charVect));

    //testing vector_clear
    vector_resize(charVect, 6);
    vector_clear(charVect);
    printf("test vector_clear: %d\n", vector_empty(charVect));

    //testing vector_pop_back
    vector_resize(charVect, 10); //this resize is setting the stage for the error later
    
    size_t sizeBeforePop = vector_size(charVect);
    printf("size before pop: %zu\n", sizeBeforePop);
    vector_pop_back(charVect);
    printf("test vector_pop: %d\n", sizeBeforePop-1 == vector_size(charVect));

    char toInsert = 'm';
    size_t sizeBeforeInsert = vector_size(charVect);
    //testing vector_insert once, current size is 9
    vector_insert(charVect, 5, &toInsert);
    //check size is 10
    printf("test vector_insert size: %d\n", sizeBeforeInsert+1 == vector_size(charVect));
    //check element is at correct place.
    printf("test vector_insert get: %d\n", 'm' == *(char*)vector_get(charVect, 5));

    //testing multiple inserts (this this the one that failed the autograder)

    //testing resize and then insert (this is the one that failed the autograder)
    vector_destroy(charVect);
    printf("PASSED charVect TESTS\n");



    //string vector
    vector* stringVect = vector_create(string_copy_constructor, string_destructor, string_default_constructor);

    vector_destroy(stringVect);

    //double vector
    vector* doubleVect = vector_create(double_copy_constructor, double_destructor, double_default_constructor);

    vector_destroy(doubleVect);

    //float vector
    vector* floatVect = vector_create(float_copy_constructor, float_destructor, float_default_constructor);

    vector_destroy(floatVect);

    //int vector
    vector* intVect = vector_create(int_copy_constructor, int_destructor, int_default_constructor);

    vector_destroy(intVect);

    //long vector
    vector* longVect = vector_create(long_copy_constructor, long_destructor, long_default_constructor);

    vector_destroy(longVect);

    //short vector
    vector* shortVect = vector_create(short_copy_constructor, short_destructor, short_default_constructor);

    vector_destroy(shortVect);

    //i think thats enough honestly. just need to make sure that it can handle different sizes of shit, not that i am even directly interfacing with the sizes.

    //testing:
}