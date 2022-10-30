#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "scheduler.h"

enum test_data {
    td_max_tasks_per_frame = NUM_WORKERS,
    td_rnd_prob = 9,
    td_rnd_max = 16,
    td_active_frames = 8,
};

void critical_task(void*) {
    pid_t tid = syscall(SYS_gettid);
    printf("Critical task executed from start to end on thread %d\n", tid);
}

void waiting_task(void*) {
    pid_t tid = syscall(SYS_gettid);
    printf("Low prio task started execution on thread %d\n", tid);
    
    if (try_save_and_yield()) {
        printf("Finish execution of a low prio task on thread %d\n", tid);
    } else {
        printf("Low prio task unable to yield execution: finishing now on thread %d\n", tid);
    }

}

void random_task_select(int* frame) {
    const int num_tasks = 1 + rand() % td_max_tasks_per_frame;
    int i = 0;
    for (; i < num_tasks; i++) {
        const int selector = rand() % td_rnd_max;
        next_task nt;
        nt.f  = (selector <= td_rnd_prob) ? critical_task : waiting_task;
        nt.params = NULL;
        if (!add_next_task(&nt)) {
            break;
        }
    }
    printf("Pushed %d new tasks to the queue\n", i);
    sleep(1);

    *frame = (*frame + 1) % td_active_frames;
    if (*frame == 0x00) {
        stop_all_workers();
        puts("Stop all workers. Wait...");
        sleep(3);
        puts("Resume all workers");
        force_yielded = TRUE; // force to complete all the yielded tasks
        resume_all_workers();
    }
}

