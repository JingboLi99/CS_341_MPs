/**
 * shell
 * CS 341 - Spring 2023
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_IN 1024
#define MAX_PATH 256

typedef struct process {
    char *command;
    pid_t pid;
} process;

//GLOBAL Variables:
static int operator_type = -1; //-1 for no operator (only 1 command), 0 = &&,  1 = ||, 2 = ;
//Helper functions:
//s is start index (inclusive), len is length from start index
char* getSubString(char *in, int s, int len){
    int i = 0;
    char *dest = malloc(len+1);
    dest[len] = '\0';
    for (;i < len; i++){
        dest[i] = in[i+s];
    }
    return dest;
}
char * get_str_from_vec(vector * cur_cmd){
    size_t i = 0;
    char * ret_str = malloc(MAX_IN);
    strcpy(ret_str, "");
    size_t vec_size = vector_size(cur_cmd);
    for (; i < vec_size; i++){
        char *cur_str = (char *)vector_get(cur_cmd, i);
        strcat(ret_str, cur_str);
        if (i != vec_size-1){ //do not add space after the last word
            strcat(ret_str, " ");
        }
    }
    return ret_str;
}
char ** vec_to_strArr(vector* cmd_segments){
    int vec_size = vector_size(cmd_segments);
    char ** ret_arr = malloc((vec_size+1) * sizeof(char *)); //+1 to account for NULL ending for arr: necessary for execvp
    ret_arr[vec_size] = NULL;
    for (int i = 0; i < vec_size; i++){
        ret_arr[i] = vector_get(cmd_segments, i);
    }
    return ret_arr;
}
vector *str_to_vector(char *input){
    sstring * in_sstr = cstr_to_sstring(input); //Need to free
    vector *splitCmds = sstring_split(in_sstr, ' '); // needs to be freed by calling function
    sstring_destroy(in_sstr);

    return splitCmds;
}
void sig_handler(){
    //TODO
    return;
}

void handle_eof(){
    //TODO
    return;
}
// handles file opening: type 0  = history file, type 1 = script file
FILE* file_handler(char * filename, int type){
    int in_len = strlen(filename);
    sstring* file_sstr = cstr_to_sstring(filename);
    char* file_type = sstring_slice(file_sstr, in_len - 4, in_len);
    sstring_destroy(file_sstr);
    if (strcmp(file_type, ".txt") != 0){ //check that file input is a valid file type (.txt)
        if (type == 0) print_history_file_error();
        else print_script_file_error();
        return NULL; //TODO: Check
    }
    FILE *fp;
    if (type == 0){
        if (!(fp = fopen(filename, "a"))){
            print_history_file_error();
            return NULL;
        }
    }
    else{
        if (!(fp = fopen(filename, "r"))){
            print_script_file_error();
            return NULL;
        }
    }
    return fp;
}
vector **split_by_operator(char * in){
    vector *cmds = str_to_vector(in); //change input into vectors. Need to free
    size_t operator_idx = 500000000; 
    int n_cmds = 1;
    size_t cmd_vec_size = vector_size(cmds);
    for (size_t i = 0; i < cmd_vec_size; i++){
        if (strcmp((char *)vector_get(cmds, i), "&&") == 0){
            operator_type = 0;
            operator_idx = i;
            n_cmds ++;
        }
        else if (strcmp((char *)vector_get(cmds, i), "||") == 0){
            operator_type = 1;
            operator_idx = i;
            n_cmds ++;
        }
        else{
            char * cur_word = (char *)vector_get(cmds, i);
            if (cur_word[strlen(cur_word)-1] == ';'){
                operator_type = 2;
                vector_insert(cmds, i+1, ";"); //add a new ";" in vector
                cmd_vec_size++; //increment cmd vector size
                //remove ';' from the previous string:
                cur_word[strlen(cur_word)-1] = '\0'; //change ; to \0 in order to end the string earlier
                operator_idx = i+1;
                i++; //add on i to skip the newly added ";" in vector
                n_cmds ++;
            }
        }
    }
    //catch operator chaining eg. cmd a && cmd b || cmd c 
    //catch invalid operator syntax: no command before or after operator
    // eg. "&& cmd1" or "cmd1 ||" or "cmd1;"
    if (n_cmds > 2 || operator_idx == 0 || operator_idx == cmd_vec_size-1){
        vector_destroy(cmds); //freed cmds vector
        return NULL;
    }
    //split current cmd vector into 2 seperate cmd vectors if operator is involved
    vector** cmd_arr = NULL;
    if (n_cmds == 1){
        cmd_arr = malloc(2 * sizeof(vector *));
        cmd_arr[1] = NULL;
        cmd_arr[0] = cmds;
    }
    else{
        cmd_arr = malloc(3 * sizeof(vector *));
        cmd_arr[2] = NULL;
        vector *cmd_one = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
        cmd_arr[0] = cmd_one;
        for (size_t m = 0; m < operator_idx; m++){
            char * seg = vector_get(cmds, m);
            vector_push_back(cmd_one, seg);
        }
        vector *cmd_two = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
        cmd_arr[1] = cmd_two;
        for (size_t m = operator_idx+1; m < cmd_vec_size; m++){
            char * seg = vector_get(cmds, m);
            vector_push_back(cmd_two, seg);
        }
        vector_destroy(cmds);
    }
    return cmd_arr;
}
char * get_pref_cmd(vector * cmd_hist, char * pref_cmd){
    char * prefix = getSubString(pref_cmd, 1, strlen(pref_cmd)-1); //Stripping off starting '!'. Need to free
    int pref_len = strlen(prefix);
    int cmd_idx = vector_size(cmd_hist) - 1;
    char * cmd_pref = NULL;
    for (; cmd_idx >= 0; cmd_idx--){
        char * cur_cmd = vector_get(cmd_hist, cmd_idx);
        int cur_cmd_len = strlen(cur_cmd);
        if (cur_cmd_len < pref_len){
            continue;
        }
        cmd_pref = getSubString(cur_cmd, 0, pref_len); //Need to free
        if (strcmp(prefix, cmd_pref) == 0){ //if there is a match between prefix and first pref_len str of current command
            free(prefix);
            free(cmd_pref);
            return cur_cmd;
        }
    }
    free(prefix);
    if (cmd_pref) free(cmd_pref);
    
    return "**No prefix command";
}
char * get_cmd_at_idx(vector *cmd_hist, char * cmd_input){
    char * idx_str = getSubString(cmd_input, 1, strlen(cmd_input)-1); //String without starting '#' Need to free
    //check idx_str is valid
    bool valid = true;
    if (strlen(idx_str) == 0){
        valid = false;
    }
    for (size_t i = 0; i < strlen(idx_str); i++) {
        if (!isdigit(idx_str[i])) {
        valid = false;
        break;
        }
    }
    int idx = atoi(idx_str);
    free(idx_str);
    int vec_size = (vector_size(cmd_hist) <= INT_MAX) ? (int) vector_size(cmd_hist) : INT_MAX;
    if (!valid || idx >= vec_size){
        return "**Not a valid index";
    }
    
    return vector_get(cmd_hist, idx);
}
int path_change_handler(vector * cur_cmd){
    char * path = vector_get(cur_cmd, 1);
    if (path[0] == '/'){// this is an absolute path
        if (chdir(path) != 0){
            print_no_directory(path);
            return 1;
        }
    }
    else{ //this is a relative path
        char * fullpath = get_full_path(path);
        if (chdir(fullpath) != 0){
            print_no_directory(fullpath);
            free(fullpath);
            return 1;
        }
        free(fullpath);
    }
    return 0;
}

int command_handler(char * command){
    //Check if command is a change path command (from !<prefix> or #<n> command executions that skip the original path change handler)
    char * first_two_chars = getSubString(command, 0, 2);
    vector * cmd_segments = str_to_vector(command); //Need to free
    if (strcmp(first_two_chars, "cd") == 0){
        if (vector_size(cmd_segments) != 2){ //check that the format is cd <path> and has 2 words
            print_invalid_command(command);
            vector_destroy(cmd_segments);
            return 1; // return one for invalid command
        }
        //if command is a valid change path command, call path_change_handler
        int path_change_stat = path_change_handler(cmd_segments);
        vector_destroy(cmd_segments);
        return path_change_stat;
    }
    //For remaining external commands:
    else{
        pid_t child_id = fork();
        if (child_id > 0){
            // this is parent
            int status;
            waitpid(child_id, &status, 0);
            if (WEXITSTATUS(status)) {
                vector_destroy(cmd_segments);
                return 1;
            }
            // print_command_executed(child_id); //external command successfully executed
        }
        else if (! child_id){
            //this is child
            pid_t cur_id = getpid();
            print_command_executed(cur_id); //external command successfully executed
            char ** args = vec_to_strArr(cmd_segments); //Need to free
            execvp(args[0], &args[0]);
            print_exec_failed(command);
            free(args); //each individual char * in args is freed when we destroy cmd_segments
            exit(1);
        }
        else{
            print_fork_failed();
            vector_destroy(cmd_segments);
            return 1; 
        }
        vector_destroy(cmd_segments);
        return 0;
    }
}

int shell(int argc, char *argv[]) {

    //variables and flags declaration:
    bool hasScript = false;
    char * script_file = NULL;
    FILE * scrpt_rdr = NULL;
    bool hasHist = false;
    char * hist_file = NULL;
    FILE * hist_wtr = NULL;
    char *input = NULL;
    char *path = NULL;
    int pid;
    vector *cmd_hist = vector_create(string_copy_constructor, string_destructor, string_default_constructor); //need to free
    //process cmd line args:
    if (argc > 1){ //check that additional cmd line arguments are passed in:
        int i = 1;
        while (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-f") == 0){
            if (strcmp(argv[i], "-h") == 0 && (i < argc)){
                hasHist = true;
                hist_file = argv[i+1];
                hist_wtr = file_handler(hist_file, 0); //need to close
                if (!hist_wtr){ // there was a print history file error
                    vector_destroy(cmd_hist);
                    return 1;
                }
            }
            else if(strcmp(argv[i], "-f") == 0 && (i < argc)){
                hasScript = true;
                script_file = argv[i+1];
                scrpt_rdr = file_handler(script_file, 1); //need to close
                if (!scrpt_rdr){ // there was a print script file error
                    vector_destroy(cmd_hist);
                    return 1;
                }
            }
            else{
                print_usage(); //all other situations, the command line arguments are not passed correctly
                vector_destroy(cmd_hist);
                return 1;
            }
            i++;
        }
    }
    // Run a loop constantly checking for std/ file input
    while(1){        
        // handle SIGINT
        signal(SIGINT, sig_handler);
        // Prompt user with current pid and path
        pid = getpid();
        size_t path_size = MAX_PATH * sizeof(char);
        path = malloc(path_size);
        getcwd(path, 256);
        if (!hasScript) print_prompt(path, pid); // do not print if file input
        //get current input:
        size_t in_size = MAX_IN * sizeof(char);
        input = malloc(in_size); //Need to free

        // If there is a script file, change the input to come from script file (previously was to check for EOF)
        if (hasScript){
            if (getline(&input, &in_size, scrpt_rdr) >= 0){ //if there are still lines to read in file:
                if (input[strlen(input)-1] == '\n'){
                    char * std_temp = getSubString(input, 0, strlen(input)-1); //strip off the '\n'
                    free(input);
                    input = std_temp;
                    std_temp = NULL;
                }
            }
            else{ //no more lines to read (EOF): getline returns -1
                //TODO: Kill running background child processes
                handle_eof();
                //free all resources up to this point. close files
                if (input) free(input);
                if (path) free(path);
                vector_destroy(cmd_hist);
                if (hist_wtr) fclose(hist_wtr);
                if (scrpt_rdr) fclose(scrpt_rdr);
                // printf("\n"); // if we come directly from printing prompt, there will be no new line
                //exit(0); //testing for the recursive shell call thing
                return 0;
            }
        }
        //No script file, input comes from stdin
        else{
            if (getline(&input, &in_size, stdin) >= 0){
                char * std_temp = getSubString(input, 0, strlen(input)-1); //strip off the '\n'
                free(input);
                input = std_temp;
                std_temp = NULL;
            }
            else{ // getline returns -1: EOF or stdin read error:
                handle_eof();
                //free all resources up to this point. close files
                if (input) free(input);
                if (path) free(path);
                vector_destroy(cmd_hist);
                if (hist_wtr) fclose(hist_wtr);
                if (scrpt_rdr) fclose(scrpt_rdr);
                printf("\n");
                return 1;
            }
        }
        //If there is a history file, write current input to file. Else, just write to history vector
        if (hasHist){
            //TODO: write empty line
            if (strlen(input) == 0 || (input[0] != '#' && input[0] != '!' && strcmp(input, "exit") != 0)){
                fprintf(hist_wtr, "%s", input);
                fprintf(hist_wtr, "%s", "\n");
            }
        }
        //record command in history only if it is not one of the built in commands or exit
        if (strlen(input) == 0 || (input[0] != '#' && input[0] != '!' && strcmp(input, "exit") != 0)){
            vector_push_back(cmd_hist, input); //write input to a history vector
        }
        //split commands by logical operator
        vector **cmd_arr = split_by_operator(input); //Need to free each vector in this array: free at end of inner while loop
        if (! cmd_arr){ // Incorrect use of operators
            print_invalid_command(input); 
            if (input) free(input);
            if (path) free(path);
            continue; //get next prompt, do not go through inner loop
        }
        //**Command Execution**
        int cmd_idx = 0;
        int n_cmds = sizeof(cmd_arr)/sizeof(cmd_arr[0]);
        int exit_status = 0; //0 is successfully exited
        while (cmd_arr[cmd_idx]){ //cmd_arr is null terminated
            #// operator flagging: check conditions to BREAK out of while loop
            if (cmd_idx == 1 && ((operator_type == 0 && exit_status == 1) ||
                                 (operator_type == 1 && exit_status == 0) ||
                                  operator_type == -1)){ //this last condition shouldnt happen, since cmd_idx shouldnt go to 1 if no operator
                                    break; //check all loop resources have been freed
                                 }
            // command variables
            vector * cur_cmd = cmd_arr[cmd_idx];
            int cur_cmd_segments = vector_size(cur_cmd); //how many words are in the current command (ie !<history> is one word but cd <path> is two)
            char * cur_cmd_str = get_str_from_vec(cur_cmd); // current command in string form: Need to free
            char * cmd_str_1 = (char *) vector_get(cur_cmd, 0); //get first string in command
            //**Built in commands**
            //Check for exit command:
            if (strcmp(cur_cmd_str, "exit") == 0){
                handle_eof();
                //free all resources up to this point. close files: Make sure to free vector array
                if (cur_cmd_str) free(cur_cmd_str);
                cur_cmd_str = NULL;
                if (input) free(input);
                input = NULL;
                if (path) free(path);
                path = NULL;
                vector_destroy(cmd_hist);
                if (hist_wtr) fclose(hist_wtr);
                if (scrpt_rdr) fclose(scrpt_rdr);
                return 1;
            }
            // Check for !history: print all history (must be stand alone command)
            if (strcmp(vector_get(cur_cmd, 0), "!history") == 0){
                if (n_cmds > 1 || cur_cmd_segments > 1){ //check that this is a stand alone command and command only has 1 line
                    print_invalid_command(cur_cmd_str);
                    //Free current loop resources, not all resources
                    free(cur_cmd_str);
                    break; //TODO: check all loop resources have been freed
                }
                for (size_t his_idx = 0; his_idx < vector_size(cmd_hist); his_idx++){
                    print_history_line(his_idx, (char *)vector_get(cmd_hist, his_idx));
                }
                //Free current loop resources, not all resources
                free(cur_cmd_str);
                break; //no more commands to execute. TODO: check all loop resources have been freed
            }
            //check for !<prefix> (Everything after prefix is considered the search term)
            else if (cmd_str_1[0] == '!'){
                char * prev_cmd = get_pref_cmd(cmd_hist, cur_cmd_str); //everything behind '!' in the whole command string is a search term
                if (strcmp(prev_cmd, "**No prefix command") == 0){ //if no prefix command match
                    print_no_history_match();
                }
                else{
                    vector_push_back(cmd_hist, prev_cmd); //write prev_cmd to cmd_hist vector again
                    //write prev_cmd to hist file again
                    if (hasHist){
                        fprintf(hist_wtr, "%s", prev_cmd);
                        fprintf(hist_wtr, "%s", "\n");
                    }
                    //TODO: Print and Execute last command
                    print_command(prev_cmd);     
                    // command_handler(prev_cmd);               
                    // ***HANDLE SECONDARY COMMAND LINE FROM HISTORY***
                    int og_op_type =  operator_type;
                    vector ** sec_cmd_arr = split_by_operator(prev_cmd); //Need to free: This array will not contain ! # exit.
                    if (! sec_cmd_arr){ // Incorrect use of operators
                        print_invalid_command(prev_cmd);
                    }
                    else{
                        int sec_cmd_idx = 0;
                        int sec_exit_stat = 0;
                        int sec_n_cmds = sizeof(sec_cmd_arr)/sizeof(sec_cmd_arr[0]);
                        while(sec_cmd_arr[sec_cmd_idx]){
                            if (sec_cmd_idx == 1 && ((operator_type == 0 && sec_exit_stat == 1) ||
                                                     (operator_type == 1 && sec_exit_stat == 0) ||
                                                     operator_type == -1)){ //this last condition shouldnt happen, since cmd_idx shouldnt go to 1 if no operator
                                                        break; //check all loop resources have been freed
                                                    }
                            vector * sec_cur_cmd = sec_cmd_arr[sec_cmd_idx];
                            char * sec_cur_cmd_str = get_str_from_vec(sec_cur_cmd); // current command in string form: Need to free
                            sec_exit_stat = command_handler(sec_cur_cmd_str);
                            free(sec_cur_cmd_str);
                            sec_cmd_idx++;
                        }
                        //free secondary vector array
                        int sec_vec_arr_idx = 0;
                        for (; sec_vec_arr_idx < sec_n_cmds; sec_vec_arr_idx++){
                            if (sec_cmd_arr[sec_vec_arr_idx]) vector_destroy(sec_cmd_arr[sec_vec_arr_idx]);
                            sec_cmd_arr[sec_vec_arr_idx] = NULL;
                        }
                        free(sec_cmd_arr);
                        sec_cmd_arr = NULL;
                    }
                    operator_type = og_op_type; //Change back the operator type to the original. Second split_by_operator call will chnage it
                }
                //Free current loop resources, not all resources
                free(cur_cmd_str);
                break; //no more commands to execute
            }
            //check for #<n> (must be stand alone command)
            else if (cmd_str_1[0] == '#'){
                // int hist_idx_cmd_stat;
                if (n_cmds > 1 || cur_cmd_segments > 1){ //check that this is a stand alone command and command only has 1 line
                    print_invalid_command(cur_cmd_str);
                    //Free current loop resources, not all resources
                    free(cur_cmd_str);
                    break; //TODO: check all loop resources have been freed
                }
                char *cmd_at_idx = get_cmd_at_idx(cmd_hist, cmd_str_1);
                if (strcmp(cmd_at_idx, "**Not a valid index") == 0){
                    print_invalid_index(cur_cmd_str);
                }
                else{
                    vector_push_back(cmd_hist, cmd_at_idx); //write prev_cmd to cmd_hist vector again
                    //write prev_cmd to hist file again
                    if (hasHist){
                        fprintf(hist_wtr, "%s", cmd_at_idx);
                        fprintf(hist_wtr, "%s", "\n");
                    }
                    //TODO: print and Execute command
                    print_command(cmd_at_idx);
                    // hist_idx_cmd_stat = command_handler(cmd_at_idx);

                    // ***HANDLE SECONDARY COMMAND LINE FROM HISTORY***
                    int og_op_type =  operator_type;
                    vector ** sec_cmd_arr = split_by_operator(cmd_at_idx); //Need to free: This array will not contain ! # exit.
                    if (! sec_cmd_arr){ // Incorrect use of operators
                        print_invalid_command(cmd_at_idx);
                    }
                    else{
                        int sec_cmd_idx = 0;
                        int sec_exit_stat = 0;
                        int sec_n_cmds = sizeof(sec_cmd_arr)/sizeof(sec_cmd_arr[0]);
                        while(sec_cmd_arr[sec_cmd_idx]){
                            if (sec_cmd_idx == 1 && ((operator_type == 0 && sec_exit_stat == 1) ||
                                                     (operator_type == 1 && sec_exit_stat == 0) ||
                                                     operator_type == -1)){ //this last condition shouldnt happen, since cmd_idx shouldnt go to 1 if no operator
                                                        break; //check all loop resources have been freed
                                                    }
                            vector * sec_cur_cmd = sec_cmd_arr[sec_cmd_idx];
                            char * sec_cur_cmd_str = get_str_from_vec(sec_cur_cmd); // current command in string form: Need to free
                            sec_exit_stat = command_handler(sec_cur_cmd_str);
                            free(sec_cur_cmd_str);
                            sec_cmd_idx++;
                        }
                        //free secondary vector array
                        int sec_vec_arr_idx = 0;
                        for (; sec_vec_arr_idx < sec_n_cmds; sec_vec_arr_idx++){
                            if (sec_cmd_arr[sec_vec_arr_idx]) vector_destroy(sec_cmd_arr[sec_vec_arr_idx]);
                            sec_cmd_arr[sec_vec_arr_idx] = NULL;
                        }
                        free(sec_cmd_arr);
                        sec_cmd_arr = NULL;
                    }
                    operator_type = og_op_type; //Change back the operator type to the original. Second split_by_operator call will chnage it
                }
                //Free current loop resources, not all resources
                free(cur_cmd_str);
                break;
            }
            //check for cd <path>
            else if (strcmp(cmd_str_1, "cd") == 0){
                if (cur_cmd_segments != 2){ //check that the format is cd <path> and has 2 words
                    print_invalid_command(cur_cmd_str);
                    exit_status = 1;
                }
                else{
                    exit_status = path_change_handler(cur_cmd);
                }
                free(cur_cmd_str);
            }
            //What is left over are all external commands
            else{
                //TODO: Execute external command
                exit_status = command_handler(cur_cmd_str);
                free(cur_cmd_str);
            }
            cmd_idx++;
        }
        //free vector array
        int vec_arr_idx = 0;
        for (; vec_arr_idx < n_cmds; vec_arr_idx++){
            if (cmd_arr[vec_arr_idx]) vector_destroy(cmd_arr[vec_arr_idx]);
            cmd_arr[vec_arr_idx] = NULL;
        }
        free(cmd_arr);
        cmd_arr = NULL;
        if (input) free(input);
        if (path) free(path);
        input = NULL;
        path = NULL;
    }  
    return 0;
}
