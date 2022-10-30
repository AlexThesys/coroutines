#ifndef _STACKS_H_
#define _STACK_H_

#include "common.h"

#define NUM_STACKS 16
#define STACK_SIZE 4096
#define ALLOCATE_PRECISE

typedef struct stack {
    u8* ptr_top;
    u8* ptr_bottom;
    volatile BOOL is_free;    
} stack;

stack stacks[NUM_STACKS];

void init_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
        // stack grows from the top and has to be aligned by 0x10 bytes
#ifndef ALLOCATE_PRECISE
        stacks[i].ptr_bottom = (u8*)malloc(STACK_SIZE+0x10); // account for 16 bytes alignment
        stacks[i].ptr_top = (u8*)(((u64)stacks[i].ptr_bottom + STACK_SIZE + 0x0F) & ~(0x0F));
#else
        stacks[i].ptr_bottom = (u8*)malloc(STACK_SIZE);
        stacks[i].ptr_top = (u8*)(((u64)stacks[i].ptr_bottom + STACK_SIZE) & ~(0x0F));
#endif
        stacks[i].is_free = TRUE;
    }    
}

void deinit_stacks() {
    for (int i = 0; i < NUM_STACKS; i++) {
        free(stacks[i].ptr_bottom);
    }
}

BOOL get_free_stack(u8** ptr, u64* stack_id) {
    for (int i = 0; i < NUM_STACKS; i++) {
        if (atomic_exchange(&stacks[i].is_free, FALSE)) {
            *ptr = stacks[i].ptr_top;
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
