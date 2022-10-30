#include "test_task.h"

#define FRAMES_LIM 60

int main() {
   // init the queues
   init_next_queue(&next_tq);
   init_yielded_queue(&yielded_tq);
   // init the stacks
   init_stacks();
   // init scheduler
   init_scheduler();
   init_workers();
   resume_all_workers();

    int total_frames = 0;
    int frame = 0;
    do {
        random_task_select(&frame);
        total_frames++;
    } while (total_frames != FRAMES_LIM);

    stop_workers = TRUE;
    deinit_workers();
    deinit_scheduler();
    deinit_stacks();

    return EXIT_SUCCESS;
}
