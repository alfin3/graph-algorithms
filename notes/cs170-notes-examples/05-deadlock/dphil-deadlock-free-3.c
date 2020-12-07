/**
   dphil-deadlock-free-3.c

   A solution of the "Dining Philosophers" problem. In the provided
   implementation, a fifo queue is used to ensure (outside the scheduler)
   that the first thread to acquire mutex after calling state_pickup is
   also the first to "eat", guaranteeing non-starvation at the expense of
   performance. A thread may be able to "eat" but must wait for its turn.

   Correctness:

   Every thread pushed onto a queue is “thinking”. Thus, the first thread
   (front) in the queue can only be blocked by the threads that are “eating”
   and are not in the queue. The first thread will be popped.

   Thread starvation:

   The fifo queue guarantees that every waiting thread will make progress
   and will not starve.

   The functions for creating a thread synchronization state and pickup and
   putdown operations are called from the driver implemented in
   dphil-driver.c.

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters,
   -  a generic fifo queue is used,
   -  a single while loop is used for waiting for the satisfaction of 
      the conjuction of three predicates (negated disjunction of negated
      predicates), each signaled separately, reducing code complexity
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
#include "queue-uint64.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  int num_phil;
  bool thinking[MAX_NUM_THREADS];
  queue_uint64_t q; //fifo queue of integer thread ids
  pthread_cond_t cond_first_adj_thinking[MAX_NUM_THREADS];
  pthread_mutex_t lock;
} state_t;

/**
   Creates a new state for handling thread synchronization wrt the
   pickup and putdown operations.
*/
void *state_new(int num_phil){
  state_t *s = malloc_perror(sizeof(state_t));
  s->num_phil = num_phil; 
  for (int i = 0; i < s->num_phil; i++){
    s->thinking[i] = true;
    cond_init_perror(&s->cond_first_adj_thinking[i]);
  }
  queue_uint64_init(&s->q, 1, sizeof(int), NULL);
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
  state_t *s = state;
  int first_id;
  mutex_lock_perror(&s->lock);
  queue_uint64_push(&s->q, &id);
  first_id = *(int *)queue_uint64_first(&s->q);
  while (first_id != id ||
	 !s->thinking[(id + 1) % s->num_phil] ||
	 !s->thinking[(id + s->num_phil - 1) % s->num_phil]){
    cond_wait_perror(&s->cond_first_adj_thinking[id], &s->lock);
    first_id = *(int *)queue_uint64_first(&s->q);
  }
  queue_uint64_pop(&s->q, &first_id);
  s->thinking[id] = false;
  if (queue_uint64_first(&s->q) != NULL){
    //signal the new first thread
    first_id = *(int *)queue_uint64_first(&s->q);
    cond_signal_perror(&s->cond_first_adj_thinking[first_id]);
  }
  mutex_unlock_perror(&s->lock);
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  state_t *s = state;
  mutex_lock_perror(&s->lock);
  s->thinking[id] = true;
  cond_signal_perror(&s->cond_first_adj_thinking[(id + 1) % s->num_phil]);
  cond_signal_perror(&s->cond_first_adj_thinking[(id + s->num_phil - 1) %
						s->num_phil]);
  mutex_unlock_perror(&s->lock);
}
