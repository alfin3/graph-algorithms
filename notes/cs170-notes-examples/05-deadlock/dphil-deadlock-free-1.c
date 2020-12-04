/**
   dphil-deadlock-free-1.c

   A solution of the "Dining Philosophers" problem. In the
   provided implementationm threads with odd ids acquire the first mutex
   on the left, and threads with even ids acquire the first mutex on the
   right. The pairs odd - right first and even - left first are also
   correct. A deadlock cannot occur.

   Correctness (# threads > 1):

   At any time, there exists a thread A that will acquire or has acquired
   its first mutex.
   case 1: A is odd and the adjacent threads are even, or A is even and
           the adjacent threads are odd. If A waits for the second mutex,
           then an adjacent thread has acquired this mutex as its second
           mutex and will release it. Thus, A will acquire its second 
           mutex.
   case 2: A is odd and an adjacent thread B is odd, or A is even and an
           adjacent thread B is even.
           a) The first mutex of A is the second mutex of B. The second
              mutex of A is the second mutex of another thread. Thus, A
              will acquire its second mutex.
           b) The second mutex of A is the first mutex of B. If A waits
              for its second mutex, then B acquired it. The second mutex
              of B is the second mutex of another thread. Thus, B will
              release its first mutex. A will acquire its second mutex.
   
   Fairness:

   The solution is prone to unfair treatment of threads. In the setting
   with five threads, thread 4 has an unfair advantage over other threads
   and is blocked for less time, because its first mutex is the second
   mutex of thread 0, whereas for all other threads the first mutex is
   also the first mutex of another thread. A second mutex has a smaller
   critical section than a first mutex.
   
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
      error checking,
   -  a correctness proof is provided.
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
  if (id & 1){
    mutex_lock_perror(f->lock[id]); //lock the left fork
    sleep(inter_lock_time);
    mutex_lock_perror(f->lock[(id + 1) % f->num_phil]); //lock the right fork
  }else{
    mutex_lock_perror(f->lock[(id + 1) % f->num_phil]); //lock the right fork
    sleep(inter_lock_time);
    mutex_lock_perror(f->lock[id]); //lock the left fork
  }
}

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id){
  forks_t *f = state;
  if (id & 1){
    mutex_unlock_perror(f->lock[(id + 1) % f->num_phil]); //lock the right fork
    sleep(inter_lock_time);
    mutex_unlock_perror(f->lock[id]); //lock the left fork
  }else{
    mutex_unlock_perror(f->lock[id]); //lock the left fork
    sleep(inter_lock_time);
    mutex_unlock_perror(f->lock[(id + 1) % f->num_phil]); //lock the right fork
    
  }
}
