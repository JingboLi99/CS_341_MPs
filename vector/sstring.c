/**
 * vector
 * CS 341 - Spring 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    vector * svec;
    size_t size; // size does not include ending char
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    
    //initialize sstring:
    struct sstring *ss = malloc(sizeof(struct sstring));
    ss->svec = vector_create(char_copy_constructor, char_destructor, char_default_constructor);
    ss->size = 0;
    //copy over items in input to sstring vector
    int i = 0;
    int slen = strlen(input);
    // to prevent affecting the const char
    char * in2 = NULL;
    in2 = malloc(slen);
    strcpy(in2, input);
    printf("Copied in string: %s\n", in2);
    for (; i < slen; i++){
        void * toAdd = &in2[i];
        vector_push_back(ss->svec, toAdd); //WARNING: toAdd is treated as a string, may have problems, depending on constructor
        ss->size ++;
    }
    
    free(in2); // free the temp copy of input
    //add ending null character:
    char nul = '\0';
    void * nulptr = &nul;
    
    vector_push_back(ss->svec, nulptr);
    
    return ss;
}

int getSize(sstring * in){
    return in->size;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    //size of input sstring:
    int in_size = input->size;
    char * out = malloc(in_size); //null char is already inside vector
    // char * out = malloc(in_size+1); //+1 for ending char
    // out[in_size] = '\0'; //assign null character
    int i = 0;
    for (;i < in_size; i++){
        out[i] =  *((char*) vector_get(input->svec, i));
    }
    return out;

}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    int add_len = addition->size;
    int i = 0;
    //remove the null character at end of this vector:
    vector_pop_back(this->svec);
    this->size --;

    for (;i < add_len; i++){
        vector_push_back(this->svec, vector_get(addition->svec, i));
        this->size ++;
    }
    return this->size;

}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    char *inStr = sstring_to_cstr(this); //need to free
    char sArr[strlen(inStr)];
    strcpy(sArr, inStr);
    //initialize output vector:
    vector *vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    //get tokens:
    char *tok = strtok(sArr, &delimiter);
    
    while (tok){
        vector_push_back(vec, (void *) tok);
        tok = strtok(NULL, &delimiter);
    }
    free(inStr);
    return vec;
}
//return
char* getSubString(char *in, int s, int len){
    int i = 0;
    char *dest = malloc(len+1);
    dest[len] = '\0';
    for (;i < len; i++){
        dest[i] = in[i+s];
    }
    return dest;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    //input as string
    char * inStr = sstring_to_cstr(this); //need to free
    int inLen = strlen(inStr); 
    
    int tar_len = strlen(target);// length of target
    // starts from offset
    int i = offset;
    for (; i < inLen; i++){
        // check that target can exist after offset based on length:
        if (tar_len + i > inLen){
            free(inStr);
            return -1;
        }
        // get substring from curr index:
        char * substring = getSubString(inStr, i, tar_len); //Need to Free
        free(inStr);
        //if there is a match:
        if (strcmp(substring, target) == 0){
            
            //erase the target from vector: 
            int j = 0;
            for (;j < tar_len; j++){
                vector_erase(this->svec, i);
                this->size--;
            }
            //add substitute to vector:
            int subLen = strlen(substitution);
            char * subCpy = malloc(subLen); //Need to Free
            strcpy(subCpy, substitution);
            int k = 0;
            for (;k < subLen; k++){
                void * cToAdd = &subCpy[k];
                vector_insert(this->svec, i+k, cToAdd);
                this->size++;
            }
            free(substring);
            free(subCpy);
            return 0;
        }
        free(substring);
    }
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char * inStr = sstring_to_cstr(this); //Need to free
    char * subStr = getSubString(inStr, start, end - start);
    free(inStr);
    return subStr;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->svec);
    free(this);
}
