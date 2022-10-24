#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H_

#include <pthread>

typedef struct semaphore {
    volatile BOOL do_wait;
    pthread_cond_ti cond;
    pthread_mutex_t wait_mtx;
} semaphore;

void semaphore_init(semaphore* sem) {
    sem->do_wait = TRUE;
    sem->cond = PTHREAD_COND_INITIALIZER;
    sem->wait_mtx = PTHREAD_MUTEX_INITIALIZER;
}

void semaphore_deinit(semaphore* sem) {
    pthread_cond_destroy(&sem->cond);
    pthread_mutex_destroy(&sem->wait_mtx);
}

void semaphore_try_wait(semaphore* sem) {
    pthread_mutex_lock(&sem->wait_mtx);
    while (sem->do_wait) {
        pthread_cond_wait(&sem->cond, &sem->wait_mtx);
    }
    pthread_mutex_unlock(&sem->wait_mtx);
}

void semaphore_wait(semaphore* sem) {
    pthread_mutex_lock(&sem->wait_mtx);
    sem->do_wait = TRUE;
    while (sem->do_wait) {
        pthread_cond_wait(&sem->cond, &sem->wait_mtx);
    }
    pthread_mutex_unlock(&sem->wait_mtx);
}

void semaphore_signal(semaphore* sem) {
    pthread_mutex_lock(&sem->wait_mtx);
    sem->do_wait = FALSE;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->wait_mtx;

}

void semaphore_signal_all(semaphore* sem) {
    pthread_mutex_lock(&sem->wait_mtx);
    sem->do_wait = FALSE;
    pthread_cond_broadcast(&sem->cond);
    pthread_mutex_unlock(&sem->wait_mtx;
}

#endif // _SEMAPHORE_H_
