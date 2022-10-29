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

extern void save_and_yield_impl(yielded_task*, pthread_mutex_t*);

typedef void(*task)(void*);

typedef struct next_task {
   task f;
   void* params;
} next_task;

#define define_task_queue(name)\
typedef struct name##_task_queue {\
    name##_task tasks[MAX_QUEUE_LENGHT];\
    pthread_mutex_t mtx_lock;\
    int read_idx;\
    int write_idx;\
    int read_round;\
    int write_round;\
} name##_task_queue;\
void init_##name##_queue(name##_task_queue* tasks_q) {\
    tasks_q->write_idx = 0;\
    tasks_q->read_idx = 0;\
    tasks_q->write_round = 0;\
    tasks_q->read_round = 0;\
}\
BOOL try_pop_##name##_task_queue(name##_task_queue* tasks_q, name##_task* task) {\
    pthread_mutex_lock(&tasks_q->mtx_lock);\
    if (tasks_q->write_idx == tasks_q->read_idx \
        && tasks_q->write_round == tasks_q->read_round) {\
        pthread_mutex_unlock(&tasks_q->mtx_lock);\
        puts("The queue is empty.");\
        return FALSE;\
    }\
    tasks_q->read_round ^= (tasks_q->read_idx == (MAX_QUEUE_LENGHT - 1));\
    tasks_q->read_idx = (tasks_q->read_idx + 1) & (MAX_QUEUE_LENGHT - 1);\
    memcpy(task, &tasks_q->tasks[tasks_q->read_idx], sizeof(*task));\
    pthread_mutex_unlock(&tasks_q->mtx_lock);\
    return TRUE;\
}\
BOOL is_empty##name(name##_task_queue* tasks_q) {\
    pthread_mutex_lock(&tasks_q->mtx_lock);\
    BOOL empty = (tasks_q->write_idx == tasks_q->read_idx \
        && tasks_q->write_round == tasks_q->read_round);\
    pthread_mutex_unlock(&tasks_q->mtx_lock);\
    puts("The queue is empty.");\
    return empty;\
}

define_task_queue(next);
define_task_queue(yielded);

BOOL try_push_yielded_task_queue(yielded_task_queue* tasks_q) {
    pthread_mutex_lock(&tasks_q->mtx_lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        pthread_mutex_unlock(&tasks_q->mtx_lock);
        puts("The queue is full.");
        return FALSE;
    }
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    const int write_idx = tasks_q->write_idx ;
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    save_and_yield_impl(&tasks_q->tasks[write_idx ], &tasks_q->mtx_lock); // we also unlock the mutex here
    return TRUE;
}    

BOOL try_push_next_task_queue(next_task_queue* tasks_q, next_task* task) {
    pthread_mutex_lock(&tasks_q->mtx_lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        pthread_mutex_unlock(&tasks_q->mtx_lock);
        puts("The queue is full.");
        return FALSE;
    }
    memcpy(&tasks_q->tasks[tasks_q->write_idx ], task, sizeof(*task));
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    pthread_mutex_unlock(&tasks_q->mtx_lock);
    return TRUE;
}    

#endif // _QUEUE_H_
