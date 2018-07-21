// hash.c
// kpadron.github@gmail.com
// Kristian Padron
// implementation for hash table object
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hash.h"


// Maps a number to the range [0, n] faster than modulo division
static inline uint32_t _map(uint32_t x, uint32_t n)
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
    if (!bucket) return;

    bucket->count = 0;
    bucket->size = 0;
    bucket->chain = NULL;
}


// Cleanup and deallocate bucket object
static void _bucket_free(bucket_t* bucket)
{
    if (!bucket) return;

    free(bucket->chain);
}


// Insert new entry into bucket
static void _bucket_insert(bucket_t* bucket, hashkey_t key, data_t data)
{
    if (!bucket) return;

    // Expand bucket memory if necessary
    if (bucket->count == bucket->size)
    {
        bucket->size += BLOCK_SIZE;
        bucket->chain = realloc(bucket->chain, bucket->size);
    }

    // Insert entry into bucket
    bucket->chain[bucket->count].key = key;
    bucket->chain[bucket->count].data = data;
    bucket->count++;
}


// Return data of entry with specified key
static data_t _bucket_search(bucket_t* bucket, hashkey_t key)
{
    if (!bucket) return NULL;

    for (uint32_t i = 0; i < bucket->count; i++)
    {
        entry_t* entry = &bucket->chain[i];

        if (entry->key == key) return entry->data;
    }

    return NULL;
}


// Remove entry with specified key from bucket returning data
static data_t _bucket_remove(bucket_t* bucket, hashkey_t key)
{
    if (!bucket) return NULL;

    uint32_t i = 0;

    for (; i < bucket->count; i++)
    {
        entry_t* entry = &bucket->chain[i];

        if (entry->key == key) break;
    }

    data_t data = NULL;

    if (i != bucket->count)
    {
        data = bucket->chain[i].data;
        memmove(&bucket->chain[i], &bucket->chain[i + 1], bucket->count - i - 1);
        bucket->count--;
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
    table->buckets = NULL;

    hash_init(table, table->size * GROWTH_FACTOR);

    for (uint64_t i = 0; i < old_size; i++)
    {
        bucket_t* bucket = &old_buckets[i];

        for (uint32_t j = 0; j < bucket->count; j++)
        {
            entry_t* entry = &bucket->chain[i];

            hash_insert(table, entry->key, entry->data);
        }

        // free(bucket->chain);
    }

    // free(old_buckets);
}


// Cleanup and deallocate a hash table object
void hash_free(hash_t* table)
{
    if (!table) return;

    for (uint64_t i = 0; i < table->size; i++)
    {
        _bucket_free(&table->buckets[i]);
    }
}


// Insert new entry into hash table using specified key O(1)
void hash_insert(hash_t* table, hashkey_t key, data_t data)
{
    if (!table) return;

    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map(hash, table->size);

    _bucket_insert(&table->buckets[index], key, data);
    table->entries++;

    if (table->entries / table->size >= MAX_ALPHA) hash_rehash(table);
}


// Return data of the entry with specified key O(1)
data_t hash_search(hash_t* table, hashkey_t key)
{
    if (!table) return NULL;

    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map(hash, table->size);

    return _bucket_search(&table->buckets[index], key);
}


// Remove entry with specified key returning data O(1)
data_t hash_remove(hash_t* table, hashkey_t key)
{
    if (!table) return NULL;

    uint32_t hash = _hash_oaat(&key, sizeof key);
    uint64_t index = _map(hash, table->size);

    return _bucket_remove(&table->buckets[index], key);
}


// Debug print used to visualize hash table
void hash_print(hash_t* table)
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
