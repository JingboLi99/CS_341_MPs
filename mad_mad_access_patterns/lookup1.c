/**
 * mad_mad_access_patterns
 * CS 341 - Spring 2023
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define btn BinaryTreeNode
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/
bool rec_find(FILE * tf, size_t offset, char * word){
  fseek(tf, offset, SEEK_SET);
  btn cnode; // cur node to check
  fread(&cnode, sizeof(btn), 1, tf);
  // get word length
  char c;
  int word_length = 0;
  while ((c = fgetc(tf)) != '\0' && c != EOF) word_length++;
  // get work
  char nodeword[word_length+1]; nodeword[word_length] = '\0';
  fseek(tf, offset + sizeof(btn), SEEK_SET);
  fread(nodeword, 1, word_length, tf);
  // printf("btn size: %zu, current node word: %s\n", sizeof(cnode), nodeword);
  // check if current node is word:
  int val = strcmp(word, nodeword);
  if (val ==0){ // current node contains word
    printFound(word, cnode.count, cnode.price);
    return true;
  } else if ( val < 0){ //left tree contains word
    if (cnode.left_child > 0){ //make sure left tree exists
      return rec_find(tf, cnode.left_child, word);
    }
  }else{ // right tree contains node
    if (cnode.right_child > 0){
      return rec_find(tf, cnode.right_child, word);
    }
  }
  return false;
}
int main(int argc, char **argv) {
  //Parse and check prerequisites
  if (argc < 3){ //take into account the program name as 1st arg
    printArgumentUsage();
    exit(1);
  }
  char * infilename = argv[1]; //input filename
  size_t nWords = argc-2; //number of word arguments
  FILE * tf = fopen(infilename, "r"); //open tree file (tf)
  if (!tf){ //check input file exists
    openFail(infilename);
    exit(2);
  }
  // check first 4 bytes are BTRE
  char firstFour[5];
  firstFour[4] = '\0';
  fread(firstFour, 1, 4, tf);
  if (strcmp(firstFour, "BTRE") != 0){
    formatFail(infilename);
    exit(2);
  }
  for (size_t i = 0; i < nWords; i++){
    bool found = rec_find(tf, BINTREE_ROOT_NODE_OFFSET, argv[i+2]);
    if (!found) printNotFound(argv[i+2]);
  }
  if (tf) fclose(tf);
  return 0;
}
