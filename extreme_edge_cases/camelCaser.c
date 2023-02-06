/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
if ispunct:
1. add /0 to curr string
2. create properly sized string
3. copy string over
4. free previous string
5. add properly sized string to toRet array
6. reset curSen
7. Reallocate curSen
8. reset makeCaps, curIdx
9. Increment retIdx

if isspace and currSidx != 0:
1. set makeCaps = 1

if not isalpha:
- directly add to curr string
-DO NOT reset makeCaps = 0

if isalpha and makeCaps:
- make caps and add curr string
-reset makeCaps = 0

if isalpha and not makeCaps:
- make tolower and add to curr string

*/

char **camel_caser(const char *input_str) {
    if (!input_str){
        return NULL;
    }
    
    int inlen = strlen(input_str);
    int sc_in = 0;
    
    // counting the number of sentences (works)
    int n_sentences = 0;
    for (;sc_in < inlen; sc_in++){
        if (ispunct(input_str[sc_in])){
            n_sentences++;
        }
    }
    
    char ** toRet = (char **) malloc(sizeof(char *) * (n_sentences + 1)); 
    toRet[n_sentences] = NULL;
    int retIdx = 0;
    int in = 0;

    char * curSen = (char*) malloc(inlen);
    int curIdx = 0;
    // flags decleration:
    int makeCaps = 0;
    char cur_char = '\0';
    char * proper = NULL;
    
    for (;in < inlen; in ++){
        cur_char = input_str[in];
        
        if (ispunct(cur_char) != 0){
            proper = NULL;
            if (curIdx == 0){
                proper = (char *)malloc(1);
                strcpy(proper, "");
            }else{
                curSen[curIdx] = '\0';
                int curSenLen = strlen(curSen);
                proper = (char *)malloc(curSenLen+1);
                proper[curSenLen] = '\0';
                int j = 0;
                for (;j < curSenLen; j++){
                    proper[j] = curSen[j];
                }
            }
            if (retIdx >= n_sentences){
                printf("*ERROR*:Number of sentences is wrong!"); //DEBUG
            }
            toRet[retIdx] = proper;
            free(curSen);
            curSen = NULL;
            curSen = (char*)malloc(inlen);
            //reset flags:
            makeCaps = 0;
            curIdx = 0;
            retIdx++;
            continue;
        }
        
        if (isspace(cur_char)){
            if (curIdx != 0){
                makeCaps = 1;
            }
            continue;
        }
        
        if (isalpha(cur_char) == 0){ // if not alphabet
            curSen[curIdx] = cur_char;
            curIdx++;
        }
        else{
            if (makeCaps){
                char madeCaps = toupper(cur_char);
                curSen[curIdx] = madeCaps;
                makeCaps = 0;
            }else{
                char madeNoCaps = tolower(cur_char);
                curSen[curIdx] = madeNoCaps;
            }
            curIdx++;
        }
        
    }

    if (curSen){
        free(curSen);
        curSen = NULL;
    }

    return toRet;
}

void destroy(char **result) {
    // TODO: Implement me!
    int i = 0;
    if (result == NULL){
        return;
    }
    while (result[i] != NULL){
        //printf("To be destroyed: %s\n", result[i]);
        free(result[i]);
        result[i] = NULL;
        i++;
    }
    free(result);
    result = NULL;

    return;
}


//**Get to checking why the below code is wrong.

// char **camel_caser(const char *input_str) {
//     // TODO: Implement me!
//     if (!input_str){
//         return NULL;
//     }
//     int in_size = strlen(input_str); // size of the input string
//     char **result = (char **) malloc(in_size * sizeof(char *)); //array of char pointers
    
//     int c = 0;
//     int s = 0; // c is the input index, s is the sentence count in results
//     char *curr_sent = (char *) malloc(300); //current sentence after previous punctuation
//     int sent_size = 0; // tracks size of current sentence to be cameled.
    
//     for (;c < in_size; c++){
//         if (ispunct(input_str[c])){
//             if (sent_size > 0){
//                 char end = '\0';
//                 strncat(curr_sent, &end, 1);
//                 curr_sent = realloc(curr_sent, sent_size+1);
//             }else{
//                 free(curr_sent);
//                 curr_sent = NULL;
//             }
//             char * new_camel = make_camel(curr_sent); // CHECK for null.
//             free(curr_sent);
//             curr_sent = (char *) malloc(300);;
//             sent_size = 0;
//             if (!new_camel){ //check that returned camelcase string is not NULL or all whitespace: "  "
//                 new_camel = "";
//             }
//             result[s] = new_camel;
//             printf("Camel added: %s\n", result[s]);
//             s++;          

//         }else{
//             strncat(curr_sent, &input_str[c], 1); //append char to current sentence
//             sent_size ++;
//         } 
//     }
    
//     result[s] = NULL;
//     result = realloc(result, sizeof(char *) * (s+1));
//     return result;
// }

// char *make_camel(const char* in_sent){
//     if (! in_sent){
//         return NULL;
//     }
//     printf("sent in: %s\n", in_sent);
//     int in_size = strlen(in_sent);
//     int i = 0;
//     int newWord = 0; //boolean to check if need to caps
//     char *toRet =NULL;
//     int n_chars = 0;
    
//     for (; i < in_size; i++){
//         if (in_sent[i] == ' '){
//             if (i != 0){
//                 newWord = 1;    
//             }
//             continue;
//         }
//         n_chars ++;
//         toRet = realloc(toRet, n_chars);
//         if (!isalpha(in_sent[i])){ // if it is not an alphabet, add directly
            
//             strncat(toRet, &in_sent[i], 1);
//             newWord = 0;
//         }
//         //all words evaluated from here are alphabets:
//         else if (newWord){
//             char is_upper = toupper(in_sent[i]);
//             strncat(toRet, &is_upper, 1);
//             newWord = 0;
//         }
//         else{
//             char is_lower = tolower(in_sent[i]);
//             strncat(toRet, &is_lower, 1);
//         }
        
//     }
//     if (n_chars > 0){
//         toRet = realloc(toRet, n_chars+1);
//         char end = '\0';
//         strncat(toRet, &end, 1);
//         //toRet = realloc(toRet, n_chars+1);
//     }else{
//         free(toRet);
//         toRet = NULL;
//     }
//     printf("to ret: %s\n", toRet);
//     return toRet; // toRet can be NULL
// }

// void destroy(char **result) {
//     // TODO: Implement me!
//     int i = 0;
//     while (result[i]){
//         free(result[i]);
//         result[i] = NULL;
//     }
//     free(result);
//     result = NULL;

//     return;
// }