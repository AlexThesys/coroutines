#ifndef _QUEUE_H_
#define _QUEUE_H_

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

extern void co_yield_impl(yielded_task*, volatile s32*, semaphore* sem);

typedef void(*task)(void*);

typedef struct next_task {
   task f;
   void* params;
} next_task;

#define define_task_queue(name)\
typedef struct name##_task_queue {\
    name##_task tasks[MAX_QUEUE_LENGHT];\
    volatile s32 lock;\
    u16 read_idx;\
    u16 write_idx;\
    u8 read_round;\
    u8 write_round;\
} name##_task_queue;\
void init_##name##_queue(name##_task_queue* tasks_q) {\
    tasks_q->lock = 0;\
    tasks_q->write_idx = 0;\
    tasks_q->read_idx = 0;\
    tasks_q->write_round = 0;\
    tasks_q->read_round = 0;\
}\
BOOL is_empty_##name(name##_task_queue* tasks_q) {\
    spinlock_lock(&tasks_q->lock);\
    BOOL empty = (tasks_q->write_idx == tasks_q->read_idx \
        && tasks_q->write_round == tasks_q->read_round);\
    spinlock_unlock(&tasks_q->lock);\
    puts("The queue is empty.");\
    return empty;\
}

define_task_queue(next);
define_task_queue(yielded);

BOOL try_pop_next_task_queue(next_task_queue* tasks_q, next_task* task) {
    spinlock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx 
        && tasks_q->write_round == tasks_q->read_round) {
        spinlock_unlock(&tasks_q->lock);
        puts("Next task queue is empty.");
        return FALSE;
    }
    memcpy(task, &tasks_q->tasks[tasks_q->read_idx], sizeof(*task));
    tasks_q->read_round ^= (tasks_q->read_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->read_idx = (tasks_q->read_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pop task from next tasks queue");
    spinlock_unlock(&tasks_q->lock);
    return TRUE;
}

BOOL try_pop_yielded_task_queue(yielded_task_queue* tasks_q, yielded_task* task) {
    spinlock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx 
        && tasks_q->write_round == tasks_q->read_round) {
        force_yielded = FALSE;
        spinlock_unlock(&tasks_q->lock);
        puts("Yielded task queue is empty.");
        return FALSE;
    }
    memcpy(task, &tasks_q->tasks[tasks_q->read_idx], sizeof(*task));
    tasks_q->read_round ^= (tasks_q->read_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->read_idx = (tasks_q->read_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pop task from yielded tasks queue");
    spinlock_unlock(&tasks_q->lock);
    return TRUE;
}

BOOL try_push_next_task_queue(next_task_queue* tasks_q, next_task* task) {
    spinlock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        spinlock_unlock(&tasks_q->lock);
        puts("Next task queue is full.");
        return FALSE;
    }
    memcpy(&tasks_q->tasks[tasks_q->write_idx], task, sizeof(*task));
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pushed task to the next_queue");
    spinlock_unlock(&tasks_q->lock);
    return TRUE;
}    

BOOL try_push_yielded_task_queue(yielded_task_queue* tasks_q, semaphore* sem) {
    spinlock_lock(&tasks_q->lock);
    if (tasks_q->write_idx == tasks_q->read_idx
            && tasks_q->write_round != tasks_q->read_round) {
        force_yielded = TRUE;
        spinlock_unlock(&tasks_q->lock);
        puts("Yielded task queue is full.");
        return FALSE;
    }
    tasks_q->write_round ^= (tasks_q->write_idx == (MAX_QUEUE_LENGHT - 1));
    const int write_idx = tasks_q->write_idx ;
    tasks_q->write_idx = (tasks_q->write_idx + 1) & (MAX_QUEUE_LENGHT - 1);
    puts("Pushing task to the yielded_queue");
    co_yield_impl(&tasks_q->tasks[write_idx], &tasks_q->lock, sem); // we also signal the semaphore and unlock the lock here
    return TRUE;
}    

#endif // _QUEUE_H_
