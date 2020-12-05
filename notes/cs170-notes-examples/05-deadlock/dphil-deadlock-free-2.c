/**
   dphil-deadlock-free-2.c

   A solution of the "Dining Philosophers" problem. In the provided
   implementation, a thread waits on the condition that both adjacent
   threads are "thinking" (condition variable for each thread). A
   deadlock cannot occur. Threads are treated fairly according to the
   scheduler.

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
#include <stdbool.h>
#include <pthread.h>
#include "dphil.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  int num_phil;
  bool thinking[MAX_NUM_THREADS];
  pthread_cond_t cond_adj_thinking[MAX_NUM_THREADS];
  pthread_mutex_t lock;
} status_t;

/**
   Creates a new state for handling thread synchronization wrt the
   pickup and putdown operations.
*/
void *state_new(int num_phil){
  status_t *s = malloc_perror(sizeof(status_t));
  s->num_phil = num_phil; 
  for (int i = 0; i < s->num_phil; i++){
    s->thinking[i] = true;
    cond_init_perror(&s->cond_adj_thinking[i]);
  }
  mutex_init_perror(&s->lock);
  return s;
}

/**
   Disposes a state for thread synchronization, freeing memory resources.
   Currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id){
  status_t *s = state;
  mutex_lock_perror(&s->lock);
  while (!s->thinking[(id + 1) % s->num_phil] ||
	 !s->thinking[(id + s->num_phil - 1) % s->num_phil]){
    cond_wait_perror(&s->cond_adj_thinking[id], &s->lock);
  }
  s->thinking[id] = false;
  mutex_unlock_perror(&s->lock);
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  status_t *s = state;
  mutex_lock_perror(&s->lock);
  s->thinking[id] = true;
  cond_signal_perror(&s->cond_adj_thinking[(id + 1) % s->num_phil]);
  cond_signal_perror(&s->cond_adj_thinking[(id + s->num_phil - 1) %
					   s->num_phil]);
  mutex_unlock_perror(&s->lock);
}
