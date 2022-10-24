#include "scheduler.h"

int main() {
    int frame = 0;
    while (1) {
        
        frame ^= 1; // schedule next/yielded tasks on even/odd frames
    }

    return EXIT_SUCCESS;
}
