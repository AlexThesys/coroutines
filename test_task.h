#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "scheduler.h"

#define ACTIVE_FRAMES_MASK 0x1f

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
    *frame = (*frame + 1) & (ACTIVE_FRAMES_MASK);
    if (*frame == 0x00) {
        stop_all_workers();
        puts("Stop all workers. Wait...");
        sleep(4);
        puts("Resume all workers");
        resume_all_workers();
    }

    const int selector = rand() & 0x0F;
    next_task nt;
    nt.f  = (selector > 0x09) ? critical_task : waiting_task;
    nt.params = NULL;
    if (!add_next_task(&nt)) {
        puts("Failed pushing new task to the queue");
    }
    sleep(1);
}

