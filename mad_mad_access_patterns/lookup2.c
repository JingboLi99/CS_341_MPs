/**
 * mad_mad_access_patterns
 * CS 341 - Spring 2023
 */
#include "tree.h"
#include "utils.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

/**
 * Recursive helper function
*/
int search(BinaryTreeNode* curr_root, char* keyword, char* addr) {
  if (strcmp(curr_root->word, keyword) == 0) {
    printFound(curr_root->word, curr_root->count, curr_root->price);
    return 1;
  }

  BinaryTreeNode* left_child = NULL;
  BinaryTreeNode* right_child = NULL;
  if (curr_root->left_child > 0) {
    left_child = (BinaryTreeNode*) (addr + curr_root->left_child);
  }
  if (curr_root->right_child > 0) {
    right_child = (BinaryTreeNode*) (addr + curr_root->right_child);
  }

  if (left_child != NULL && strcmp(keyword, curr_root->word) < 0) {
    return search(left_child, keyword, addr);
  } else if (right_child != NULL && strcmp(keyword, curr_root->word) > 0) {
    return search(right_child, keyword, addr);
  }

  // If we get here, then we didn't find it (I think?)
  printNotFound(keyword);
  return 0;
}

int main(int argc, char **argv) {

    if (argc < 3) {
      printArgumentUsage();
      exit(1);
    }

    char* file_name = argv[1];

    // mmap
    struct stat sb;
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
      openFail(file_name);
      exit(2);
    }
    fstat(fd, &sb);

    char* addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
      mmapFail(file_name);
      exit(3);
    }

    // Read first 4 bytes
    char expected[4] = {'B', 'T', 'R', 'E'};
    if (memcmp(addr, expected, 4) != 0) {
      openFail(file_name);
      exit(2);
    }

    // Get pointer to root
    BinaryTreeNode* root = (BinaryTreeNode*) (addr + 4);

    // Iterate through all provided words
    for (int i = 2; i < argc; ++i) {
      // Print the info of the word argv[i]
      char* curr_word = argv[i];
      search(root, curr_word, addr);
    }

    // munmap
    if (munmap(addr, sb.st_size) == -1) {
      mmapFail(file_name);
      exit(3);
    }

    close(fd);

    return 0;
}