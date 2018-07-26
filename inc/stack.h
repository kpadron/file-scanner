// stack.h
// kpadron.github@gmail.com
// Kristian Padron
// dynamic array FILO stack module
#pragma once
#include <stdlib.h>
#include <stdint.h>

#define STACK_BLOCK_SIZE 64

typedef void* data_t;

typedef struct
{
    uint64_t count;
    uint64_t size;
    data_t* array;
} stackarray_t;

// Initialize a stack object
extern void stack_init(stackarray_t* stack);

// Cleanup and deallocate a stack object
extern void stack_free(stackarray_t* stack);

// Add a new pointer to the top of the stack
extern void stack_push(stackarray_t* stack, data_t data);

// Remove data from the top of the stack
extern data_t stack_pop(stackarray_t* stack);

// Search for key in stack using keycmp function
extern data_t stack_search(stackarray_t* stack, void* key, int (*keycmp)(void*, void*));
