/**
   avg-single-thread.c

   A program for finding the average value of a set of random numbers with 
   a single POSIX thread spawned from the main thread.

   usage: avg-single-thread count
   count: the number of random values to generate

   Adoted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and refactoring for readability.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

const char *usage = "usage: avg-single-thread count";
#define RAND() (drand48()) //basic Linux random number generator

typedef struct{
  int size;
  double *data;
} thread_arg_t;

typedef struct{
  double sum;
} thread_result_t;

void *sum_thread(void *arg){
  thread_arg_t *a = arg;
  thread_result_t *r = malloc(sizeof(thread_result_t));
  assert(r != NULL);
  r->sum = 0.0;
  printf("sum thread running\n");
  fflush(stdout);
  for (int i = 0; i < a->size; i++){
    r->sum += a->data[i];
  }
  free(a); //data block still allocated
  printf("sum thread done, returning\n");
  fflush(stdout);
  return r;
}

int main(int argc, char **argv){
  pthread_t thread_id;
  thread_arg_t *a = malloc(sizeof(thread_arg_t));
  assert(a != NULL);
  thread_result_t *r;
  int count, err;
  double *data = a->data; //for freeing data block in main
  if (argc <= 1){
    fprintf(stderr,"must specify count\n %s", usage);
    exit(1);
  }
  count = atoi(argv[1]);
  if (count <= 0){
    fprintf(stderr,"invalid count %d\n", count);
    exit(1);
  }
  a->size = count;
  a->data = malloc(a->size * sizeof(double));
  assert(a->data != NULL);
  for (int i = 0; i < count; i++){
    a->data[i] = RAND();
  }
  printf("main thread forking sum thread\n");
  fflush(stdout);
  //a block allocated in main, freed in sum
  //r block allocated in sum, freed in main
  err = pthread_create(&thread_id, NULL, sum_thread, (void *)a);
  assert(err == 0);
  printf("main thread running after sum thread created, "
	 "about to call join\n");
  fflush(stdout);
  //thread return value is copied to the pointer block pointed to by &r
  err = pthread_join(thread_id, (void **)&r); //main waits until joined
  assert(err == 0);
  printf("main thread joined with sum thread\n");
  fflush(stdout);
  printf("the average over %d random numbers on (0,1) is %f\n",
	 count, r->sum / (double)count);
  free(r);
  free(data);
  return 0;
}
