/**
 * vector
 * CS 341 - Spring 2023
 */
#include "sstring.h"

int main() {
    // TODO create some tests
    
    // //1. Test sstring to string and string to ssttring
    // printf("***Test 1: Conversion\n");
    // const char * t = "12345678901234567891234567890";
    // sstring* strs = cstr_to_sstring(t);

    // char * ret_str = sstring_to_cstr(strs);
    // printf("Back to string: %s\n", ret_str);

    // free(ret_str);
    // sstring_destroy(strs);

    // //2. Test append:
    // printf("\n***Test 2: append\n");
    // const char * as = "......";
    // const char *p2 = "***";
    // sstring* og = cstr_to_sstring(as);
    // //Testing to see if last element is \0
    // vector * ogvec = getVec(og);
    // printf("last ele: %d\n", *(char *) vector_get(ogvec, vector_size(ogvec) - 1));

    // sstring* ap = cstr_to_sstring(p2);
    // printf("str 1 len: %d, str 2 len: %d\n", getSize(og), getSize(ap));
    // int stat = sstring_append(og, ap);
    // printf("Append status: %d\n", stat);

    // ret_str = sstring_to_cstr(og);
    // printf("Appended og string: %s\n", ret_str);

    // free(ret_str);
    // sstring_destroy(og);
    // sstring_destroy(ap);

    //3. Test split: WRONG OUTPUT FOR CONSEC DELIMETER IN STRING
    printf("\n***Part 3: String splitting\n");
    char * toSplit = "This is a string..";
    sstring * og = cstr_to_sstring(toSplit);
    vector * split = sstring_split(og, '.');
    size_t i = 0;
    for (; i < vector_size(split); i++){
        char * part = (char *) vector_get(split, i);
        printf("Part %zu: %s\n", i, part);
    }

    vector_destroy(split);
    sstring_destroy(og);

    // //4. Test substitution: ERROR: CANNOT REPLACE WITH LARGER STRING
    // printf("\n***Part 4: Substitution\n");
    // sstring *replace_me = cstr_to_sstring("Maybe the pointer involved had part of it's address overwritten?");
    // int ret = sstring_substitute(replace_me, 0, "Maybe the", " But someone else can probably interpret that error better than me.");
    // if (ret == 0){
    //     char *out = sstring_to_cstr(replace_me);
    //     printf("First replacement: %s\n", out);
    //     free(out);
    // }else{
    //     printf("Ret: %d\n", ret);
    // }
    // sstring_destroy(replace_me);

    // //5. Test slice:
    // printf("\n***Part 5: Slicing\n");
    // char * toSlice = "abcdefghi";
    // og = cstr_to_sstring(toSlice);
    // char * res = sstring_slice(og, 0, 7);
    // printf("Sliced string: %s\n", res);
    // sstring_destroy(og);
    // free(res);
}
