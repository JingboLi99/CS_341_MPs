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




