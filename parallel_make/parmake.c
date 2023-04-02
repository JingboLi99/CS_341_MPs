/**
 * parallel_make
 * CS 341 - Spring 2023
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"
#include "vector.h"
#include "queue.h"
#include "compare.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

//**HELPER FUNCTIONS
//DFS function used by cycle checker: Return true if cycle is detected, false otherwise
bool dfs(graph * dgraph, char * node, set * visited, set * stack){
    set_add(visited, node); // this current node has been visited
    set_add(stack, node); //this node is in the path we are checking
    vector * node_nbrs = graph_neighbors(dgraph, node); //need to free vector, but not keys
    size_t n_nbrs = vector_size(node_nbrs); //number of neighbours of this current node
    //check if neighbours has already been visited ie circular dependency
    for (size_t n = 0; n < n_nbrs; n++){
        char * cur_nbr = vector_get(node_nbrs, n);
        if (set_contains(visited, cur_nbr) == false){
            bool nbr_has_cycle = dfs(dgraph, cur_nbr, visited, stack);
            if (nbr_has_cycle == true){
                free(node_nbrs); //free the vector without freeing the elements
                return true;
            } 
        }
        else if (set_contains(stack, cur_nbr) == true){
            free(node_nbrs); //free the vector without freeing the elements
            return true;
        }
    }
    set_remove(stack, node); // the set makes a deep copy, so we can remove it
    free(node_nbrs); //free the vector without freeing the elements
    return false;
}
//check if there is a cycle for this target, or any of the dependencies it has
bool checkCycle(graph * dgraph, char * tar){
    set * visited = set_create(string_hash_function, string_compare, 
                               string_copy_constructor, string_destructor); //need to destroy
    set * stack = set_create(string_hash_function, string_compare, 
                               string_copy_constructor, string_destructor); //need to destroy
    bool has_cycle = dfs(dgraph, tar, visited, stack);
    set_destroy(visited);
    set_destroy(stack);
    return has_cycle;
}   
//Check if given rule is the name of a file on disk that the current process has permission to access
bool checkIsFile(rule_t * cRule){
    char * rule_name = cRule->target;
    if (access(rule_name, F_OK) == 0){
        return true;
    }
    return false;
}
//Recursive program to run a rule
bool runRule(graph * dgraph, char * tar){
    //check if the current rule has been run before:
    //NOTE: This is indicated by the rule's state -> 0: Not run, 1: Satisfied, 2: Failed
    rule_t * rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) tar);
    if (rule->state == 1){ //if the current rule is already satisfied
        return true;
    }
    else if (rule->state == 2){ //if current rule has failed (either it failed or its dependencies failed)
        return false;
    }
    //Current rule has NOT been run before
    //get dependencies:
    vector* dependencies = graph_neighbors(dgraph, tar); //free vector, not elements
    size_t n_dep = vector_size(dependencies);
    for (size_t i = 0; i < n_dep; i++){ // run the previous dependencies first
        char * cur_dep_tar = vector_get(dependencies, i);
        //run current dependency, returns true if depedency is satisfied, false if it failed
        bool dep_success = runRule(dgraph, cur_dep_tar); 
        if (dep_success == false){
            //Mark the current rule as has failed and dont run its commands
            rule->state = 2;
            free(dependencies);
            return false;
        }
        //if current dependency succeeds, run remaining dependencies
    }
    //*Reaching here means all dependencies have been successfully run: run based on rule flowchart
    // /https://cs341.cs.illinois.edu/images/assignment-docs/mp/parallel_make/parmake_flowchart.svg
    bool run_rule_cmds = true;
    if (checkIsFile(rule)){ //Check if the rule is a file on disk
        //this vector stores the *INDEX* of the dependencies that are file dependencies
        vector * file_dependencies = vector_create(int_copy_constructor, int_destructor, int_default_constructor); //Need to free
        bool has_nonfile_dep = false;
        for (size_t j = 0; j < n_dep; j++){ 
            char * dep_tar = vector_get(dependencies, j);
            rule_t * dep_rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) dep_tar);
            if (checkIsFile(dep_rule) == true){
                vector_push_back(file_dependencies, (void *) j); //CHECK if stack int is ok
            }
            else{ // there are non file dependencies -> run the rule's commands
                has_nonfile_dep = true;
            }
        }
        //Check if there are no non file dependencies, check modification time:
        if (!has_nonfile_dep){ //no has_nonfile_dep means either 0 dependencies or all dependencies are filenames
            bool dep_modded_later = false; //True if there exists >1 dependency that is modified after the current file
            struct stat cur_stat;
            if (stat(tar, &cur_stat) == -1){
                printf("**ERROR: Unable to read current file rule's stat file\n");
            }
            time_t cur_mtime = cur_stat.st_mtime;
            size_t n_file_deps = vector_size(file_dependencies);
            for (size_t i = 0; i < n_file_deps; i++){
                int dep_idx = *(int *)vector_get(file_dependencies, i);
                char * cur_dep_file = vector_get(dependencies, dep_idx);
                struct stat cur_dep_stat;
                if (stat(cur_dep_file, &cur_dep_stat) == -1){
                    printf("**ERROR: Unable to read dependency file rule's stat file\n");
                }
                time_t dep_mtime = cur_dep_stat.st_mtime;
                double time_diff = difftime(dep_mtime, cur_mtime);
                if (time_diff > 0.0){ // current dependency file is modified later than cur file -> need to run current files commands
                    dep_modded_later = true;
                    break; //We only need 1 file dep to be modded later to need to run the current rule
                }
            }
            if(!dep_modded_later) run_rule_cmds = false; // Only if none of the 3 conditions are true, then we do not need to run the rule's cmds.
        }
        vector_destroy(file_dependencies);
    }
    free(dependencies); //free dependencies vector
    //Check if we need to run the current rule's commands:
    if (!run_rule_cmds){ //do not need to run: mark rule as satisfied
        rule->state = 1;
        return true;
    }
    else{ // Run rule commands one by one
        size_t n_crule_cmds = vector_size(rule->commands);
        for (size_t i = 0; i < n_crule_cmds; i++){
            char * cur_cmd = vector_get(rule->commands, i);
            int cmd_stat = system(cur_cmd); //Run the command
            if (cmd_stat != 0){ //System failed to execute command
                //Either unable to start child process, or child shell failed to execute command
                rule->state = 2;
                return false;
            }
        } 
        rule->state = 1;
        return true;
    }
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck
    //TODO: NOTE!! NEED TO FREE EVERY SINGLE VECTOR RETURNED BY DGRAPH!
    graph * dgraph = parser_parse_makefile(makefile, targets); // Create dependency graph
    vector * goals = graph_neighbors(dgraph, ""); //list of "goal" targets that we need to evaluate for //Need to free vector, but not they keys in the vectors
    size_t ngoals = vector_size(goals); //number of "goal" targets
    //Evaluate for each goal target: 1. Is it good? 2. If its good, sort it and run from tail upwards
    for (size_t i = 0; i < ngoals; i++){
        char * cur_goal = vector_get(goals, i);
        //check if there is a cycle for the current goal 
        bool has_cycle = checkCycle(dgraph, cur_goal);
        if (has_cycle){
            print_cycle_failure(cur_goal);
            //set the rule of current goal target to fail
            rule_t * cur_grule = (rule_t *) graph_get_vertex_value(dgraph, (void *) cur_goal);
            cur_grule->state = 2;
            //do not return, as we need to check for all goals
        }
        else{ //no cycle, run program from the current goal, indicating its run status in the dictionary
            runRule(dgraph, cur_goal);
        }
    }
    //free graph and goals vector
    graph_destroy(dgraph);
    free(goals);
    return 0;
}
