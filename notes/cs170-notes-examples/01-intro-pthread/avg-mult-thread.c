/**
   avg-mult-thread.c

   A program for finding the average value of a set of random numbers with 
   multiple POSIX threads spawned from the main thread.

   usage: avg-mult-thread count num_threads
   count: the number of random values to generate
   num_threads: the number of threads
   usage example: avg-mult-thread 100000000 3

   Adoted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and refactoring.

   A thread argument block is deallocated where it was previously allocated,
   consistent with the practice of deallocating where resources
   are allocated. However, the parent thread deallocates a result block 
   previously allocated by a child thread, consistent with 
   https://man7.org/linux/man-pages/man3/pthread_create.3.html
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "utilities-mem.h"
#include "utilities-concur.h"

const char *usage = "usage: avg-mult-thread count num_threads";
#define RAND() (drand48()) //basic Linux random number generator

typedef struct{
  int id;
  int size;
  int start;
  double *data; //parent data
} thread_arg_t;

typedef struct{
  double sum;
} thread_result_t;

void *sum_thread(void *arg){
  thread_arg_t *a = arg;
  thread_result_t *r = malloc_perror(sizeof(thread_result_t));
  r->sum = 0.0;
  printf("sum thread %d running, starting at %d for %d\n",
	 a->id,
	 a->start,
	 a->size);
  fflush(stdout);
  for (int i = a->start; i < a->start + a->size; i++){
    r->sum += a->data[i];
  }
  printf("sum thread %d done, returning\n", a->id);
  fflush(stdout);
  return r;
}

int main(int argc, char **argv){
  pthread_t *tid_arr = NULL;
  thread_arg_t *a_arr = NULL;
  thread_result_t *r = NULL;
  int count, num_thread_elts, num_threads;
  int start = 0;
  double sum = 0.0;
  double *data = NULL; //parent data block
  //input checking and initialization
  if (argc <= 2){
    fprintf(stderr,"must specify count and number of threads\n%s\n",
	    usage);
    exit(EXIT_FAILURE);
  }
  count = atoi(argv[1]);
  if (count <= 0){
    fprintf(stderr,"invalid count %d\n", count);
    exit(EXIT_FAILURE);
  }
  num_threads = atoi(argv[2]);
  if (num_threads <= 0){
    fprintf(stderr,"invalid thread count %d\n", num_threads);
    exit(EXIT_FAILURE);
  }
  if (num_threads > count){
    num_threads = count;
  }
  if ((count / num_threads) * num_threads < count){
    num_thread_elts = count / num_threads + 1;
  }else{
    num_thread_elts = count / num_threads;
  }
  tid_arr = malloc_perror(sizeof(pthread_t) * num_threads);
  a_arr = malloc_perror(sizeof(thread_arg_t) * num_threads);
  data = malloc_perror(count * sizeof(double));
  for (int i = 0; i < count; i++){
    data[i] = RAND();
  }
  //spawn threads
  printf("main thread about to create %d sum threads\n", num_threads);
  fflush(stdout);
  for (int i = 0; i < num_threads; i++){
    a_arr[i].id = i;
    if (start + num_thread_elts > count){
      a_arr[i].size = count - start;
    }else{
      a_arr[i].size = num_thread_elts;
    }
    a_arr[i].start = start;
    a_arr[i].data = data;
    printf("main thread creating sum thread %d\n", i);
    fflush(stdout);
    thread_create_perror(&tid_arr[i], sum_thread, &a_arr[i]);
    printf("main thread has created sum thread %d\n", i);
    fflush(stdout);
    start += num_thread_elts;
  }
  //join with main
  for (int i = 0; i < num_threads; i++){
    printf("main thread about to join with sum thread %d\n", i);
    fflush(stdout);
    thread_join_perror(tid_arr[i], (void **)&r);
    printf("main thread joined with sum thread %d\n", i);
    fflush(stdout);
    sum += r->sum;
    free(r); //result block allocated by a child thread
    r = NULL;
  }
  printf("the average over %d random numbers on (0,1) is %f\n",
	 count, sum / (double)count);
  free(tid_arr);
  free(a_arr);
  free(data);
  tid_arr = NULL;
  a_arr = NULL;
  data = NULL;
  return 0;
}

  
