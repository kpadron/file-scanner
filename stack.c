// stack.c
// kpadron.github@gmail.com
// Kristian Padron
// implementation of FILO stack
#include <stdlib.h>
#include <stdint.h>

#include "stack.h"


static void _stack_resize(stackarray_t* stack, uint64_t size)
{
    if (!stack || stack->size == size) return;

    stack->size = size;
    stack->array = realloc(stack->array, stack->size * sizeof(data_t));
}

void stack_init(stackarray_t* stack)
{
    if (!stack) return;

    stack->count = 0;
    stack->size = 0;
    stack->array = NULL;
}


void stack_free(stackarray_t* stack)
{
    if (!stack) return;

    free(stack->array);
}


void stack_push(stackarray_t* stack, data_t data)
{
    if (!stack) return;

    if (stack->count == stack->size)
        _stack_resize(stack, stack->size + STACK_BLOCK_SIZE);

    stack->array[stack->count++] = data;
}

data_t stack_pop(stackarray_t* stack)
{
    if (!stack || !stack->count) return NULL;

    return stack->array[--stack->count];
}
