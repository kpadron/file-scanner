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

#define SCANNER_LOGNAME ".fslog"
#define SCANNER_DIRNAME ".fsdir"

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

    char* logpath = (char*) malloc(128);
    logpath = stracat(logpath, SCANNER_LOGNAME);

    stackarray_t old_stack;
    stackarray_t new_stack;
    stackarray_t diff_stack;

    stack_init(&old_stack);
    stack_init(&new_stack);
    stack_init(&diff_stack);

    // Lookup old file info
    filelog_read(logpath, &old_stack);

    // Lookup new file info
    for (int i = 1; i < argc; i++)
    {
        filetree_parse(argv[i], &new_stack);
    }

    // Compare diff of new and old
    fileinfo_diff(&old_stack, &new_stack, &diff_stack);

    // Write records to logfile
    filelog_write(logpath, &diff_stack);

    exit(0);
}
