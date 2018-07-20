// hash.c
// kpadron.github@gmail.com
// Kristian Padron
// implementation for hash table object
#include <stdlib.h>

#include "list.h"
#include "hash.h"


// Perform one-at-a-time hash on input bytes
inline unsigned hash_oaat(void* key, size_t length)
{
    char* p = (char*) key;
    unsigned h = 0;

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
