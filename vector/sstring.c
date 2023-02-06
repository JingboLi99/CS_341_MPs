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

// ADDITIONAL HELPER FUNCTIONS:

char* getSubString(char *in, int s, int len){
    int i = 0;
    char *dest = malloc(len+1);
    dest[len] = '\0';
    for (;i < len; i++){
        dest[i] = in[i+s];
    }
    return dest;
}

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
    char * in2 = malloc(slen+1); // account for null charater
    strcpy(in2, input);
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

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    //size of input sstring:
    int in_size = input->size; //size does not account for null char
    
    char * out = malloc(in_size+1); //+1 for ending char
    out[in_size] = '\0'; //assign null character
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

    for (;i < add_len; i++){
        vector_push_back(this->svec, vector_get(addition->svec, i));
        this->size ++;
    }
    return this->size;

}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    char *inStr = sstring_to_cstr(this); //need to free
    int inlen = strlen(inStr);

    //initialize output vector:
    vector *vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    
    char * curSen = malloc(inlen);
    int curIdx = 0; // index for curSen
    int in = 0;
    char cur_char = '\0';
    char * proper = NULL;

    for (;in < inlen; in ++){
        cur_char = inStr[in];
        if (cur_char == delimiter){
            
            proper = NULL;
            if (curIdx == 0){
                //printf("Curr empty sentence: %s\n", curSen);
                proper = malloc(1);
                strcpy(proper, "");
            }else{
                //printf("Curr sentence: %s\n", curSen);
                curSen[curIdx] = '\0';
                int curSenLen = strlen(curSen);
                proper = malloc(curSenLen+1);
                proper[curSenLen] = '\0';
                int j = 0;
                for (;j < curSenLen; j++){
                    proper[j] = curSen[j];
                }
            }
            vector_push_back(vec, proper);
            free(curSen);
            free(proper);
            curSen = NULL;
            curSen = malloc(inlen);
            curIdx = 0;
            continue;
        }
        curSen[curIdx] = cur_char;
        curIdx++;
        
    }
    if (curIdx > 0){
        curSen[curIdx] = '\0';
        int curSenLen = strlen(curSen);
        proper = malloc(curSenLen+1);
        proper[curSenLen] = '\0';
        int j = 0;
        for (;j < curSenLen; j++){
            proper[j] = curSen[j];
        }
        printf("Proper: %s\n", proper);
        vector_push_back(vec, proper);
        free(curSen);
        free(proper);
        curSen = NULL;
        
        curIdx = 0;
    }
    if (inStr[strlen(inStr)-1] == delimiter){
        char * empty = "";
        vector_push_back(vec, empty);
    }

    if (curSen){
        free(curSen);
        curSen = NULL;
    }

    free(inStr);
    return vec;
    //SPLIT WITH STRTOK: WRONG FOR EMPTY STRINGS
    // char *inStr = sstring_to_cstr(this); //need to free
    // char sArr[strlen(inStr)+1];
    // strcpy(sArr, inStr);
    // //initialize output vector:
    // vector *vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    // //get tokens:
    // char *tok = strtok(sArr, &delimiter);
    
    // while (tok){
    //     vector_push_back(vec, (void *) tok);
    //     tok = strtok(NULL, &delimiter);
    // }
    // free(inStr);
    // return vec;
}


int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    //input as string
    char * inStr = sstring_to_cstr(this); //need to free
    int inLen = strlen(inStr); 
    //printf("in string: %s\n", inStr); // TEST
    int tar_len = strlen(target);// length of target
    //printf("target length: %lu\n", strlen(target)); // TEST
    // starts from offset
    int i = offset;
    //substring declared outside
    char * substring = NULL;
    // printf("str: %s, str len: %d, target len: %d\n", inStr, inLen, tar_len);
    for (; i < inLen; i++){
        // check that target can exist after offset based on length:

        if (tar_len + i > inLen){
            free(inStr);
            // printf("ERROR: No target found\n"); // TEST
            return -5;
        }
        
        // get substring from curr index:
        substring = getSubString(inStr, i, tar_len); //Need to Free
        // char subStr[strlen(substring)];
        // printf("Comparing substring: %s, len: %lu\n", substring, strlen(substring)); // TEST
        //if there is a match:
        if (strcmp(substring, target) == 0){
            
            //erase the target from vector: 
            int j = 0;
            // printf("Erasing target:\n"); // TEST
            for (;j < tar_len; j++){
                // char * cur = sstring_to_cstr(this);
                // printf("Current string: %s\n", cur); // TEST
                // free(cur);
                vector_erase(this->svec, i);
                this->size--;
            }
            //add substitute to vector:
            int subLen = strlen(substitution);
            char * subCpy = malloc(subLen+1); //Need to Free
            strcpy(subCpy, substitution);
            int k = 0;
            for (;k < subLen; k++){
                // char * cur = sstring_to_cstr(this);
                // printf("Current string: %s\n", cur);
                // free(cur);
                void * cToAdd = &subCpy[k];
                vector_insert(this->svec, i+k, cToAdd);
                this->size++;
            }
            free(substring);
            free(subCpy);
            free(inStr);
            return 0;
        }
        free(substring);
        substring = NULL;
    }
    free(inStr);
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) { //returns a heap string
    // your code goes here
    char * inStr = sstring_to_cstr(this); //Need to free
    char * subStr = getSubString(inStr, start, end - start);
    free(inStr);
    return subStr;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->svec);
    this->svec = NULL;
    free(this);
    this = NULL;
}


