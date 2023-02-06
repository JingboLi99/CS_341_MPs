// /**
//  * vector
//  * CS 341 - Spring 2023
//  */
// #include "sstring.h"

// int main() {
//     // TODO create some tests
    
//     //1. Test sstring to string and string to ssttring
//     printf("***Test 1: Conversion\n");
//     const char * t = "12345678901234567891234567890";
//     sstring* strs = cstr_to_sstring(t);

//     char * ret_str = sstring_to_cstr(strs);
//     printf("Back to string: %s\n", ret_str);

//     free(ret_str);
//     sstring_destroy(strs);

//     //2. Test append:
//     printf("\n***Test 2: append\n");
//     const char * as = "......";
//     const char *p2 = "***";
//     sstring* og = cstr_to_sstring(as);
//     // //Testing to see if last element is \0
//     // vector * ogvec = getVec(og);
//     // printf("last ele: %d\n", *(char *) vector_get(ogvec, vector_size(ogvec) - 1));

//     sstring* ap = cstr_to_sstring(p2);
//     // printf("str 1 len: %d, str 2 len: %d\n", getSize(og), getSize(ap));
//     int stat = sstring_append(og, ap);
//     printf("Append status: %d\n", stat);

//     ret_str = sstring_to_cstr(og);
//     printf("Appended og string: %s\n", ret_str);

//     free(ret_str);
//     sstring_destroy(og);
//     sstring_destroy(ap);

//     //3. Test split: WRONG OUTPUT FOR CONSEC DELIMETER IN STRING
//     printf("\n***Part 3: String splitting\n");
//     char * toSplit = "This is a string..";
//     og = cstr_to_sstring(toSplit);
//     vector * split = sstring_split(og, '.');
//     size_t i = 0;
//     for (; i < vector_size(split); i++){
//         char * part = (char *) vector_get(split, i);
//         printf("Part %zu: %s\n", i, part);
//     }

//     vector_destroy(split);
//     sstring_destroy(og);

//     //4. Test substitution: ERROR: CANNOT REPLACE WITH LARGER STRING
//     printf("\n***Part 4: Substitution\n");
//     sstring *replace_me = cstr_to_sstring("Maybe the pointer involved had part of it's address overwritten?");
//     int ret = sstring_substitute(replace_me, 0, "Maybe the", " But someone else can probably interpret that error better than me.");
//     if (ret == 0){
//         char *out = sstring_to_cstr(replace_me);
//         printf("First replacement: %s\n", out);
//         free(out);
//     }else{
//         printf("Ret: %d\n", ret);
//     }
//     sstring_destroy(replace_me);

//     //5. Test slice:
//     printf("\n***Part 5: Slicing\n");
//     char * toSlice = "abcdefghi";
//     og = cstr_to_sstring(toSlice);
//     char * res = sstring_slice(og, 0, 7);
//     printf("Sliced string: %s\n", res);
//     sstring_destroy(og);
//     free(res);
// }


/**
 * vector
 * CS 341 - Spring 2023
 */

#include "sstring.h"
#include <string.h>

// int main(int argc, char *argv[]) {
int main() {
    // TODO create some tests
    printf("STARTING SSTRING TESTS...\n");

    //testing string creation - ss to cstr and cstr to ss
    char* input = "Hello World!";
    sstring* newString = cstr_to_sstring(input);
    char* output = sstring_to_cstr(newString); //THIS NEEDS TO BE FREED FOR NO MEM LEAK
    // printf("output: %s\n", output);
    printf("testing ss to cstr and cstr to ss: %d\n", 0 == strcmp(output, input));

    //sstring_append
    char* stringToAdd = " This is Fang Yi!";
    sstring* testAddition = cstr_to_sstring(stringToAdd);
    int totalLength = sstring_append(newString, testAddition);
    char* additionOutput = sstring_to_cstr(newString);
    char* correctAppend = "Hello World! This is Fang Yi!";
    printf("testing append: %d\n", 0 == strcmp(additionOutput, correctAppend));
    printf("checking returned totalLength from append: %d | totalLength = %d\n", totalLength == 28, totalLength);

    sstring_destroy(testAddition);
    free(additionOutput);
    
    sstring_destroy(newString);
    free(output);
    
    //sstring_split. this returns a vector
    char* splitInput = "a b c d e";
    
    //prepping the correct vector
    vector* correctVector = string_vector_create();
    vector_push_back(correctVector, "a");
    vector_push_back(correctVector, "b");
    vector_push_back(correctVector, "c");
    vector_push_back(correctVector, "d");
    vector_push_back(correctVector, "e");
    
    sstring* splitString = cstr_to_sstring(splitInput);
    vector* splitVector = sstring_split(splitString, ' ');
    size_t vectIndex = 0;
    printf("checking contents of vector...\n");
    while (vectIndex < vector_size(splitVector)) {
    char* currString = (char*)vector_get(splitVector, vectIndex);
    char* correctString = (char*)vector_get(correctVector, vectIndex);
    printf("validity of string at index: %zu is %d\n", vectIndex, 0 == strcmp(currString, correctString));
    
    vectIndex++;
    }
    
    vector_destroy(correctVector);
    sstring_destroy(splitString);
    vector_destroy(splitVector);
    
    //sstring slice
    sstring *slice_me = cstr_to_sstring("1234567890");
    char* sliced = sstring_slice(slice_me, 2, 5);
    printf("testing slice: %d\n", 0 == strcmp(sliced, "345"));
    
    sstring_destroy(slice_me);
    free(sliced);
    
    //sstring substitute
    
    sstring *replace_me = cstr_to_sstring("This is a {} day, {}!");
    sstring_substitute(replace_me, 18, "{}", "friend");
    char* subOne = sstring_to_cstr(replace_me); // == "This is a {} day, friend!"
    
    printf("subOne: %s\n", subOne);
    
    printf("testing subOne: %d\n", 0 == strcmp("This is a {} day, friend!", subOne));
    
    sstring_substitute(replace_me, 0, "{}", "good");
    char* subTwo = sstring_to_cstr(replace_me); // == "This is a good day, friend!"
    
    printf("subTwo: %s\n", subTwo);
    
    printf("testing subTwo: %d\n", 0 == strcmp("This is a good day, friend!", subTwo));
    
    sstring_destroy(replace_me);
    free(subOne);
    free(subTwo);
    
    //sstring_split with empty strings
    
    
    printf("PASSED ALL SSTRING TESTS.\n");
    return 0;
}