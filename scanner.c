// scanner.c
// kpadron.github@gmail.com
// Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

// #include "scanner.h"
#include "hash.h"

// SCAN FILE OR DIRECTORY AND SAVE METADATA
// * Use a hash table with chaining
// * Each entry stores filename, filesize, filetime, filehash
// * Maybe branch into differential backup service

double wtime(void);
uint32_t rand32(void);
uint64_t rand64(void);

void file_recursive(char* filepath);
char* stracat(char* left, char* right);

int main(int argc, char** argv)
{
    char* filepath = NULL;

    if (argc < 2)
    {
        printf("Usage: scanner <filepath> ...\n");
    }

    if (argc > 1)
    {
        filepath = strdup(argv[1]);
    }

    file_recursive(filepath);

    return 0;
}


void file_recursive(char* filepath)
{
    struct stat filestat;

    if (stat(filepath, &filestat) < 0)
    {
        perror("stat");
        return;
    }

    if (S_ISREG(filestat.st_mode))
    {
        printf("%s: %zu\n", filepath, filestat.st_size);
    }
    else if (S_ISDIR(filestat.st_mode))
    {
        DIR* dir = opendir(filepath);

        struct dirent* dp;
        while ((dp = readdir(dir)) != NULL)
        {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

            char* newpath = strdup(filepath);

            if (newpath[strlen(newpath) - 1] != '/')
                newpath = stracat(newpath, "/");

            newpath = stracat(newpath, dp->d_name);

            file_recursive(newpath);
            free(newpath);
        }

        closedir(dir);
    }
}


char* stracat(char* left, char* right)
{
    left = (char*) realloc(left, strlen(left) + strlen(right) + 1);
    return strcat(left, right);
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
