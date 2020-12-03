/**
   dphil-driver.c

   Driver functions for solutions of the "Dining Philosophers" problem.

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters, and the thread argument struct is moved to
      dphil-driver.c, where the thread entry function is defined,
   -  some variables were renamed or eliminated; some functions were
      renamed,
   -  memory allocation and pthread functions are used with wrapped
      error checking.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "dphil.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

typedef struct{
  int id;
  int num_phil;
  long start_time;       //program start time
  long max_dur;          //max time for thinking/eating
  long *block_times;     //total time each thread is blocked
  void *state;           //sync. state wrt the pickup and putdown ops
  pthread_mutex_t *block_times_lock; //monitor for updating block times
} phil_arg_t;

void *phil_thread(void *arg){
  phil_arg_t *pa = arg;
  long t;
  while (true){
    //think
    t = random() % pa->max_dur + 1;
    printf("%3ld Philosopher %d thinking for %ld seconds\n", 
                time(0) - pa->start_time, pa->id, t);
    fflush(stdout);
    sleep(t);
    //pick up
    printf("%3ld Philosopher %d calling state_pickup\n", 
            time(0) - pa->start_time, pa->id);
    fflush(stdout);
    t = time(0);
    state_pickup(pa->state, pa->id);
    mutex_lock_perror(pa->block_times_lock);
    pa->block_times[pa->id] += time(0) - t;
    mutex_unlock_perror(pa->block_times_lock);
    //eat
    t = random() % pa->max_dur + 1;
    printf("%3ld Philosopher %d eating for %ld seconds\n", 
                time(0) - pa->start_time, pa->id, t);
    fflush(stdout);
    sleep(t);
    //put down
    printf("%3ld Philosopher %d calling state_putdown\n", 
            time(0) - pa->start_time, pa->id);
    fflush(stdout);
    state_putdown(pa->state, pa->id);
  }
}

int main(int argc, char **argv){
  phil_arg_t pa[MAX_NUM_THREADS];
  pthread_t phil_thread_ids[MAX_NUM_THREADS];
  pthread_mutex_t *block_times_lock = NULL;
  int num_phil;
  int print_interval = 10;
  long start_time = time(0);
  long *block_times = NULL;
  long total_block_time = 0;
  char s[500], *curr = NULL;
  void *state = NULL;
  if (argc != 3) {
    fprintf(stderr, "usage: executable_name num_phil maxsleepsec\n");
    exit(EXIT_FAILURE);
  }
  num_phil = atoi(argv[1]);
  if (num_phil > MAX_NUM_THREADS) num_phil = MAX_NUM_THREADS;
  block_times = malloc_perror(sizeof(long) * num_phil);
  for (int i = 0; i < num_phil; i++) block_times[i] = 0;
  state = state_new(num_phil);
  block_times_lock = malloc_perror(sizeof(pthread_mutex_t));
  mutex_init_perror(block_times_lock);
  srandom(time(0));
  for (int i = 0; i < num_phil; i++){
    pa[i].id = i;
    pa[i].start_time = start_time;;
    pa[i].max_dur = atoi(argv[2]);
    pa[i].block_times = block_times;
    pa[i].state = state;
    pa[i].block_times_lock = block_times_lock;
    thread_create_perror(phil_thread_ids + i, phil_thread, pa + i);
  }
  while (true){
    //exit and free resources with Ctrl+C
    mutex_lock_perror(block_times_lock);
    curr = s;
    for(int i = 0; i < num_phil; i++) total_block_time += block_times[i];
    sprintf(curr,"%3ld Total blocktime: %5ld : ", 
		    time(0) - start_time, total_block_time);
    curr = s + strlen(s);
    for(int i = 0; i < num_phil; i++){
    	sprintf(curr, "%5ld ", block_times[i]);
	curr = s + strlen(s);
    }
    mutex_unlock_perror(block_times_lock);
    printf("%s\n", s);
    fflush(stdout);
    sleep(print_interval);
  }
 //no dellocation performed due to Ctrl+C
}
