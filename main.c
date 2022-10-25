#include "scheduler.h"

next_task_queue next_tq;
yielded_task_queue yielded_tq;

int main() {
    int frame = 0;
    while (1) {
        
        frame ^= 1; // schedule next/yielded tasks on even/odd frames
    }

    return EXIT_SUCCESS;
}
