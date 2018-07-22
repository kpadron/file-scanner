// list.h
// kpadron.github@gmail.com
// Kristian Padron
// linked list module
#pragma once
#include <sdtlib.h>

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

// Initialize a list object
extern void list_init(list_t* list);

// Cleanup and deallocate a list object
extern void list_free(list_t* list);

// Add a new node to the end of the list O(1)
extern void list_append(list_t* list, key_t key, data_t data);

// Add a new node to the start of the list O(1)
extern void list_prepend(list_t* list, key_t key, data_t data);

// Add new node in sorted order based on key O(N)
extern void list_insert(list_t* list, key_t key, data_t data);

// Add new node in sorted order based on key O(N)
extern void list_insort(list_t* list, key_t key, data_t data);

// Return the data of the node at the specified location O(N)
extern data_t list_access(list_t* list, size_t index);

// Return the data of the first node with the specified key O(N)
extern data_t list_search(list_t* list, key_t key);

// Remove node from the end of the list returning data O(1)
extern data_t list_pop(list_t* list);

// Remove node from the start of the list returning data O(1)
extern data_t list_shift(list_t* list);

// Remove node from specified position in the list returning data O(N)
extern data_t list_remove(list_t* list, size_t index);

// Remove first node with the specified key returning data O(N)
extern data_t list_delete(list_t* list, key_t key);
