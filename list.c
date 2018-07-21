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


// Search linearly through nodes until node with key is found
static node_t* _node_search(node_t* node, key_t key)
{
    while (node)
    {
        if (key == node->key) return node;
        node = node->next;
    }

    return NULL;
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
    if (!list) return;

    // Append to end of list
    if (index >= list->length)
    {
        list_append(list, key, data);
    }
    // Prepend to front of list
    else if (index <= 0)
    {
        list_prepend(list, key, data);
    }
    // Insert into middle of list
    else
    {
        node_t* rover = list->head;
        int reverse = 0;
        size_t shifts = index;

        // Start from tail if beneficial
        if (index > list->length - index - 1)
        {
            rover = list->tail;
            reverse = 1;
            shifts = list->length - index - 1;
        }

        // Shift to the correct position
        if (reverse) rover = _node_lshift(rover, shifts);
        else rover = _node_rshift(rover, shifts);

        // Insert node into list adjusting pointers
        node_t* node = _node_alloc(key, data);
        node->prev = rover->prev;
        node->next = rover;

        rover->prev->next = node;
        rover->prev = node;

        list->length++;
    }
}


// Add a new node in sorted order based on key O(N)
void list_insort(list_t* list, key_t key, data_t data)
{

}


// Return the data of the node at the specified location O(N)
data_t list_access(list_t* list, size_t index, key_t key)
{
    if (!list) return NULL;

    index = index < list->length ? index : list->length - 1;

    node_t* rover = list->head;
    int reverse = 0;
    size_t shifts = index;

    // Start from tail if beneficial
    if (index > list->length - index - 1)
    {
        rover = list->tail;
        reverse = 1;
        shifts = list->length - index - 1;
    }

    // Shift to the correct position
    if (reverse) rover = _node_lshift(rover, shifts);
    else rover = _node_rshift(rover, shifts);

    return rover->data;
}


// Return the data of the first node with the specified key O(N)
data_t list_search(list_t* list, key_t key)
{
    if (!list) return NULL;

    return _node_search(list->head, key);
}


// Remove node from the end of the list returning data O(1)
data_t list_pop(list_t* list)
{
    if (!list || !list->tail) return NULL;

    // Remove node from list
    node_t* node = list->tail;
    list->tail = list->tail->prev;

    // Readjust list pointers
    if (list->tail) list->tail->next = NULL;
    else list->head = list->tail;

    // Cleanup removed node
    data_t data = node->data;
    _node_dealloc(node);
    list->length--;

    return data;
}


// Remove node from the start of the list returning data O(1)
data_t list_shift(list_t* list)
{
    if (!list || !list->head) return NULL;

    // Remove node from list
    node_t* node = list->head;
    list->head = list->head->next;

    // Readjust list pointers
    if (list->head) list->head->prev = NULL;
    else list->tail = list->head;

    // Cleanup removed node
    data_t data = node->data;
    _node_dealloc(node);
    list->length--;

    return data;
}


// Remove node from specified position in the list returning data O(N)
data_t list_remove(list_t* list, size_t index)
{
    if (!list) return NULL;

    // Pop from end of list
    if (index >= list->length - 1)
    {
        return list_pop(list);
    }
    // Shift from front of list
    else if (index <= 0)
    {
        return list_shift(list);
    }
    // Remove from middle of list
    else
    {
        node_t* rover = list->head;
        int reverse = 0;
        size_t shifts = index;

        // Start from tail if beneficial
        if (index > list->length - index - 1)
        {
            rover = list->tail;
            reverse = 1;
            shifts = list->length - index - 1;
        }

        // Shift to the correct position
        if (reverse) rover = _node_lshift(rover, shifts);
        else rover = _node_rshift(rover, shifts);

        // Remove node from list adjusting pointers
        rover->prev->next = rover->next;
        rover->next->prev = rover->prev;

        // Cleanup removed node
        data_t data = rover->data;
        _node_dealloc(rover);
        list->length--;

        return data;
    }
}


// Remove first node with the specified key returning data O(N)
data_t list_delete(list_t* list, key_t key)
{
    if (!list) return NULL;

    node_t* node = _node_search(list->head, key);
    data_t data = NULL;

    if (node)
    {
        // Remove node from list and adjust pointers
        if (node->prev) node->prev->next = node->next;
        else list->head = node->next;

        if (node->next) node->next->prev = node->prev;
        else list->tail = node->prev;

        // Cleanup removed node
        data = node->data;
        _node_dealloc(node);
        list->length--;
    }

    return data;
}
