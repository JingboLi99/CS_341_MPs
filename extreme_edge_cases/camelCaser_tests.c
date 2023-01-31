/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

//return 0 for different, 1 for same
int compare(char **ans, char **res){
    if (!res && !ans){
        return 1;
    }
    if ((!res && ans) || (res && !ans)){
        return 0;
    }else{
        int i = 0;
        while (res[i] && ans[i]){
            char * ans_str = ans[i];
            char * res_str = res[i];
            if (strcmp(ans_str, res_str)){ // 0 is same, >0 if different
                return 0; 
            }
            i++;
        }if (res[i] || ans[i]){ // if either one still not 0
            return 0;
        }
        return 1;
    }
}

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    //0. Simple:
    char * test_input_0 = "Hi.";
    char ** test_ouput_0 = camelCaser(test_input_0);
    char * actual_output_0[] = {"hi",NULL};
    
    if (compare(test_ouput_0, actual_output_0)){
        printf("Test 0 passed \n");
        fflush(stdout);
        destroy(test_ouput_0);
    }else{
        destroy(test_ouput_0);
        return 0;
    }
    //1. Given.
    char * test_input_1 = "The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.";
    char ** test_ouput_1 = camelCaser(test_input_1);
    char * actual_output_1[] = {"theHeisenbugIsAnIncredibleCreature", "facenovelServersGetTheirPowerFromItsIndeterminism", "codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener", "godObjectsAreTheNewReligion", NULL};
    
    if (compare(test_ouput_1, actual_output_1)){
        printf("Test 1 passed \n");
        fflush(stdout);
        destroy(test_ouput_1);
    }else{
        destroy(test_ouput_1);
        return 0;
    }
    
    // 2. Consecutive punctuation.
    char * test_input_2 = "!!This is ÓSentence??Hi.";
    char ** test_ouput_2 = camelCaser(test_input_2);
    char * actual_output_2[] = {"","","thisIsÓSentence","","hi",NULL};

    if (compare(test_ouput_2, actual_output_2)){
        printf("Test 2 passed \n");
        fflush(stdout);
        destroy(test_ouput_2);
    }else{
        destroy(test_ouput_2);
        return 0;
    }

    // 3. only spaces between puncuation.
    char * test_input_3 = "!!      .   ??  .";
    char ** test_ouput_3 = camelCaser(test_input_3);
    char * actual_output_3[] = {"","","","","","",NULL};

    if (compare(test_ouput_3, actual_output_3)){
        printf("Test 3 passed \n");
        fflush(stdout);
        destroy(test_ouput_3);
    }else{
        destroy(test_ouput_3);
        return 0;
    }
    // 4. ending with words no punctuation.
    char * test_input_4 = "this is test. another Sentence";
    char ** test_ouput_4 = camelCaser(test_input_4);
    char * actual_output_4[] = {"thisIsTest",NULL};

    if (compare(test_ouput_4, actual_output_4)){
        printf("Test 4 passed \n");
        fflush(stdout);
        destroy(test_ouput_4);
    }else{
        destroy(test_ouput_4);
        return 0;
    }

    //5. Numbers.
    char * test_input_5 = "5513.00_52.003/";
    char ** test_ouput_5 = camelCaser(test_input_5);
    char * actual_output_5[] = {"5513","00","52","003",NULL};

    if (compare(test_ouput_5, actual_output_5)){
        printf("Test 5 passed \n");
        fflush(stdout);
        destroy(test_ouput_5);
    }else{
        destroy(test_ouput_5);
        return 0;
    }

    //6. NULL
    char * test_input_6 = NULL;
    char ** test_ouput_6 = camelCaser(test_input_6);
    char ** actual_output_6 = NULL;

    if (compare(test_ouput_6, actual_output_6)){
        printf("Test 6 passed \n");
        fflush(stdout);
        destroy(test_ouput_6);
    }else{
        destroy(test_ouput_6);
        return 0;
    }

    //7. All space
    char * test_input_7 = "   ";
    char ** test_ouput_7 = camelCaser(test_input_7);
    char * actual_output_7[] = {NULL};

    if (compare(test_ouput_7, actual_output_7)){
        printf("Test 7 passed \n");
        fflush(stdout);
        destroy(test_ouput_7);
    }else{
        destroy(test_ouput_7);
        return 0;
    }

    //8. All punc
    char * test_input_8 = "%^*(#%)_+";
    char ** test_ouput_8 = camelCaser(test_input_8);
    char * actual_output_8[] = {"","","","","","","","","",NULL};

    if (compare(test_ouput_8, actual_output_8)){
        printf("Test 8 passed \n");
        fflush(stdout);
        destroy(test_ouput_8);
    }else{
        destroy(test_ouput_8);
        return 0;
    }
    //9. Weird characters:
    char * test_input_9 = "Ó.Ô..Ò.Â».’.ﬂ.—.";
    char ** test_ouput_9 = camelCaser(test_input_9);
    char * actual_output_9[] = {"Ó","Ô","","Ò","Â»","’","ﬂ","—",NULL};


    if (compare(test_ouput_9, actual_output_9)){
        printf("Test 9 passed \n");
        fflush(stdout);
        destroy(test_ouput_9);
    }else{
        destroy(test_ouput_9);
        return 0;
    }

    printf("All tests passed \n");
    return 1;

}