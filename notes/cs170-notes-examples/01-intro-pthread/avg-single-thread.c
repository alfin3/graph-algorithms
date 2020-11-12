/**
   avg-single-thread.c

   A program for finding the average value of a set of random numbers with 
   a single POSIX thread spawned from the main thread.

   usage: avg-single-thread count
   count: the number of random values to generate

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

const char *usage = "usage: avg-single-thread count";
#define RAND() (drand48()) //basic Linux random number generator

typedef struct{
  int size;
  double *data; //parent data
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
  printf("sum thread done, returning\n");
  fflush(stdout);
  return r;
}

int main(int argc, char **argv){
  pthread_t tid;
  thread_arg_t *a = NULL;
  thread_result_t *r = NULL;
  int count, err;
  double *data = NULL; //parent data block
  //input checking and initialization
  if (argc <= 1){
    fprintf(stderr,"must specify count\n%s\n", usage);
    exit(1);
  }
  count = atoi(argv[1]);
  if (count <= 0){
    fprintf(stderr,"invalid count %d\n", count);
    exit(1);
  }
  a = malloc(sizeof(thread_arg_t));
  assert(a != NULL);
  data = malloc(count * sizeof(double));
  assert(data != NULL);
  for (int i = 0; i < count; i++){
    data[i] = RAND();
  }
  //spawn a thread
  printf("main thread forking sum thread\n");
  fflush(stdout);
  a->size = count;
  a->data = data;
  err = pthread_create(&tid, NULL, sum_thread, a);
  assert(err == 0);
  printf("main thread running after sum thread created, "
	 "about to call join\n");
  fflush(stdout);
  //join with main
  //thread return value is copied to the pointer block pointed to by &r
  err = pthread_join(tid, (void **)&r); //main waits until joining
  assert(err == 0);
  printf("main thread joined with sum thread\n");
  fflush(stdout);
  printf("the average over %d random numbers on (0,1) is %f\n",
	 count, r->sum / (double)count);
  free(r); //result block allocated by a child thread
  free(a);
  free(data);
  r = NULL;
  a = NULL;
  data = NULL;
  return 0;
}
