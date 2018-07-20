// list.c
// kpadron.github@gmail.com
// Kristian Padron
// implementation for linked list object
#include <stdlib.h>

#include "list.h"

#define BLOCK_SIZE 4096
#define NODE_ALLOC_TYPE 0

static char* _memory_pool = NULL;
static size_t _pool_size = 0;
static node_t* _free_list = NULL;


// Allocate a node from the memory pool
static node_t* _node_pool_alloc(void)
{
    // Need more memory if free list is empty
    if (!_free_list)
    {
        // Request new memory from the OS
        _memory_pool = (char*) realloc(_memory_pool, _pool_size + BLOCK_SIZE);

        // Determine number of nodes that fit in old size
        size_t offset = _pool_size / sizeof(node_t);

        // Offset free list to correct memory address
        _free_list = (node_t*) _memory_pool + offset;

        // Determine number of nodes that fits in a block
        size_t nodes = BLOCK_SIZE / sizeof(node_t);

        // Add new nodes to the free list
        node_t* rover = _free_list;
        for (size_t i = 1; i < nodes; i++)
        {
            rover->next = rover + 1;
            rover = rover->next;
        }
        rover->next = NULL;

        // Increase pool size
        _pool_size += BLOCK_SIZE;
    }

    // Grab a new node from the free list
    node_t* node = _free_list;
    _free_list = _free_list->next;

    return node;
}


// Cleanup and deallocate a node object
static void _node_pool_dealloc(node_t* node)
{
    if (!node) return;

    // Add node back on to the free list
    node->next = _free_list;
    _free_list = node;
}


// Allocate and return node object with (key, data)
static node_t* _node_alloc(key_t key, data_t data)
{
    node_t* node;

    // Allocate a new node
    if (NODE_ALLOC_TYPE)
    {
        node = _node_pool_alloc();
    }
    else
    {
        node = (node_t*) malloc(sizeof(node_t));
    }

    // Set key and data
    node->key = key;
    node->data = data;

    return node;
}


// Free node deallocating memory
static void _node_dealloc(node_t* node)
{
    if (!node) return;

    // Deallocate node returning memory to OS
    if (NODE_ALLOC_TYPE)
    {
        _node_pool_dealloc(node);
    }
    else
    {
        free(node);
    }
}


// Shift to the node left by <shifts> positions
static node_t* _node_lshift(node_t* node, size_t shifts)
{
    while (node && shifts--)
    {
        node = node->prev;
    }

    return node;
}


// Shift to the node right by <shifts> positions
static node_t* _node_rshift(node_t* node, size_t shifts)
{
    while (node && shifts--)
    {
        node = node->next;
    }

    return node;
}


// Recursive list free
static void _list_free(node_t* node)
{
    if (!node) return

    _list_free(node->next);
    _node_dealloc(node);
}


// Initialize a list object
void list_init(list_t* list)
{
    if (!list) return;

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}


// Cleanup and deallocate a list object
void list_free(list_t* list)
{
    if (!list) return;

    _list_free(list->head);
}


// Add a new node to the end of the list O(1)
void list_append(list_t* list, key_t key, data_t data)
{
    if (!list) return;

    node_t node = _node_alloc(key, data);
    node->prev = list->tail;
    node->next = NULL;

    if (list->tail) list->tail->next = node;
    else list->head = node;

    list->tail = node;
    list->length++;
}


// Add a new node to the start of the list O(1)
void list_prepend(list_t* list, key_t key, data_t data)
{
    if (!list) return;

    node_t node = _node_alloc(key, data);
    node->prev = NULL;
    node->next = list->head;

    if (list->head) list->head->prev = node;
    else list->tail = node;

    list->head = node;
    list->length++;
}


// Add a new node to the list at the specified position O(N)
void list_insert(list_t* list, size_t index, key_t key, data_t data)
{
    
}


// Add a new node in sorted order based on key O(N)
void list_insort(list_t* list, key_t key, data_t data)
{

}


// Return the data of the first node with the specified key O(N)
data_t list_search(list_t* list, key_t key)
{
    if (!list) return NULL;

    node_t* rover = list->head;

    while (rover)
    {
        if (key == rover->key) return rover->data;
        rover = rover->next;
    }

    return NULL;
}
