// hash.h
// kpadron.github@gmail.com
// Kristian Padron
// hash table module
#ifndef HASH_H
#define HASH_H

typedef unsigned key_t; // key value to be used for search and comparison
typedef void* data_t;   // pointer to data to be stored in list

typedef struct node
{
    key_t key;
    data_t data;
    struct node* next;
    struct node* prev;
} node_t;

typedef struct
{
    node_t* head;
    node_t* tail;
    unsigned length;
} list_t;

// Perform one-at-a-time hash on input bytes
extern inline unsigned hash_oaat(void* key, size_t length);

#endif
