/**
   avg-mult-thread.c

   A program for finding the average value of a set of random numbers with 
   multiple POSIX threads spawned from the main thread.

   usage: avg-mult-thread count num_threads
   count: the number of random values to generate
   num_threads: the number of threads

   Adoted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and refactoring for readability.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

const char *usage = "usage: avg-mult-thread count num_threads";
#define RAND() (drand48()) //basic Linux random number generator

typedef struct{
  int id;
  int size;
  int start;
  double *data; //pointer to parent data
} thread_arg_t;

typedef struct{
  double sum;
} thread_result_t;

void *sum_thread(void *arg){
  thread_arg_t *a = arg;
  thread_result_t *r = malloc(sizeof(thread_result_t));
  assert(r != NULL);
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
  free(a); //data block still allocated
  return r;
}

int main(int argc, char **argv){
  pthread_t *thread_ids = NULL;
  thread_arg_t *a = NULL;
  thread_result_t *r = NULL;
  int count, num_thread_elts, num_threads, err;
  int start = 0;
  double sum = 0.0;
  double *data = NULL; //parent data block
  //input checking
  if (argc <= 2){
    fprintf(stderr,"must specify count and number of threads\n %s",
	    usage);
    exit(1);
  }
  count = atoi(argv[1]);
  if (count <= 0){
    fprintf(stderr,"invalid count %d\n", count);
    exit(1);
  }
  num_threads = atoi(argv[2]);
  if (num_threads <= 0){
    fprintf(stderr,"invalid thread count %d\n", num_threads);
    exit(1);
  }
  if (num_threads > count){
    num_threads = count;
  }
  //create thread ids and data
  thread_ids = malloc(sizeof(pthread_t) * num_threads);
  assert(thread_ids != NULL);
  if ((count / num_threads) * num_threads < count){
    num_thread_elts = count / num_threads + 1;
  }else{
    num_thread_elts = count / num_threads;
  }
  data = malloc(count * sizeof(double));
  assert(data != NULL);
  for (int i = 0; i < count; i++){
    data[i] = RAND();
  }
  //create threads each with a separately allocated argument struct
  printf("main thread about to create %d sum threads\n", num_threads);
  fflush(stdout);
  for (int t = 0; t < num_threads; t++){
    a = malloc(sizeof(thread_arg_t));
    assert(a != NULL);
    a->id = t;
    if (start + num_thread_elts > count){
      a->size = count - start;
    }else{
      a->size = num_thread_elts;
    }
    a->start = start;
    a->data = data;
    printf("main thread creating sum thread %d\n", t);
    fflush(stdout);
    err = pthread_create(&(thread_ids[t]), NULL, sum_thread, (void *)a);
    assert(err == 0);
    printf("main thread has created sum thread %d\n", t);
    fflush(stdout);
    start += num_thread_elts;
  }
  //join main with each sum thread one at a time
  for (int t = 0; t < num_threads; t++){
    printf("main thread about to join with sum thread %d\n", t);
    fflush(stdout);
    err = pthread_join(thread_ids[t],(void **)&r);
    printf("main thread joined with sum thread %d\n", t);
    fflush(stdout);
    sum += r->sum;
    free(r);
  }
  printf("the average over %d random numbers on (0,1) is %f\n",
	 count, sum / (double)count);
  free(thread_ids);
  free(data);
  return 0;
}

  
