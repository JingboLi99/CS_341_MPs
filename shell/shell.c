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
#include <pthread.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_IN 1024
#define MAX_PATH 256

typedef struct process { //when free process, free string first before free process
    char *command; //command is a string in heap
    pid_t pid;
} process;
//Process constructor, destructor, default_constructor:
void *process_copy_constructor(void * elem){
    if (!elem){
        return elem;
    }
    process * copy = malloc(sizeof(process));
    size_t cmd_strlen = strlen(((process *)elem)->command);
    copy->command = malloc(cmd_strlen+1); //account for \0
    strcpy(copy->command, ((process *)elem)->command); // copy string in elem to copy
    copy->pid = ((process *)elem)->pid;
    return copy;
}
void process_destructor(void *elem){
    if (!elem) return;
    free(((process *)elem)->command); //free the heap string that command points to
    free(elem); // free the actual struct memory

}
void *process_default_constructor(void){
    process * def_prc = malloc(sizeof(process));
    def_prc->command = NULL;
    def_prc->pid = -2; //negative pid means not valid
    return def_prc;
}

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
    //kill any foreground child, ie any child process in the current process group
    // pid_t cur_pid = getpid(); //this is the pid of parent, when sigint is called
    // pid_t cur_pgid = getpgid(cur_pid);
    kill(0, SIGTERM); //send kill to all processes in the process group// TODO: Check if this kills the current process too
    return;
}
//kill all active and stopped processes
void eof_handler(vector * act, vector * stp){
    //TODO
    while (vector_size(act) > 0){
        process * cur_proc = vector_get(act, 0);
        kill(cur_proc->pid, SIGTERM);
        vector_erase(act, 0);
    }
    while (vector_size(stp) > 0){
        process * cur_proc = vector_get(stp, 0);
        kill(cur_proc->pid, SIGTERM);
        vector_erase(stp, 0);
    }
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
//returns the a 3 element string array: ele 1 is redir type, ele 2 is cmd string, ele 3 is redir file name
char** redir_handler(vector * cmd){
    size_t i = 0; //must at least have one command
    size_t vec_size = vector_size(cmd);
    char* op_type = malloc(2);// one for type, one of ending char
    strcpy(op_type, "");
    size_t op_idx = -1;
    char* act_cmd = malloc(MAX_IN);
    strcpy(act_cmd, "");
    char* redir_file = malloc(MAX_IN);
    strcpy(redir_file, "");
    bool get_cmd = true;
    //get operator type and index, if any (operator index cannot be first of last element)
    for (; i < vec_size; i++){
        char * cur_s = vector_get(cmd, i);
        if (strcmp(cur_s, ">") == 0){
            op_idx = i;
            strcpy(op_type, "0");
            act_cmd[strlen(act_cmd)-1] = '\0'; //remove the last space in act cmd
            get_cmd = false;
            continue;
        }
        else if (strcmp(cur_s, ">>") == 0){
            op_idx = i;
            strcpy(op_type, "1");
            act_cmd[strlen(act_cmd)-1] = '\0'; //remove the last space in act cmd
            get_cmd = false;
            continue;
        }
        else if (strcmp(cur_s, "<") == 0){
            op_idx = i;
            strcpy(op_type, "2");
            act_cmd[strlen(act_cmd)-1] = '\0'; //remove the last space in act cmd
            get_cmd = false;
            continue;
        }
        if (get_cmd){ //write non operator strings to command string (strings before operator)
            strcat(act_cmd, cur_s);
            strcat(act_cmd, " ");
        }
        else{ //write non operator strings to file string (strings after operator) NOTE: This should be the last index in vector as filepath is only 1 string
            strcat(redir_file, cur_s);
        }
        
    }
    // if no operator exists:
    if (strcmp(op_type, "") == 0){// no operator detected
        return NULL;
    }
    char **to_ret = malloc(3 * sizeof(char *));
    to_ret[0] = op_type;
    to_ret[1] = act_cmd;
    to_ret[2] = redir_file;
    // printf("Returned commands are: %s, %s, %s\n", op_type, act_cmd, redir_file); //REMOVE
    return to_ret;
}
// a single signal command: must have had one of kill, stop, cont, and one <pid>
int sig_cmd_handler(vector * cmds, vector* active_procs, vector * stopped_procs){
    // TODO: 
    //if signal command format is incorrect, print invalid command error
    if (vector_size(cmds) != 2){
        print_invalid_command(get_str_from_vec(cmds));
        return 1;
    }
    char *endptr = NULL;
    long pid_l = strtol(vector_get(cmds, 1), &endptr, 10); //process to act on
    if (pid_l > INT_MAX || pid_l < 1 || *endptr != '\0') {
        print_no_process_found(pid_l);
        return 1;
    }
    pid_t cpid = (pid_t) pid_l;
    char * cmd  = (char *) vector_get(cmds, 0);
    size_t pidx = 0;
    size_t n_act_procs = vector_size(active_procs);
    size_t n_stopped_procs = vector_size(stopped_procs);
    //iterate over active processes to find
    while (pidx < n_act_procs){
        process * cur_proc = vector_get(active_procs, pidx);
        if (cur_proc->pid == cpid){ //this is the pid in question: either kill or stop it.
            if (strcmp(cmd, "cont") == 0){ // you cannot continue an already active process: means that process is technically not found in stopped vector
                print_no_process_found(cpid);
                return 1;
            }
            else if (strcmp(cmd, "kill") == 0){ //if it is a kill command:
                if (kill(cpid, SIGTERM) == -1){ // try to kill
                    printf("***FOUND PROCESS BUT CANNOT KILL\n"); //REMOVE
                    print_no_process_found(cpid);
                    return 1;
                }
                print_killed_process(cpid, cur_proc->command);
                vector_erase(active_procs, pidx);
                return 0;
            }
            else{ // is a stop command;
                if (kill(cpid, SIGSTOP) == -1){ // try to kill
                    printf("***FOUND PROCESS BUT CANNOT STOP\n"); //REMOVE
                    print_no_process_found(cpid);
                    return 1;
                }
                print_stopped_process(cpid, cur_proc->command);
                process * cur_proc_cpy = process_copy_constructor(cur_proc);
                vector_erase(active_procs, pidx);
                vector_push_back(stopped_procs, cur_proc_cpy);
                return 0;
            }
        }
        pidx ++;
    }
    //iterate over stopped processes to find: Reaching here means we have already checked all active processes
    pidx = 0; //reset pidx
    while (pidx < n_stopped_procs){
        process * cur_proc = vector_get(stopped_procs, pidx);
        if (cur_proc->pid == cpid){ //this is the pid in question: either kill or continue it.
            if (strcmp(cmd, "stop") == 0){ // you cannot stop an already stopped process: means that process is technically not found in active vector
                print_no_process_found(cpid);
                return 1;
            }
            else if (strcmp(cmd, "kill") == 0){ //if it is a kill command:
                if (kill(cpid, SIGTERM) == -1){ // try to kill
                    printf("***FOUND PROCESS BUT CANNOT KILL\n"); //REMOVE
                    print_no_process_found(cpid);
                    return 1;
                }
                print_killed_process(cpid, cur_proc->command);
                vector_erase(stopped_procs, pidx);
                return 0;
            }
            else{ // is a cont command;
                if (kill(cpid, SIGCONT) == -1){ // try to kill
                    printf("***FOUND PROCESS BUT CANNOT CONTINUE\n"); //REMOVE
                    print_no_process_found(cpid);
                    return 1;
                }
                print_continued_process(cpid, cur_proc->command);
                process * cur_proc_cpy = process_copy_constructor(cur_proc);
                vector_erase(stopped_procs, pidx);
                vector_push_back(active_procs, cur_proc_cpy);
                return 0;
            }
        }
        pidx ++;
    }
    //Reaching here means that the process was found in neither vectors
    print_no_process_found(pidx);
    return 1;
}
//goes through all active processes and prints their info
int ps_handler(vector* procs){
    //loop through child process to check for finished processes and removes them from active_procs:
    size_t pidx = 0;
    size_t proc_vec_size = vector_size(procs); 
    print_process_info_header();
    while (pidx < proc_vec_size){
        // printf("**idx: %zu\n", pidx);//REMOVE: TODO: THIS CAUSES PROBLEMS FOR SOME REASON
        process * cur_cproc = vector_get(procs, pidx); //current child process
        pid_t cur_cpid = cur_cproc->pid;
        // Open the status file of the current process
        char cur_stat_path[128];
        snprintf(cur_stat_path, sizeof(cur_stat_path), "/proc/%d/stat", cur_cpid); //write status path based on curr child process pid
        FILE* cur_status_file = fopen(cur_stat_path, "r");
        if (cur_status_file == NULL || kill(cur_cpid, 0) == -1){ //null status file means the process has died -> remove from vector
            vector_erase(procs, pidx); //after erasing, no need to increment idx
            proc_vec_size --; 
        }
        else{ // There is a valid stat file for this pid
            // printf("**CURRENTLY EVALUATING: %d\n", cur_cpid); // REMOVE
            process_info* cur_act_proc = calloc(1, sizeof(process_info)); // initialize the proccess info struct
            //read in pid and command first
            cur_act_proc->pid = cur_cproc->pid;
            char * cmdcpy = malloc(strlen(cur_cproc->command) + 1);// add one for null char
            strcpy(cmdcpy, cur_cproc->command);
            cur_act_proc->command = cmdcpy;
            // Read the contents of the process status file
            char *cline = NULL; //need to free
            size_t in_size = MAX_IN;
            if (getline(&cline, &in_size, cur_status_file) >= 0){
                if (cline[strlen(cline)-1] == '\n'){ //strip off \n if it exists
                    char * std_temp = getSubString(cline, 0, strlen(cline)-1); //strip off the '\n'
                    free(cline);
                    cline = std_temp;
                    std_temp = NULL;
                }
                // printf("Current stat info: %s\n", cline); REMOVE
                vector * stat_vec = str_to_vector(cline); //vector of attributes from /proc/[pid]/stat file. Need to free
                cur_act_proc->nthreads = strtol((char *)vector_get(stat_vec, 19), NULL, 10);
                cur_act_proc->state = ((char *)vector_get(stat_vec, 2))[0];
                cur_act_proc->vsize = strtoul((char *)vector_get(stat_vec, 22), NULL, 10);
                //get execution time
                unsigned long utime_clkt = strtoul((char *)vector_get(stat_vec, 13), NULL, 10);
                unsigned long stime_clkt = strtoul((char *)vector_get(stat_vec, 14), NULL, 10);
                unsigned long exec_time_s = (utime_clkt + stime_clkt)/sysconf(_SC_CLK_TCK); //execution time in seconds
                cur_act_proc->time_str = malloc(32); // string of MM:SS
                execution_time_to_string(cur_act_proc->time_str, 32, (exec_time_s/60), (exec_time_s%60));
                // print_process_info(cur_act_proc); //REMOVE
                //get boot time then start time
                FILE * stat_file = NULL;
                stat_file = fopen("/proc/stat", "r"); //open proc/stat file
                char * cur_line = NULL; //needs to free
                size_t bootsize = MAX_IN;
                while (getline(&cur_line, &bootsize, stat_file) >= 0){
                    if (strncmp(cur_line, "btime", 5) == 0){ //the current line contains btime
                        if (cur_line[strlen(cur_line)-1] == '\n'){ //remove nextline character
                            char * std_temp = getSubString(cur_line, 0, strlen(cur_line)-1); //strip off the '\n'
                            if (cur_line) free(cur_line);
                            cur_line = std_temp;
                            std_temp = NULL;
                        }
                        break;
                    }
                    free(cur_line);
                    cur_line = NULL;
                }
                if (stat_file) fclose(stat_file); //close proc/stat file
                char * boot_tm_str = strdup(cur_line+6);//only copy the boot time, not the "btime " : needs to free
                // printf("*** BOOT TIME STR: %s\n", boot_tm_str);
                unsigned long boot_tm = strtoul(boot_tm_str, NULL, 10);
                if (cur_line) free(cur_line);
                free(boot_tm_str);
                unsigned long start_clkt = strtoul((char *)vector_get(stat_vec, 21), NULL, 10); //start time in clock ticks
                time_t starttime = (start_clkt/sysconf(_SC_CLK_TCK)) + boot_tm;

                struct tm tm_info;
                memcpy(&tm_info, localtime(&starttime), sizeof(struct tm));
                cur_act_proc->start_str = malloc(32);
                time_struct_to_string(cur_act_proc->start_str, 32, &tm_info);
            }
            // print process header and info
            print_process_info(cur_act_proc);
            //free the process info struct:
            if (cur_act_proc->time_str) free (cur_act_proc->time_str);
            if (cur_act_proc->start_str) free (cur_act_proc->start_str);
            if (cur_act_proc->command) free (cur_act_proc->command);
            if (cur_act_proc) free(cur_act_proc);
            if (cline) free(cline);
            pidx ++;
        }
        fclose(cur_status_file);
    }
    return 0;
}
//handles single external commands:
int command_handler(char * command, vector *active_procs, vector * stopped_procs){
    char * first_two_chars = getSubString(command, 0, 2); //need to free
    vector * cmd_segments = str_to_vector(command); //Need to free
    // Check for ps: print all active processes:
    if (strcmp(first_two_chars, "ps") == 0){
        if (strlen(command) > 2){
            print_invalid_command(command);
            if (first_two_chars) free(first_two_chars);
            if (cmd_segments) vector_destroy(cmd_segments);
            return 1;
        }
        int ps_stat = ps_handler(active_procs);
        free(first_two_chars);
        vector_destroy(cmd_segments);
        return ps_stat;
    }
    //Check if command is a change path command (from !<prefix> or #<n> command executions that skip the original path change handler)
    if (strcmp(first_two_chars, "cd") == 0){
        if (vector_size(cmd_segments) != 2){ //check that the format is cd <path> and has 2 words
            print_invalid_command(command);
            vector_destroy(cmd_segments);
            free(first_two_chars);
            return 1; // return one for invalid command
        }
        //if command is a valid change path command, call path_change_handler
        int path_change_stat = path_change_handler(cmd_segments);
        vector_destroy(cmd_segments);
        free(first_two_chars);
        return path_change_stat;
    }
    //Check for redirection command: returns null if there is no redirection operator
    char ** op_arr = redir_handler(cmd_segments); //Need to free each string in array
    FILE *fptr = NULL; //redirection file
    bool redir_out = false; //indicates if output is written to file if true, or stdout if false.
    bool redir_in = false; //indicates if we should take command from a input file.
    char *file_cmd_in = NULL;
    if (op_arr){ //operator exists
        command = op_arr[1]; // reset command
        free(cmd_segments); //free the current cmd segment
        cmd_segments = str_to_vector(command); // redefine cmd segment
        if (strcmp(op_arr[0], "0") == 0){ // operator is: >
            if (!(fptr = fopen(op_arr[2], "w"))){
                print_redirection_file_error();
                vector_destroy(cmd_segments);
                free(first_two_chars);
                for (int i = 0; i < 3; i++){
                    if (op_arr[i]) free(op_arr[i]);
                }
                free(op_arr);
                return 1;
            }
            redir_out = true;
        }
        else if (strcmp(op_arr[0], "1") == 0){ // operator is : >>
            if (!(fptr = fopen(op_arr[2], "a"))){
                print_redirection_file_error();
                vector_destroy(cmd_segments);
                free(first_two_chars);
                for (int i = 0; i < 3; i++){
                    if (op_arr[i]) free(op_arr[i]);
                }
                free(op_arr);
                return 1;
            }
            redir_out = true;
        }
        else{ // operator is: <
            if (!(fptr = fopen(op_arr[2], "r"))){
                print_redirection_file_error();
                vector_destroy(cmd_segments);
                free(first_two_chars);
                for (int i = 0; i < 3; i++){
                    if (op_arr[i]) free(op_arr[i]);
                }
                free(op_arr);
                return 1;
            }
            size_t f_insize = MAX_IN;
            // read new command input file info into a string, and strip of newline character
            if (getline(&file_cmd_in, &f_insize, fptr) >= 0){
                if (file_cmd_in[strlen(file_cmd_in)-1] == '\n'){
                    char * std_temp = getSubString(file_cmd_in, 0, strlen(file_cmd_in)-1); //strip off the '\n'
                    free(file_cmd_in);
                    file_cmd_in = std_temp;
                    std_temp = NULL;
                }
            }
            strcat(command, " ");
            strcat(command, file_cmd_in); //add command arguments to command string 
            // printf("**NEW COMMAND: %s\n", command);
            if (file_cmd_in) free(file_cmd_in); //TODO: check getline heap allocs this
            free(cmd_segments); //free the current cmd segment
            cmd_segments = str_to_vector(command); // redefine cmd segment
            redir_in = true;
        }
    }
    if (op_arr){
        for (int i = 0; i < 3; i++){
            if (op_arr[i]) free(op_arr[i]);
        }
        free(op_arr);
    }
    //check for Signal commands: kill, stop, continue
    if (strcmp(vector_get(cmd_segments, 0), "kill") == 0 || strcmp(vector_get(cmd_segments, 0), "stop") == 0 || strcmp(vector_get(cmd_segments, 0), "cont") == 0){
        int sig_res = sig_cmd_handler(cmd_segments, active_procs, stopped_procs);
        free(cmd_segments);
        free(first_two_chars);
        if (fptr) fclose(fptr);
        return sig_res;
    }
    //For remaining external commands:
    //1. Check if this is to be run as a background command:
    bool hasBG = false;
    if (command[strlen(command) - 1] == '&'){
        hasBG = true;
        vector_pop_back(cmd_segments);// remove & from command segments
    }
    pid_t child_id = fork();
    if (child_id > 0){ // this is parent
        int status;
        if (!hasBG){ //if this is a foreground child process: No need to add to process vector, as it ends before parent continues
            waitpid(child_id, &status, 0);
        }
        else{
            waitpid(child_id, &status, WNOHANG); // WNOHANG does not stop the program from running in a child process.
            process * new_backgrd = malloc(sizeof(process)); //need to free
            //TODO: Check if we want to keep suffix &
            new_backgrd->command = getSubString(command, 0, (strlen(command)-2));// Remove ending " &" //getSubString already allocates command on heap. up to process deconstructor to free
            new_backgrd->pid = child_id;
            vector_push_back(active_procs, new_backgrd);
            free(new_backgrd);
        }
        if (WEXITSTATUS(status)) {
            if (cmd_segments) vector_destroy(cmd_segments);
            if (first_two_chars) free(first_two_chars);
            if (fptr) fclose(fptr);
            return 1;
        }
    }
    else if (! child_id){ //this is child
        pid_t child_id = getpid();
        if (hasBG){// SET PGID: if this is a background process, set process group id to be its own group
            if (setpgid(child_id, child_id) < 0){
                print_setpgid_failed();
                exit(1);
            }
        }
        if (redir_out){ //if we need to redirect output to a file:
            int out_fd = fileno(fptr);
            dup2(out_fd, STDOUT_FILENO);
            fclose(fptr);
        }
        print_command_executed(child_id); //external command successfully executed
        char ** args = vec_to_strArr(cmd_segments); //Need to free
        execvp(args[0], &args[0]);
        print_exec_failed(command);
        free(args); //each individual char * in args is freed when we destroy cmd_segments
        exit(1);
    }
    else{
        print_fork_failed();
        vector_destroy(cmd_segments);
        free(first_two_chars);
        fclose(fptr);
        return 1; 
    }
    if (cmd_segments) vector_destroy(cmd_segments);
    if (first_two_chars) free(first_two_chars);
    if (fptr) fclose(fptr);
    return 0;
    
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
    //part 2: active and stopped process vectors
    vector *active_procs = vector_create(process_copy_constructor, process_destructor, process_default_constructor);
    vector *stopped_procs = vector_create(process_copy_constructor, process_destructor, process_default_constructor);
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
                    vector_destroy(active_procs);
                    vector_destroy(stopped_procs);
                    return 1;
                }
            }
            else if(strcmp(argv[i], "-f") == 0 && (i < argc)){
                hasScript = true;
                script_file = argv[i+1];
                scrpt_rdr = file_handler(script_file, 1); //need to close
                if (!scrpt_rdr){ // there was a print script file error
                    vector_destroy(cmd_hist);
                    vector_destroy(active_procs);
                    vector_destroy(stopped_procs);
                    return 1;
                }
            }
            else{
                print_usage(); //all other situations, the command line arguments are not passed correctly
                vector_destroy(cmd_hist);
                vector_destroy(active_procs);
                vector_destroy(stopped_procs);
                return 1;
            }
            i++;
        }
    }
    // Run a loop constantly checking for std/ file input
    while(1){        
        // handle SIGINT
        signal(SIGINT, sig_handler);
        //**PROMPTING**
        //Prompt user with current pid and path
        pid = getpid();
        size_t path_size = MAX_PATH * sizeof(char);
        path = malloc(path_size);
        getcwd(path, 256);
        if (!hasScript) print_prompt(path, pid); // do not print if file input
        //**PROCESS CHECKING**
        //loop through child process to check for finished processes and removes them from active_procs (stopped processes cannot die)
        size_t pidx = 0;
        size_t proc_vec_size = vector_size(active_procs); 
        while (pidx < proc_vec_size){
            process * cur_cproc = vector_get(active_procs, pidx); //current child process
            pid_t cur_cpid = cur_cproc->pid;
            // printf("***Currently checking Process [%d], kill process returns: %d\n", cur_cpid, kill(cur_cpid, 0)); REMOVE
            // Open the status file of the current process
            char cur_stat_path[128];
            snprintf(cur_stat_path, sizeof(cur_stat_path), "/proc/%d/status", cur_cpid); //write status path based on curr child process pid
            FILE* cur_status_file = fopen(cur_stat_path, "r");
            if (cur_status_file == NULL || kill(cur_cpid, 0) == -1){ //null status file means the process has died -> remove from vector
                printf("Process [%d] has expired\n", cur_cpid);
                vector_erase(active_procs, pidx); //after erasing, no need to increment idx
                proc_vec_size --; 
            }
            else{ //vector is still active
                pidx ++;
            }
            fclose(cur_status_file);
        }
        //**INPUT SOURCE CHOOSING**
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
                eof_handler(active_procs, stopped_procs);
                //free all resources up to this point. close files
                if (input) free(input);
                if (path) free(path);
                vector_destroy(cmd_hist);
                vector_destroy(active_procs);
                vector_destroy(stopped_procs);
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
                eof_handler(active_procs, stopped_procs);
                //free all resources up to this point. close files
                if (input) free(input);
                if (path) free(path);
                vector_destroy(cmd_hist);
                vector_destroy(active_procs);
                vector_destroy(stopped_procs);
                if (hist_wtr) fclose(hist_wtr);
                if (scrpt_rdr) fclose(scrpt_rdr);
                printf("\n");
                return 1;
            }
        }
        //**HISTORY WRITING**
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
        //**COMMAND EXECUTION**
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
                eof_handler(active_procs, stopped_procs); //terminate all child processes
                //free all resources up to this point. close files: Make sure to free vector array
                if (cur_cmd_str) free(cur_cmd_str);
                cur_cmd_str = NULL;
                if (input) free(input);
                input = NULL;
                if (path) free(path);
                path = NULL;
                vector_destroy(cmd_hist);
                vector_destroy(active_procs);
                vector_destroy(stopped_procs);
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
                            sec_exit_stat = command_handler(sec_cur_cmd_str, active_procs, stopped_procs);
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
                            sec_exit_stat = command_handler(sec_cur_cmd_str, active_procs, stopped_procs);
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
                exit_status = command_handler(cur_cmd_str, active_procs, stopped_procs);
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
    vector_destroy(cmd_hist);
    vector_destroy(active_procs);
    vector_destroy(stopped_procs);
    return 0;
}
