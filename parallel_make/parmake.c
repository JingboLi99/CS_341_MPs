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
#include <pthread.h>
//**GLOBALS:
static queue * ruleq = NULL;
//**THREAD STARTING FUNCTION
void * ruleExec(void * args){
    char * ctar = NULL;
    graph * dgraph = (graph *) args;
    while ((ctar = (char *) queue_pull((queue *) ruleq)) != NULL){
        rule_t * crule = (rule_t *) graph_get_vertex_value(dgraph, (void *) ctar);
        size_t n_crule_cmds = vector_size(crule->commands);
        bool hasFailed = false;
        for (size_t i = 0; i < n_crule_cmds; i++){
            char * cur_cmd = vector_get(crule->commands, i);
            int cmd_stat = system(cur_cmd); //Run the command
            if (cmd_stat != 0){ //System failed to execute command
                //Either unable to start child process, or child shell failed to execute command
                crule->state = 2;
                hasFailed = true;
                break;
            }
        } 
        if (! hasFailed) crule->state = 1;
        if (ctar) free(ctar);
    }
    
    return NULL;
}
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
                vector_destroy(node_nbrs); //free the vector without freeing the elements
                return true;
            } 
        }
        else if (set_contains(stack, cur_nbr) == true){
            vector_destroy(node_nbrs); //free the vector without freeing the elements
            return true;
        }
    }
    set_remove(stack, node); // the set makes a deep copy, so we can remove it
    vector_destroy(node_nbrs); //free the vector without freeing the elements
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
// Pushes current rule and all non satisfied dependencies of current rule
// into th vector in recursive fashion
void pushDepToVec(graph * dgraph, vector * rule_vec, set * vec_set, char * tar){
    rule_t * c_rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) tar);
     //if current rule is already in the vector or is already run, we do not need to check it anymore
    if (set_contains(vec_set, tar) || c_rule->state != 0){
        return;
    }
    //Reaching here means the current rule is not satisfied:
        // We push all its dependnecies and then push itself
    vector* dependencies = graph_neighbors(dgraph, tar); //free vector
    size_t n_dep = vector_size(dependencies);
    for (size_t i = 0; i < n_dep; i++){
        char * dep_tar = vector_get(dependencies, i);
        rule_t * dep_rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) dep_tar);
        // if the dependency has not been run and has not been included into vector
        if (!set_contains(vec_set, tar) && dep_rule->state == 0){ 
            pushDepToVec(dgraph, rule_vec, vec_set, dep_tar);
        }   
    }
    vector_push_back(rule_vec, tar);
    set_add(vec_set, tar);
    vector_destroy(dependencies);
}
// isAvailable checks if the current target is available for execution
//      ie. 1. Do not need to be executed (satisfed or failed)
//          2. all dependencies satisfied
//          3. Needs to be satisfied according to flowchart conditions
// Returns: -1 -> no need to executed (cond 1 or 3) | 0 -> available | 1 -> not available (dependencies not met)
int isAvailable(graph * dgraph, char * tar){
    //check if the current rule has been run before:
    //NOTE: This is indicated by the rule's state -> 0: Not run, 1: Satisfied, 2: Failed
    rule_t * rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) tar);
    if (rule->state == 1 || rule->state == 2){ //if the current rule is already satisfied or failed
        return -1;
    }
    //Current rule has NOT been run before
    //get dependencies:
    vector* dependencies = graph_neighbors(dgraph, tar); //free vector, not elements
    size_t n_dep = vector_size(dependencies);
    //check the rules immediate dependencies
    for (size_t i = 0; i < n_dep; i++){
        char * cur_dep_tar = vector_get(dependencies, i);
        rule_t * cdep_rule = (rule_t *) graph_get_vertex_value(dgraph, (void *) cur_dep_tar);
        if (cdep_rule->state == 2){ //if the cur dependency is failed, then the current rule will also fail
            rule->state = 2;
            vector_destroy(dependencies);
            return -1; // current rule has failed, no need to execute anymore
        }
        else if (cdep_rule->state == 0){ // dependency is not run -> cannot run the current rule
            vector_destroy(dependencies);
            return 1;
        }
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
                int * idx = malloc(sizeof(int));
                *idx = (int) j;
                vector_push_back(file_dependencies, (void *) idx); //CHECK if stack int is ok
                free(idx);
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
                fprintf(stdout, "**ERROR: Unable to read current file rule's stat file\n");
                fflush(stdout);
            }
            time_t cur_mtime = cur_stat.st_mtime;
            size_t n_file_deps = vector_size(file_dependencies);
            // fprintf(stdout, "**DEBUG: Rule <%s> reached here!\n", tar);
            for (size_t i = 0; i < n_file_deps; i++){
                int dep_idx = *(int *)vector_get(file_dependencies, i);
                // fprintf(stdout, "file dep idx: %d\n", dep_idx);
                char * cur_dep_file = vector_get(dependencies, dep_idx);
                struct stat cur_dep_stat;
                if (stat(cur_dep_file, &cur_dep_stat) == -1){
                    fprintf(stderr, "**ERROR: Unable to read dependency file rule's stat file\n");
                    fflush(stderr);
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
    vector_destroy(dependencies); //free dependencies vector
    //Check if we need to run the current rule's commands:
    if (!run_rule_cmds){ //do not need to run: mark rule as satisfied
        rule->state = 1;
        return -1;
    }
    return 0; //Rule is available to be executed
}
//run rule takes the goal rule, and makes sure it is executed (either satisfied or failed)
//It pushes all dependencies of the goal rule onto a vector, and continuously iterates through the vector,
// pushing rules with all dependencies satisfied onto the queue 
void runGoal(graph * dgraph, char * goal){
    vector * rule_vec = vector_create(string_copy_constructor, string_destructor, string_default_constructor); //need to free (destroy)
    //set is used to check if an element is already in the vector
    set * vec_set = set_create(string_hash_function, string_compare, string_copy_constructor, string_destructor); //need to free (destroy)
    pushDepToVec(dgraph, rule_vec, vec_set, goal); //add all dependencies to the vector
    set_destroy(vec_set);
    //keep iterating through vector and execute available rules. (available means the rule needs to be executed and all of its dependencies are satisfied)
    // printf("Evaluating goal: %s\n",goal);
    while(vector_size(rule_vec) > 0){
        // printf("Iterating through\n");
        for (size_t i = 0; i < vector_size(rule_vec); i++){
            char * ctar = (char *) vector_get(rule_vec, i);
            // printf("%s : ", ctar);
            int tar_stat = isAvailable(dgraph, ctar);
            if (tar_stat == -1){ //Already satisfied or failed
                vector_erase(rule_vec, i); //remove rule from vector
            }
            else if(tar_stat == 1){ //current rule is still not available: check again later
                continue;
            }
            else{ // current rule is available
                // printf("**Pushing to queue: %s\n", ctar);
                char * ctar_heap = malloc(32);
                strcpy(ctar_heap, ctar);
                queue_push(ruleq, (void *) ctar_heap);
                vector_erase(rule_vec, i);
                i--;
            }
        }
    }
    vector_destroy(rule_vec);
}
//**ENTRY
int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck
    //TODO: NOTE!! NEED TO FREE EVERY SINGLE VECTOR RETURNED BY DGRAPH!
    graph * dgraph = parser_parse_makefile(makefile, targets); // Create dependency graph
    vector * goals = graph_neighbors(dgraph, ""); //list of "goal" targets that we need to evaluate for //Need to free vector
    size_t ngoals = vector_size(goals); //number of "goal" targets
    //THREADS POOL AND QUEUE INITIALIZATION:
    ruleq = queue_create(-1);
    pthread_t threads[num_threads];
    for (size_t t = 0; t < num_threads; t++){
        pthread_create(&threads[t], NULL, ruleExec, (void *) dgraph); //Passing dependency graph as only argument
    }
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
            runGoal(dgraph, cur_goal);
        }
    }
    //Send ending signal into queue to tell the worker threads to stop polling
    //At this points, all goal rules have finish running: aka can close all threads
    for (size_t ti = 0; ti < num_threads + 1; ti ++){
        queue_push(ruleq, NULL);
    }
    //join threads
    for (size_t j = 0; j < num_threads; j++){
        pthread_join(threads[j], NULL);
    }
    //free graph and goals vector
    graph_destroy(dgraph);
    vector_destroy(goals);
    queue_destroy(ruleq);
    return 0;
}
