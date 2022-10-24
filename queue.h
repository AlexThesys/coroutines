#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <pthread.h>

#include "common.h"

#define MAX_QUEUE_LENGHT 16

typedef struct gp_regs {
    u64 rbx;
    u64 rbp;
    u64 rsp;

    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
} gp_regs;

typedef struct yielded_task {
    void* rip;
    gp_regs  gprs;
} yielded_task;

typedef void(*task)(void*);

typedef struct next_task {
   task f;
   void* params;
} next_task;

#define task_queue(name)\ 
typedef struct ##name##_task_queue {\
    ##name##_task tasks[MAX_QUEUE_LENGHT];\
    pthread_mutex_t mtx_lock;\
    int read_idx;\
    int write_idx;\
    int read_round;\
    int write_round;\
} ##name##_task_queue\
void init_##name##_queue() {\
    ##name##_task_queue->write_index = 0;\
    ##name##_task_queue->read_index = 0;\
    ##name##_task_queue->write_round = 0;\
    ##name##_task_queue->read_round = 0;\
}\
BOOL try_pop_##name##(###name##_task_queue* tasks, #name##_task* task) {\
    pthread_mutex_lock(&##name##_task_queue->mtx_lock);\
    if (##name##_task_queue->write_index == ##name##_task_queue->read_index \
        && ##name##_task_queue->write_round == ##name##_task_queue->read_round) {\
        pthread_mutex_unlock(&##name##_task_queue->mtx_lock);\
        puts("The queue is empty.");\
        return FALSE;\
    }\
    ##name##_task_queue->read_round ^= (##name##_task_queue->read_index == MAX_QUEUE_LENGHT - 1);
    ##name##_task_queue->read_index = (##name##_task_queue->read_index + 1) & (MAX_QUEUE_LENGHT - 1);\
    memcpy(task, &##name##_task_queue->tasks[##name##_task_queue->read_index], sizeof(*task));\
    pthread_mutex_unlock(&##name##_task_queue->mtx_lock);\
    return TRUE;\
}\    
BOOL is_empty##name##(###name##_task_queue* tasks) {\
    pthread_mutex_lock(&##name##_task_queue->mtx_lock);\
    BOOL empty = (##name##_task_queue->write_index == ##name##_task_queue->read_index \
        && ##name##_task_queue->write_round == ##name##_task_queue->read_round);\
    pthread_mutex_unlock(&##name##_task_queue->mtx_lock);\
    puts("The queue is empty.");\
    return empty;\
}

task_queue(next);
task_queue(yielded);

void try_push_yielded_task_queue(yielded_task_queue* tasks) {
    pthread_mutex_lock(&yielded_task_queue->mtx_lock);
    if (yielded_task_queue->write_index == yielded_task_queue->read_index 
            && yielded_task_queue->write_round != yielded_task_queue->read_round) {
        pthread_mutex_unlock(&yielded_task_queue->mtx_lock);
        puts("The queue is full.");
        return;
    }
    yielded_task_queue->write_round ^= (yielded_task_queue->write_index == MAX_QUEUE_LENGHT - 1);
    const int write_index = yielded_task_queue->write_index;
    yielded_task_queue->write_index = (yielded_task_queue->write_index + 1) & (MAX_QUEUE_LENGHT - 1);
    save_and_yield_impl(&yielded_task_queue->tasks[write_index], &yielded_task_queue->mtx_lock); // we also unlock the mutex here
    return;
}    

BOOL try_push_next_task_queue(next_task_queue* tasks, next_task* task) {
    pthread_mutex_lock(&next_task_queue->mtx_lock);
    if (next_task_queue->write_index == next_task_queue->read_index 
            && next_task_queue->write_round != next_task_queue->read_round) {
        pthread_mutex_unlock(&next_task_queue->mtx_lock);
        puts("The queue is full.");
        return FALSE;
    }
    memcpy(&next_task_queue->tasks[next_task_queue->write_index], task, sizeof(*task));
    next_task_queue->write_round ^= (next_task_queue->write_index == MAX_QUEUE_LENGHT - 1);
    next_task_queue->write_index = (next_task_queue->write_index + 1) & (MAX_QUEUE_LENGHT - 1);
    pthread_mutex_unlock(&next_task_queue->mtx_lock);
    return TRUE;
}    

#endif // _QUEUE_H_
