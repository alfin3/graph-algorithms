/**
   utilities-concur.c

   Utility functions for concurrency, including
   1) pthread synchronization functions with wrapped error checking, and
   2) implementation of semaphore operations based on 1),
   adopted from The Little Book of Semaphores by Allen B. Downey
   (Version 2.2.1) with modifications.
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utilities-concur.h"

/** Create and join threads */

void pthread_create_perror(pthread_t *thread,
			   void *(*start_routine)(void *),
			   void *arg){
  int err = pthread_create(thread, NULL , start_routine, arg);
  if (err != 0){
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }
}

void pthread_join_perror(pthread_t thread, void **retval){
  int err = pthread_join(thread, retval);
  if (err != 0){
    perror("pthread_join failed");
    exit(EXIT_FAILURE);
  }
}

/** Initialize, lock, and unlock mutex */

void pthread_mutex_init_perror(pthread_mutex_t *mutex){
  int err = pthread_mutex_init(mutex, NULL);
  if (err != 0){
    perror("pthread_mutex_init failed");
    exit(EXIT_FAILURE);
  }
}

void pthread_mutex_lock_perror(pthread_mutex_t *mutex){
  int err = pthread_mutex_lock(mutex);
  if (err != 0){
    perror("pthread_mutex_lock failed");
    exit(EXIT_FAILURE);
  }
}

void pthread_mutex_unlock_perror(pthread_mutex_t *mutex){
  int err = pthread_mutex_unlock(mutex);
  if (err != 0){
    perror("pthread_mutex_unlock failed");
    exit(EXIT_FAILURE);
  }
}

/** Initialize, wait on, and signal a condition */

void pthread_cond_init_perror(pthread_cond_t *cond){
  int err = pthread_cond_init(cond, NULL);
  if (err != 0){
    perror("pthread_cond_init failed");
    exit(EXIT_FAILURE);
  }
}

void pthread_cond_wait_perror(pthread_cond_t *cond, pthread_mutex_t *mutex){
  int err = pthread_cond_wait(cond, mutex);
  if (err != 0){
    perror("pthread_cond_wait failed");
    exit(EXIT_FAILURE);
  }
}

void pthread_cond_signal_perror(pthread_cond_t *cond){
  int err = pthread_cond_signal(cond);
  if (err != 0){
    perror("pthread_cond_signal failed");
    exit(EXIT_FAILURE);
  }
}

/** Initialize, wait on, and signal a semaphore */

void sema_init_perror(semaphore_t *sema, int value){
  sema->value = value;
  sema->num_wakeups = 0;
  pthread_mutex_init_perror(&sema->mutex);
  pthread_cond_init_perror(&sema->cond);
}

void sema_wait_perror(semaphore_t *sema){
  pthread_mutex_lock_perror(&sema->mutex);
  sema->value--;
  if (sema->value < 0){
    do{
      pthread_cond_wait_perror(&sema->cond, &sema->mutex);
    }while (sema->num_wakeups < 1);
    sema->num_wakeups--;
  }
  pthread_mutex_unlock_perror(&sema->mutex);
}

void sema_signal_perror(semaphore_t *sema){
  pthread_mutex_lock_perror(&sema->mutex);
  sema->value++;
  if (sema->value <= 0){
    sema->num_wakeups++;
    pthread_cond_signal_perror(&sema->cond);
  }
  pthread_mutex_unlock_perror(&sema->mutex);
}
