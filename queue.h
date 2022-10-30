#ifndef _QUEUE_H_
#define _QUEUE_H_

//#define SPINLOCK
#ifndef SPIN_LOCK
#include <pthread.h>
#endif

#include "common.h"
#include "spinlock.h"

#define MAX_QUEUE_LENGHT 16

volatile BOOL force_yielded = FALSE;

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

#ifdef SPINLOCK
extern void save_and_yield_impl(yielded_task*, volatile s32*);
#define LOCK volatile s32
#define LOCK_INIT tasks_q->lock = 0
#define lock_lock spinlock_lock
#define lock_unlock spinlock_unlock
#else
extern void save_and_yield_impl(yielded_task*, pthread_mutex_t*);
#define LOCK pthread_mutex_t 
#define LOCK_INIT
#define lock_lock pthread_mutex_lock
#define lock_unlock pthread_mutex_unlock
#endif

typedef void(*task)(void*);

typedef struct next_task {
   task f;
   void* params;
} next_task;

#define define_task_queue(name)\
typedef struct name##_task_queue {\
    name##_task tasks[MAX_QUEUE_LENGHT];\
    LOCK lock;\
    int read_idx;\
    int write_idx;\
    int read_round;\
    int write_round;\
} name##_task_queue;\
void init_##name##_queue(name##_task_queue* tasks_q) {\
    LOCK_INIT;\
    tasks_q->write_idx = 0;\
    tasks_q->read_idx = 0;\
    tasks_q->write_round = 0;\
    tasks_q->read_round = 0;\
}\
BOOL is_empty##name(name##_task_queue* tasks_q) {\
    lock_lock(&tasks_q->lock);\
    BOOL empty = (tasks_q->write_idx == tasks_q->read_idx \
        && tasks_q->write_round == tasks_q->read_round);\
    lock_unlock(&tasks_q->lock);\
    puts("The queue is empty.");\
    return empty;\
}

define_task_queue(next);
define_task_queue(yielded);

BOOL try_pop_next_task_queue(next_task_queue* tasks_q, next_task* task) {
    lock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx 
        && tasks_q->write_round == tasks_q->read_round) {
        lock_unlock(&tasks_q->lock);
        puts("Next task queue is empty.");
        return FALSE;
    }
    memcpy(task, &tasks_q->tasks[tasks_q->read_idx], sizeof(*task));
    tasks_q->read_round ^= (tasks_q->read_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->read_idx = (tasks_q->read_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pop task from next tasks queue");
    lock_unlock(&tasks_q->lock);
    return TRUE;
}

BOOL try_pop_yielded_task_queue(yielded_task_queue* tasks_q, yielded_task* task) {
    lock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx 
        && tasks_q->write_round == tasks_q->read_round) {
        lock_unlock(&tasks_q->lock);
        force_yielded = FALSE;
        puts("Yielded task queue is empty.");
        return FALSE;
    }
    memcpy(task, &tasks_q->tasks[tasks_q->read_idx], sizeof(*task));
    tasks_q->read_round ^= (tasks_q->read_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->read_idx = (tasks_q->read_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pop task from yielded tasks queue");
    lock_unlock(&tasks_q->lock);
    return TRUE;
}

BOOL try_push_next_task_queue(next_task_queue* tasks_q, next_task* task) {
    lock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        lock_unlock(&tasks_q->lock);
        puts("Next task queue is full.");
        return FALSE;
    }
    memcpy(&tasks_q->tasks[tasks_q->write_idx], task, sizeof(*task));
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    lock_unlock(&tasks_q->lock);
    return TRUE;
}    

BOOL try_push_yielded_task_queue(yielded_task_queue* tasks_q) {
    lock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        lock_unlock(&tasks_q->lock);
        puts("Yielded task queue is full.");
        return FALSE;
    }
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    const int write_idx = tasks_q->write_idx ;
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    save_and_yield_impl(&tasks_q->tasks[write_idx], &tasks_q->lock); // we also unlock the lock here
    return TRUE;
}    

#endif // _QUEUE_H_
