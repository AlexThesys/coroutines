#ifndef _STACKS_H_
#define _STACK_H_

#include "common.h"

#define NUM_STACKS 16
#define STACK_SIZE 4096

typedef struct stack {
    u8* ptr;
    volatile BOOL is_free;    
} stack;

stack stacks[NUM_STACKS];

void init_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
        stacks[i].ptr = (u8*)malloc(STACK_SIZE+0x10); // account for 16 bytes alignment
        stacks[i].is_free = TRUE;
    }    
}

void deinit_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
        free(staks[i].ptr);
    }
}

BOOL get_free_stack(u8* ptr, u64* stack_id) {
    for (int i = 0; i < NUM_STACKS; i++) {
        if (atomic_compare_exchange_strong_explicit(&stacks[i].is_free, TRUE, FALSE)) {
            // stack grows from the top and has to be aligned by 0x10 bytes
            ptr = (stacks[i].ptr + STACK_SIZE + 0x0F) & ~(0x0F);
            assert((ptr & 0x0F) == 0);
            *stack_id = i;
            return TRUE; 
        }
    }
    puts("No free stacks available!");
    return FALSE;
}

void set_free_stack(u64 stack_id) {
    stacks[stack_id].is_free = TRUE;
}

#endif // _STACKS_H_
