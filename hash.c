// hash.c
// kpadron.github@gmail.com
// Kristian Padron
// implementation for hash table object
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hash.h"

#define BSEARCH 1


// Maps a number to the range [0, n] faster than modulo division
static inline uint32_t _map32(uint32_t x, uint32_t n)
{
    return ((uint64_t) x * (uint64_t) n) >> 32;
}


// Perform one-at-a-time hash on input bytes
static inline uint32_t _hash_oaat(void* key, size_t length)
{
    char* p = (char*) key;
    register uint32_t h = 0;

    for (size_t i = 0; i < length; i++)
    {
        h += p[i];
        h += h << 10;
        h ^= h >> 6;
    }

    h += h << 3;
    h ^= h >> 11;
    h += h << 15;

    return h;
}


// Iniatialize bucket object
static void _bucket_init(bucket_t* bucket)
{
    bucket->count = 0;
    bucket->size = 0;
    bucket->chain = NULL;
}


// Return index to the position of entry with specified key O(log N)
static uint32_t _bucket_index_bsearch(bucket_t* bucket, hashkey_t key)
{
    entry_t* sorted = bucket->chain;

    uint32_t l = 0;
    uint32_t u = bucket->count;

    // Perform a binary search by comparing to middle element
    while (l < u)
    {
        // Update pivot index to be middle of the range
        uint32_t p = l + ((u - l) >> 1);

        // Compare key to pivot
        int comparison = keycmp(key, sorted[p].key);

        // Use lower half as new range
        if (comparison < 0) u = p;
        // Use upper half as new range
        else if (comparison > 0) l = p + 1;
        // Return matching pivot
        else return p;
    }

    return l;
}


// Return data of entry with specified key O(log N)
static data_t _bucket_bsearch(bucket_t* bucket, hashkey_t key)
{
    data_t data = NULL;

    // Find matching entry O(log N)
    uint32_t index = _bucket_index_bsearch(bucket, key);
    entry_t* chain = bucket->chain;

    // Return matching entry
    if (index < bucket->count && !keycmp(key, chain[index].key))
    {
        data = chain[index].data;
    }

    return data;
}

// Insert new entry into bucket in sorted order O(N)
static void _bucket_binsert(bucket_t* bucket, hashkey_t key, data_t data)
{
    // Expand bucket memory if necessary
    if (bucket->count == bucket->size)
    {
        bucket->size += BLOCK_SIZE;
        bucket->chain = (entry_t*) realloc(bucket->chain, bucket->size * sizeof(entry_t));
    }

    // Determine position to insert at O(log N)
    uint32_t index = _bucket_index_bsearch(bucket, key);
    entry_t* chain = bucket->chain;

    // Insert and shift entries into place O(N)
    if (index <= bucket->count)
    {
        memmove(&chain[index + 1], &chain[index], (bucket->count++ - index) * sizeof(entry_t));
        chain[index].key = key;
        chain[index].data = data;
    }
}

static data_t _bucket_bremove(bucket_t* bucket, hashkey_t key)
{
    data_t data = NULL;

    // Find matching entry O(log N)
    uint32_t index = _bucket_index_bsearch(bucket, key);
    entry_t* chain = bucket->chain;

    // Shift entries into place O(N)
    if (index < bucket->count && !keycmp(key, chain[index].key))
    {
        data = chain[index].data;
        memmove(&chain[index], &chain[index + 1], (--bucket->count - index) * sizeof(entry_t));
    }

    return data;
}


// Return index to entry with specified key O(N)
static uint32_t _bucket_index_search(bucket_t* bucket, hashkey_t key)
{
    uint32_t i = 0;

    for (; i < bucket->count; i++)
    {
        entry_t* entry = &bucket->chain[i];

        if (!keycmp(key, entry->key)) break;
    }

    return i;
}


// Return data of entry with specified key O(N)
static data_t _bucket_search(bucket_t* bucket, hashkey_t key)
{
    data_t data = NULL;

    // Find matching entry O(N)
    uint32_t index = _bucket_index_search(bucket, key);
    entry_t* chain = bucket->chain;

    // Return matching entry
    if (index < bucket->count)
    {
        data = chain[index].data;
    }

    return data;
}


// Insert new entry into bucket O(1)
static void _bucket_insert(bucket_t* bucket, hashkey_t key, data_t data)
{
    // Expand bucket memory if necessary
    if (bucket->count == bucket->size)
    {
        bucket->size += BLOCK_SIZE;
        bucket->chain = (entry_t*) realloc(bucket->chain, bucket->size * sizeof(entry_t));
    }

    // Insert entry into bucket in sorted order by key
    bucket->chain[bucket->count].key = key;
    bucket->chain[bucket->count++].data = data;
}


