// @file scanner.h
// @email kpadron.github@gmail.com
// @author Kristian Padron
// @brief hash table module
#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "stack.h"

#define SCANNER_BLOCK_SIZE 1 << 20 // MB

// Structure to store file metadata
typedef struct
{
    char* name;     //< Name and path to file
    uint64_t size;  //< Size in bytes of file
    uint32_t mtime; //< Time of last modification
    uint32_t hash;  //< 32-bit hash of files contents
} fileinfo_t;


// @brief Recursively search through the file tree starting at <filepath>
//
// @param[in]  <filepath> - starting point to recursively search filetree
// @param[out] <stack>    - stack object to store fileinfo structures in
extern void filetree_parse(char* filepath, stackarray_t* stack);


// @brief Generate a 32-bit hash of the contents of <filepath>
//
// @param[in] <filepath> - path to file to hash
// @param[in] <length>   - size in bytes of file to hash
//
// @return 32-bit hash produced from <filepath> contents
extern uint32_t file_hash(char* filepath, size_t length);


// @brief strcat wrapper that ensures enough memory is allocated for the result
//
// @param[in,out] <left>  - string to store concatentation result
// @param[in]     <right> - string to be appended to result
//
// @return concatenation of <left> and <right> strings
extern char* stracat(char* left, char* right);
