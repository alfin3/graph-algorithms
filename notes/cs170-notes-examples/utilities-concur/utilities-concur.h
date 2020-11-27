/**
   utilities-concur.h

   Declarations of accessible utility functions for concurrency, including
   1) pthread synchronization functions with wrapped error checking, and
   2) implementation of semaphore operations based on 1),
   adopted from The Little Book of Semaphores by Allen B. Downey
   (Version 2.2.1) with modifications.
*/

#ifndef UTILITIES_CONCUR_H
#define UTILITIES_CONCUR_H

#include <pthread.h>

typedef struct{
  int value , num_wakeups;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} semaphore_t;

/** Create and join threads */

void pthread_create_perror(pthread_t *thread,
			   void *(*start_routine)(void *),
			   void *arg);

void pthread_join_perror(pthread_t thread, void **retval);

/** Initialize, lock, and unlock mutex */

void pthread_mutex_init_perror(pthread_mutex_t *mutex);

void pthread_mutex_lock_perror(pthread_mutex_t *mutex);

void pthread_mutex_unlock_perror(pthread_mutex_t *mutex);

/** Initialize, wait on, and signal a condition */

void pthread_cond_init_perror(pthread_cond_t *cond);

void pthread_cond_wait_perror(pthread_cond_t *cond, pthread_mutex_t *mutex);

void pthread_cond_signal_perror(pthread_cond_t *cond);

/** Initialize, wait on, and signal a semaphore */

void sema_init_perror(semaphore_t *sema, int value);

void sema_wait_perror(semaphore_t *sema);

void sema_signal_perror(semaphore_t *sema);

#endif
