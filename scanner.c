// @file scanner.c
// @email kpadron.github@gmail.com
// @author Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include "scanner.h"


void filetree_parse(char* filepath, stackarray_t* stack)
{
    struct stat filestat;

    if (stat(filepath, &filestat) < 0)
    {
        perror("stat");
        return;
    }

    if (S_ISREG(filestat.st_mode))
    {
        fileinfo_t* info = (fileinfo_t*) malloc(sizeof(fileinfo_t));
        info->name = strdup(filepath);
        info->stat = filestat;
        stack_push(stack, info);
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

            filetree_parse(newpath, stack);
            free(newpath);
        }

        closedir(dir);
    }
}


uint64_t file_hash(char* filepath, size_t length)
{
    FILE* f = fopen(filepath, "rb");

    uint64_t h = 14695981039346656037ULL;

    for (size_t i = 0; i < length; i++)
    {
        uint8_t k;
        fread(&k, 1, 1, f);

        h = (h ^ k) * 1099511628211ULL;
    }

    fclose(f);

    return h;
}


char* stracat(char* left, char* right)
{
    left = (char*) realloc(left, strlen(left) + strlen(right) + 1);
    return strcat(left, right);
}
