/**
 * utilities_unleashed
 * CS 341 - Spring 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include "format.h"

int main(int argc, char *argv[]) {
    // check that at least 3 arguments
    if (argc < 3){
        print_env_usage();
        exit(1);
    }
    //check that there is an executable program at the end:
    if (strcmp(argv[argc-1], "--") == 0){ // '--' cannot be the last argument
        print_env_usage();
        exit(1);
    }

    pid_t child_id = fork();

    if (child_id > 0){
        // this is parent
        int status;
        waitpid(child_id, &status, 0);
        if (WEXITSTATUS(status)) {
            exit(1);
        }
    }else if (! child_id){
        // this is in child:
        int i = 1; //skip current program name in argv
        while (strcmp(argv[i], "--") != 0){ //iterate through all the env key=val pairs
            char* cPair = argv[i];
            char *p = strchr(cPair, '=');
            if (p == NULL){
                print_env_usage();
                exit(1);
            }

            int key_len = p - cPair;
            int val_len = strlen(cPair) - key_len - 1;

            if (key_len <= 0 || val_len <= 0){ //check that neither are 0
                print_env_usage();
                exit(1);
            }

            char key[key_len + 1];
            char val[val_len + 1];
            
            int j = 0;
            for (; j < key_len;j++){
                key[j] = *(cPair + j);
            }
            key[key_len] = '\0';

            j = 0;
            for (; j < val_len;j++){
                val[j] = *(cPair + key_len + 1 + j);
            }
            val[val_len] = '\0';

            bool is_ref = false;
            
            //check if val is a reference:
            if (val[0] == '%'){
                
                for (int i = 0; i < val_len - 1; i++){
                    val[i] = val[i+1];
                }
                val[val_len-1] = '\0';
                val_len --;
                is_ref = true;
            }
            
            if (is_ref){
                char * cur_val = getenv(val);
                if (cur_val == NULL){
                    print_environment_change_failed();
                    exit(1);
                }
                int stat = setenv(key, cur_val, 1);
                if (stat != 0){
                    print_environment_change_failed();
                    exit(1);
                }
            }else{
                int stat = setenv(key, val, 1);
                if (stat != 0){
                    print_environment_change_failed();
                    exit(1);
                }
            }
            i++;
        }
        char ** args = &argv[i];
        execvp(args[1], &args[1]);
        print_exec_failed();
        exit(1);
    }else{
        print_fork_failed();
        exit(1);
    }


    return 0;

}
