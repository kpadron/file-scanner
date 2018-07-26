// @file scanner.c
// @email kpadron.github@gmail.com
// @author Kristian Padron
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include "scanner.h"

/**
 * [_keycmp description]
 * @param  a [description]
 * @param  b [description]
 * @return   [description]
 */
static inline int _keycmp(void* a, void* b)
{
    return strcmp(((fileinfo_t*)a)->name, ((fileinfo_t*)b)->name);
}

/**
 * [_memdup description]
 * @param src  [description]
 * @param size [description]
 */
static inline void* _memdup(void* src, size_t size)
{
    return memcpy(malloc(size), src, size);
}

/**
 * [_murmur3 description]
 * @param  key    [description]
 * @param  length [description]
 * @param  seed   [description]
 * @return        [description]
 */
static inline uint32_t _murmur3(void* key, size_t length, uint32_t seed)
{
    uint8_t* key_x1 = (uint8_t*) key;
    uint32_t h = seed;

    if (length > 3)
    {
        uint32_t* key_x4 = (uint32_t*) key_x1;
        size_t i = length >> 2;

        do
        {
            uint32_t k = *key_x4++;
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = (h * 5) + 0xe6546b64;
        } while (--i);

        key_x1 = (uint8_t*) key_x4;
    }

    if (length & 3)
    {
        uint32_t k = 0;
        size_t i = length & 3;
        key_x1 = &key_x1[i - 1];

        do
        {
            k <<= 8;
            k |= *key_x1--;
        } while (--i);

        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }

    h ^= length;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}


void filelog_read(char* logpath, stackarray_t* stack)
{
    FILE* f = fopen(logpath, "r");
    if (!f)
    {
        char error[64];
        sprintf(error, "filelog read <%s>", logpath);
        perror(error);
        return;
    }

    fileinfo_t* info;
    char logstr[1024];

    for (size_t i = 0; fgets(logstr, 1024, f) != NULL; i++)
    {
        switch (i % 4)
        {
            case 0:
            info = (fileinfo_t*) malloc(sizeof(fileinfo_t));
            logstr[strlen(logstr) - 1] = '\0';
            info->name = strdup(logstr);
            break;

            case 1:
            sscanf(logstr, "%"PRIu64"\n", &info->size);
            break;

            case 2:
            sscanf(logstr, "%"PRIu32"\n", &info->mtime);
            break;

            default:
            sscanf(logstr, "%"PRIu32"\n", &info->hash);
            stack_push(stack, info);
            break;
        }
    }

    fclose(f);
}

void filelog_write(char* logpath, stackarray_t* stack)
{
    FILE* f = fopen(logpath, "w");
    if (!f)
    {
        char error[64];
        sprintf(error, "filelog write <%s>", logpath);
        perror(error);
        exit(1);
    }

    for (uint64_t i = 0; i < stack->count; i++)
    {
        fileinfo_t* info = (fileinfo_t*) stack->array[i];

        fprintf(f, "%s", info->name);
        fprintf(f, "%"PRIu64"", info->size);
        fprintf(f, "%"PRIu32"", info->mtime);
        fprintf(f, "%"PRIu32"", info->hash);
    }

    fclose(f);
}


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

        info->name = (char*) strdup(filepath);
        info->size = (uint64_t) filestat.st_size;
        info->mtime = (uint32_t) filestat.st_mtime;

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


uint32_t file_hash(char* filepath, size_t length)
{
    uint32_t hash = 0;

    int fd = open(filepath, O_RDONLY);
    void* buffer = malloc(SCANNER_BLOCK_SIZE);

    while (length)
    {
        size_t read_size = SCANNER_BLOCK_SIZE;
        if (length < read_size) read_size = length;

        read(fd, buffer, read_size);

        hash = _murmur3(buffer, read_size, hash);

        length -= read_size;
    }

    free(buffer);
    close(fd);

    return hash;
}


void fileinfo_diff(stackarray_t* old, stackarray_t* new, stackarray_t* diff)
{
    if (!old->count)
    {
        while (new->count)
        {
            fileinfo_t* new_info = (fileinfo_t*) stack_pop(new);
            new_info->hash = file_hash(new_info->name, new_info->size);
            stack_push(diff, new_info);
        }
    }
    else if (!new->count)
    {
        while (old->count)
        {
            stack_push(diff, stack_pop(old));
        }
    }
    else
    {
        while (new->count)
        {
            // Determine if new file is in log
            fileinfo_t* new_info = (fileinfo_t*) stack_pop(new);
            fileinfo_t* old_info = (fileinfo_t*) stack_remove(old, new_info, _keycmp);

            // Old file need to determine if versions are the same
            if (old_info && new_info->size == old_info->size && new_info->mtime == old_info->mtime)
            {
                // Add old info to diff stack
                stack_push(diff, old_info);
                fileinfo_free(new_info);
            }
            // New file to be recorded
            else
            {
                // Add new info to diff stack
                new_info->hash = file_hash(new_info->name, new_info->size);
                stack_push(diff, new_info);
                if (old_info) fileinfo_free(old_info);

                printf("Added %s\n", new_info->name);
                printf("\tsize: %zu, mtime: %zu, hash: %zu\n", (size_t) new_info->size, (size_t) new_info->mtime, (size_t) new_info->hash);
            }
        }

        // Add any old left-overs to diff stack
        while (old->count)
        {
            stack_push(diff, stack_pop(old));
        }
    }
}

void fileinfo_free(fileinfo_t* info)
{
    free(info->name);
    free(info);
}


char* stracat(char* left, char* right)
{
    left = (char*) realloc(left, strlen(left) + strlen(right) + 1);
    return strcat(left, right);
}
