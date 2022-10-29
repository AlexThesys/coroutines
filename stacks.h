#ifndef _STACKS_H_
#define _STACK_H_

#include "common.h"

#define NUM_STACKS 16
#define STACK_SIZE 4096
#define ALLOCATE_PRECISE

typedef struct stack {
    u8* ptr;
    volatile BOOL is_free;    
} stack;

stack stacks[NUM_STACKS];

void init_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
#ifndef ALLOCATE_PRECISE
        stacks[i].ptr = (u8*)malloc(STACK_SIZE+0x10); // account for 16 bytes alignment
#else
        stacks[i].ptr = (u8*)malloc(STACK_SIZE);
#endif
        stacks[i].is_free = TRUE;
    }    
}

void deinit_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
        free(stacks[i].ptr);
    }
}

BOOL get_free_stack(u8** ptr, u64* stack_id) {
    for (int i = 0; i < NUM_STACKS; i++) {
        if (atomic_exchange(&stacks[i].is_free, FALSE)) {
            // stack grows from the top and has to be aligned by 0x10 bytes
#ifndef ALLOCATE_PRECISE
            *ptr = (u8*)(((u64)stacks[i].ptr + STACK_SIZE + 0x0F) & ~(0x0F));
#else
            *ptr = (u8*)(((u64)stacks[i].ptr + STACK_SIZE) & ~(0x0F));
#endif
            assert(((u64)*ptr & 0x0F) == 0);
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