// Remove entry with specified key from bucket returning data O(N)
static data_t _bucket_remove(bucket_t* bucket, hashkey_t key)
{
    data_t data = NULL;

    // Find matching entry O(N)
    uint32_t index = _bucket_index_search(bucket, key);
    entry_t* chain = bucket->chain;

    // Shift entries into place O(N)
    if (index < bucket->count)
    {
        data = chain[index].data;
        memmove(&chain[index], &chain[index + 1], (--bucket->count - index) * sizeof(entry_t));
    }

    return data;
}


// Initialize a hash table object
void hash_init(hash_t* table, uint64_t size)
{
    if (!table) return;

    table->entries = 0;
    table->size = size;
    table->buckets = (bucket_t*) malloc(table->size * sizeof(bucket_t));

    // Initialize buckets
    for (uint64_t i = 0; i < table->size; i++)
    {
        _bucket_init(&table->buckets[i]);
    }
}


// Create new larger hash table
void hash_rehash(hash_t* table)
{
    if (!table) return;

    bucket_t* old_buckets = table->buckets;
    uint64_t old_size = table->size;

    hash_init(table, table->size * GROWTH_FACTOR);

    for (uint64_t i = 0; i < old_size; i++)
    {
        bucket_t* bucket = &old_buckets[i];

        for (uint32_t j = 0; j < bucket->count; j++)
        {
            entry_t* entry = &bucket->chain[j];

            hash_insert(table, entry->key, entry->data);
        }

        free(bucket->chain);
    }

    free(old_buckets);
}


// Cleanup and deallocate a hash table object
void hash_free(hash_t* table)
{
    if (!table) return;

    for (uint64_t i = 0; i < table->size; i++)
    {
        free(table->buckets[i].chain);
    }
    free(table->buckets);
}


// Insert new entry into hash table using specified key O(1)
void hash_insert(hash_t* table, hashkey_t key, data_t data)
{
    if (!table) return;

    // Determine which bucket to process
    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map32(hash, table->size);

    // Insert into bucket
    if (BSEARCH) _bucket_binsert(&table->buckets[index], key, data);
    else _bucket_insert(&table->buckets[index], key, data);
    table->entries++;

    // Resize table if necessary
    if (table->entries / table->size >= MAX_ALPHA) hash_rehash(table);
}


// Return data of the entry with specified key O(1)
data_t hash_search(hash_t* table, hashkey_t key)
{
    if (!table) return NULL;

    data_t data = NULL;

    // Determine which bucket to process
    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map32(hash, table->size);

    // Search bucket for key
    if (BSEARCH) data = _bucket_bsearch(&table->buckets[index], key);
    else data = _bucket_search(&table->buckets[index], key);

    return data;
}


// Remove entry with specified key returning data O(1)
data_t hash_remove(hash_t* table, hashkey_t key)
{
    if (!table) return NULL;

    data_t data = NULL;

    // Determine which bucket to process
    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map32(hash, table->size);

    // Remove entry from bucket
    if (BSEARCH) data = _bucket_bremove(&table->buckets[index], key);
    else data = _bucket_remove(&table->buckets[index], key);

    return data;
}


// Print table statistics
void hash_print_stats(hash_t* table)
{
    if (!table) return;

    uint32_t max = 0;
    uint32_t min = UINT32_MAX;
    uint32_t avg = 0;

    for (uint64_t i = 0; i < table->size; i++)
    {
        bucket_t* bucket = &table->buckets[i];

        if (bucket->count < min) min = bucket->count;
        if (bucket->count > max) max = bucket->count;
        avg += bucket->count;
    }

    printf("entries: %zu, size: %zu, alpha %.2f\n", (size_t) table->entries, (size_t) table->size, (float) table->entries / table->size);
    printf("min-depth: %zu, avg-depth: %zu, max-depth: %zu\n", (size_t) min, (size_t) avg / table->size, (size_t) max);
}


// Debug print used to visualize hash table
void hash_print_debug(hash_t* table)
{
    if (!table) return;

    for (uint64_t i = 0; i < table->size; i++)
    {
        bucket_t* bucket = &table->buckets[i];

        printf("[%zu] ", (size_t) i);
        for (uint32_t j = 0; j < bucket->count; j++) printf("*");
        printf("\n");
    }
}
