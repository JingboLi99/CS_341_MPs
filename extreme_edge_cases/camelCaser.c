/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (!input_str){
        return NULL;
    }

    char **result = NULL; //array of char pointers
    int in_size = strlen(input_str); // size of the input string
    int c = 0;
    int s = 0; // c is the input index, s is the sentence count in results
    char *curr_sent = NULL;
    int res_size = 0; //tracks size of result array (how many pointers)
    int sent_size = 0; // tracks suze of current sentence to be cameled.
    
    for (;c < in_size; c++){
        if (ispunct(input_str[c])){
            //current resultant camel cased string
            char * new_camel = make_camel(curr_sent); // CHECK for null.
            
            //delete and free current sentence:
            free(curr_sent);
            curr_sent = NULL;
            sent_size = 0;
            if (!new_camel){ //check that returned camelcase string is not NULL or all whitespace: "  "
                new_camel = "";
            }
            // reallocate memory to result to accomadate new string:
            res_size ++;
            result = realloc(result, sizeof(char *) * res_size); // current size + 8 for a new char pointer
            result[s] = new_camel;
            s+=1;

        }else{
            sent_size ++;
            printf("Size of curr sen: %d\n", sent_size);
            curr_sent = realloc(curr_sent, sent_size);
            strncat(curr_sent, &input_str[c], 1); //append char to current sentence
        }
        
    }
    
    //add NULL ending character to result
    res_size++;
    result = realloc(result, sizeof(char *) + res_size); //add more space to accomodate end NULL
    result[s+1] = NULL;

    return result;
}

char *make_camel(const char* in_sent){
    if (! in_sent){
        return NULL;
    }
    
    int in_size = strlen(in_sent);
    int i = 0;
    int newWord = 0; //boolean to check if need to caps
    char *toRet = NULL;
    int n_chars = 0;
    //int c_ptr = -1;
    
    for (; i < in_size; i++){
        if (in_sent[i] == ' '){
            if (i != 0){
                newWord = 1;    
            }
            continue;
        }
        n_chars ++;
        toRet = realloc(toRet, n_chars);

        if (!isalpha(in_sent[i])){ // if it is not an alphabet, add directly
            // printf("Adding non alpha: %c\n", in_sent[i]);
            strncat(toRet, &in_sent[i], 1);
            newWord = 0;
        }
        //all words evaluated from here are alphabets:
        else if (newWord){
            char is_upper = toupper(in_sent[i]);
            // printf("Adding first letter alphabet: %c\n", is_upper);
            fflush(stdout);
            strncat(toRet, &is_upper, 1);
            newWord = 0;
        }
        else{
            char is_lower = tolower(in_sent[i]);
            // printf("Adding normal alphabet: %c\n", is_lower);
            fflush(stdout);
            strncat(toRet, &is_lower, 1);
        }
    }
    
    return toRet; // toRet can be NULL
}

void destroy(char **result) {
    // TODO: Implement me!
    int i = 0;
    while (result[i]){
        free(result[i]);
        result[i] = NULL;
    }
    free(result);
    result = NULL;

    return;
}
