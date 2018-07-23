// hash.h
// kpadron.github@gmail.com
// Kristian Padron
// hash table module
#pragma once
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_SIZE 64
#define GROWTH_FACTOR 1.6180339887
#define MAX_ALPHA 50

typedef void* hashkey_t; // key value to be used for search and comparison
typedef void* data_t;    // pointer to data to be stored in list

// Object representing a hash table entry
typedef struct
{
    hashkey_t key;
    data_t data;
} entry_t;

// Object representing a hash table bucket chain
typedef struct
{
    uint32_t count;
    uint32_t size;
    entry_t* chain;
} bucket_t;

// Object representing a hash table
typedef struct
{
    uint64_t entries;
    uint32_t size;
    bucket_t* buckets;

    size_t (*keysize)(const void* key);
    int (*keycmp)(const void* a, const void* b);
} hash_t;

// Initalize a hash table object
extern void hash_init(hash_t* table, uint32_t size, size_t (*keysize)(const void* key), int (*keycmp)(const void* a, const void* b));

// Cleanup and deallocate a hash table object
extern void hash_free(hash_t* table);

// Insert new entry into hash table using specified key O(1)
extern void hash_insert(hash_t* table, hashkey_t key, data_t data);

// Return data of the entry with specified key O(1)
extern data_t hash_search(hash_t* table, hashkey_t key);

// Remove entry with specified key returning data O(1)
extern data_t hash_remove(hash_t* table, hashkey_t key);

// Print table statistics
extern void hash_print_stats(hash_t* table);

// Debug print used to visualize hash table
extern void hash_print_debug(hash_t* table);
