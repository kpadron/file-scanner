// main.c
// kpadron.github@gmail.com
// Kristian Padron

#include <stdio.h>
#include <stdlib.h>

#include "hash.h" // Hash table library

// SCAN FILE OR DIRECTORY AND SAVE METADATA
// * Use a hash table with chaining
// * Each entry stores filename, filesize, filetime, filehash
// * Maybe branch into differential backup service

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: scanner <filepath> ...\n");
    }

    


    return 0;
}
