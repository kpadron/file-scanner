// main.c
// kpadron.github@gmail.com
// Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

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

    hash_t table;

    hash_init(&table, 10);

    for (uint32_t i = 0; i < 100000; i++)
    {
        system("clear");
        uint32_t k = rand()%1000;
        hash_insert(&table, k, NULL);
        hash_print(&table);
        usleep(0.25 * 1e6);
    }

    return 0;
}
