#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "semaphore.h"
#include "queue.h"
#include "stacks.h"

//#define NUM_WORKERS 4

extern void launch_task(void* params, task t, u8*stack_ptr, u64 stack_id);
extern void resume_yielded_task(yielded_task*);

volatile BOOL stop_workers = FALSE;

semaphore sem_sync;  // sync point
semaphore sem_empty_queue; // wait on empty queue

pthread_t workers[NUM_WORKERS];

//typedef struct thread_stack_ptr {
//    pid_t thread_id;
//    u8* stack_ptr;
//} thread_stack_ptr;
//
//thread_stack_ptr tsp[NUM_WORKERS];

next_task_queue next_tq;
yielded_task_queue yielded_tq;

BOOL add_next_task(next_task* task) {
    if (try_push_next_task_queue(&next_tq, task)) {
        semaphore_signal(&sem_empty_queue);
        return TRUE;
    }
    return FALSE;
}

BOOL try_co_yield() {
    return try_push_yielded_task_queue(&yielded_tq, &sem_empty_queue);
}

void* execute_task(void*); 

void init_scheduler() {
    semaphore_init(&sem_sync);
    semaphore_init(&sem_empty_queue);
}

void deinit_scheduler() {
    semaphore_deinit(&sem_sync);
    semaphore_deinit(&sem_empty_queue);
}

void init_workers() {
    int result_code = 0;
    for (int i = 0; i < NUM_WORKERS; i++) {
        result_code |= pthread_create(&workers[i], NULL, execute_task, NULL);
    }
    assert(!result_code);
}

void deinit_workers() {
    int result_code = 0;
    for (int i = 0; i < NUM_WORKERS; ++i) {
        result_code = pthread_join(workers[i], NULL);
        assert(!result_code);
    }
}

void stop_all_workers() {
    sem_sync.do_wait = TRUE;
}

void resume_all_workers() {
    semaphore_signal_all(&sem_sync);
}


//static __thread BOOL thread_stack_ptr_set;
//
//attribute__((always_inline)) void set_stack_ptr_once() {
//    assert(sizeof(pid_t <= sizeof(u64)));
//    if (!thread_stack_ptr_set) {
//        const pid_t tid = syscall(SYS_gettid);
//        int i = 0;
//        while (!atomic_cmpxchg(&tsp[i].thread_id, 0, tid)) {
//            i++;
//        }        
//        tsp[i].stack_ptr = set_thread_stack_ptr;
//        thread_stack_ptr_set = TRUE; 
//    }
//}
//
//u8* get_original_stack_ptr() {
//    const pid_t tid = syscall(SYS_gettid);
//    int i = 0;
//    while (tid != tps[i].thread_id) {
//        i++;
//    }
//    return tps[i].stack_ptr;
//}

static __thread u8* thread_stack_ptr;

void __attribute__ ((noinline)) set_thread_stack_ptr(u8* rsp) {
    if (!thread_stack_ptr)
        thread_stack_ptr = rsp;
}

typedef struct exec_state {
    void* execute_task_address;
    u8* thread_stack_ptr;
} exec_state;

void restore_state(exec_state* state) {
    state->execute_task_address = (void*)execute_task;
    state->thread_stack_ptr = thread_stack_ptr;    
}

void* execute_task_inner(void*) {
    u8* stack_ptr;
    u64 stack_id;
    next_task n_task;
    yielded_task y_task;
    while (!stop_workers) {
        // wait for schedulers signal to continue
        semaphore_try_wait(&sem_sync);
        // try executing next task 
        if (!force_yielded && try_pop_next_task_queue(&next_tq, &n_task)
                && get_free_stack(&stack_ptr, &stack_id)) {
            printf("Acquire stack ID: %d\n", (int)stack_id);
            puts("Launch next task");
            launch_task(n_task.params, n_task.f, stack_ptr, stack_id);
        } else if (try_pop_yielded_task_queue(&yielded_tq, &y_task)) {   // or finish a previously started task
            puts("Resume yielded task");
            resume_yielded_task(&y_task); 
        } else {
            // if both queues are empty - wait
            puts("Waitings until new tasks are available");
            semaphore_wait(&sem_empty_queue);
        }
    }
    return NULL;
}

#endif //  _SCHEDULER_H_
