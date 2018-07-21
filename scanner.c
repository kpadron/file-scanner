// scanner.c
// kpadron.github@gmail.com
// Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "scanner.h"
#include "hash.h"

// SCAN FILE OR DIRECTORY AND SAVE METADATA
// * Use a hash table with chaining
// * Each entry stores filename, filesize, filetime, filehash
// * Maybe branch into differential backup service

double wtime(void);
uint32_t rand32(void);
uint64_t rand64(void);

int main(int argc, char** argv)
{
    char* tests[] = { "hash_insert", "hash_search", "hash_remove" };
    if (argc < 2)
    {
        printf("Usage: scanner <filepath> ...\n");
    }

    hash_t table;

    hash_init(&table, 10);


    for (size_t i = 0; i < 3; i++)
    {
        double test_duration = 5;
        double test_start = 0;
        double test_time = 0;
        size_t test_cycles = 0;

        do
        {
            uint32_t k = rand32();

            test_start = wtime();
            if (!strcmp(tests[i], "hash_insert"))
            {
                hash_insert(&table, k, NULL);
            }
            else if (!strcmp(tests[i], "hash_search"))
            {
                hash_search(&table, k);
            }
            else if (!strcmp(tests[i], "hash_remove"))
            {
                hash_remove(&table, k);
            }
            test_time += wtime() - test_start;

            test_cycles++;

        } while (test_time < test_duration);

        printf("%s: %zu iterations over %.2f s -> %.4f ns per operation\n", tests[i], test_cycles, test_time, test_time * 1E9 / test_cycles);
    }

    hash_free(&table);

    return 0;
}


// walltime of the computer in seconds (useful for performance analysis)
double wtime(void)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1E9;
}


// Return 32-bit random number
uint32_t rand32(void)
{
    return (rand() ^ (rand() << 15) ^ (rand() << 30));
}


// Return 64-bit random number
uint64_t rand64(void)
{
    return (((uint64_t) rand32()) << 32) | rand32();
}
