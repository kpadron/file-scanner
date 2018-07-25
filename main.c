// main.c
// kpadron.github@gmail.com
// Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "scanner.h"
#include "stack.h"

// SCAN FILE OR DIRECTORY AND SAVE METADATA
// * Use a hash table with chaining
// * Each entry stores filename, filesize, filetime, filehash
// * Maybe branch into differential backup service
// * Maybe use fork() or other multi-processing methods


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <filepath> ...\n", argv[0]);
        exit(1);
    }

    stackarray_t stack;
    stack_init(&stack);

    // Loop through all arguments
    for (int i = 1; i < argc; i++)
    {
        filetree_parse(argv[i], &stack);
    }

    // Loop through and generate hashes
    for (uint64_t i = 0; i < stack.count; i++)
    {
        fileinfo_t* info = (fileinfo_t*)stack.array[i];

        info->hash = file_hash(info->name, info->stat.st_size);
    }

    for (uint64_t i = 0; i < stack.count; i++)
    {
        fileinfo_t* info = (fileinfo_t*)stack.array[i];

        printf("%s:\n", info->name);
        printf("\t%zu\n", info->stat.st_size);
        printf("\t0x%zx\n", (size_t) info->hash);
    }

    exit(0);
}
