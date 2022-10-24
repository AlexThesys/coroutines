#indef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <unistd.h>

#include "queue.h"
#include "stacks.h"
#include "semaphore.h"

#define NUM_WORKERS 4

extern void save_and_yield_impl(yielded_task* yt);
extern void launch_task(void* params, task* t, u8*stack_ptr);

semaphore sem_sync;  // sync point
semaphore sem_empty_queue; // wait on empty queue

pthread_t workers[NUM_WORKERS];

BOOL add_next_task(next_task* task) {
    if (try_push_next_task_queue(&next_task_queue, task)) {
        semaphore_signal(sem_empty_queue);
        return TRUE;
    }
    return FALSE;
}

BOOL try_save_and_yield() {
    if (try_push_yielded_task_queue(&yielded_task_queue)) {
        semaphore_signal(sem_empty_queue);
        return TRUE;
    }
    return FALSE;
}

void execute_task(); 

void init_scheduler() {
    init_semaphore(&sem_sync);
    init_semaphore(&sem_empty_queue);
}

void deinit_scheduler() {
    deinit_semaphore(&sem_sync);
    deinit_semaphore(&sem_empty_queue);
}

void init_workers() {
    int result_code = 0;
    for (int i = 0; i < NUM_WORKERS; i++) {
        result_code |= pthread_create(&threads[i], NULL, execute_task, NULL);
    }
    assert(!result_code);
}

void deinit_workers() {
    for (int i = 0; i < NUM_WORKERS; ++i) {
        result_code = pthread_join(threads[i], NULL);
        assert(!result_code);
    }
}

void stop_all_workers() {
    sem_sync.do_wait = TRUE;
}

void resume_all_workers() {
    semaphore_signal_all(&sem_sync);
}

void execute_task() {
    u8* stack_ptr;
    u64 stack_id;
    next_task n_task;
    yielded_task y_task;
    while (TRUE) {
        // wait for schedulers signal to continue
        semaphore_try_wait(&sem_sync);
       // try executing next task 
        if (try_pop_next_task_queue(&next_task_queue, &n_task)
                && get_free_stack(&stack_ptr, &stack_id)) {
            launch_task(n_task->params, n_task->task, stack_ptr);
            set_free_stack(stack_id); // both next and yielded task are going to return here
        } else if (try_pop_yielded_task_queue(&yielded_task_queue, &y_task)) {   // or finish a previously started task
            resume_yielded_task(&y_task); 
        } else {
            // if both queues are empty - wait
            semaphore_wait(&sem_empty_queue);
        }
    }
}

#endif \\ _SCHEDULER_H_
