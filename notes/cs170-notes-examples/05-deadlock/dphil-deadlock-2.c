/**
   dphil-deadlock-2.c

   A solution of the "Dining Philosophers" problem. The implementation
   is prone to deadlock, which is likely to be reached due to an increased
   probability of preemption resulting from a call to sleep after acquiring
   the first mutex and before acquiring the second mutex in state_pickup.
   
   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   dphil-driver.c.

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters,
   -  some variables are renamed or eliminated; some functions are
      renamed,
   -  memory allocation and pthread functions are used with wrapped
      error checking.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "dphil.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  int num_phil;
  pthread_mutex_t *lock[MAX_NUM_THREADS];
} forks_t;

const long inter_lock_time = 3;

/**
   Creates a new state for handling thread synchronization wrt the
   pickup and putdown operations.
*/
void *state_new(int num_phil){
  forks_t *f = malloc_perror(sizeof(forks_t));
  f->num_phil = num_phil;
  for (int i = 0; i < f->num_phil; i++){
    f->lock[i] =  malloc_perror(sizeof(pthread_mutex_t));
    mutex_init_perror(f->lock[i]);
  }
  return f;
}

/**
   Disposes a state for thread synchronization, freeing memory resources.
   Currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id){
  forks_t *f = state;
  mutex_lock_perror(f->lock[id]); //lock the left fork
  sleep(inter_lock_time);
  mutex_lock_perror(f->lock[(id + 1) % f->num_phil]); //lock the right fork
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  forks_t *f = state;
  mutex_unlock_perror(f->lock[(id + 1) % f->num_phil]); //unlock the right fork
  mutex_unlock_perror(f->lock[id]);  //unlock the left fork
}
