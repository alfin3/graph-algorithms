/**
   race-condition-1.c

   usage: race-condition-1 nthreads stringsize iterations
   usage example: race-conditions-1 4 100 5

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef struct{
  int id;
  int size;
  int iterations;
  char *s;
} thread_arg_t;

const char *usage =
  "usage: race-condition-1 nthreads stringsize iterations\n";

void *thread_fn(void *arg){
  thread_arg_t *a = arg;
  for (int i = 0; i < a->iterations; i++){
    for (int j = 0; j < a->size - 1; j++){
      a->s[j] = 'A'+ a->id;
    }
    a->s[a->size - 1] = '\0';
    printf("thread %d: %s\n", a->id, a->s);
  }
  //pthread_exit(NULL);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t *tid_arr = NULL;
  thread_arg_t *a_arr = NULL;
  int num_threads, size, iterations, err;
  char *s = NULL;
  if (argc != 4){
    fprintf(stderr,"%s\n", usage);
    exit(1);
  }
  //initialize
  num_threads = atoi(argv[1]);
  size = atoi(argv[2]) + 1;
  iterations = atoi(argv[3]);
  tid_arr = malloc(sizeof(pthread_t) * num_threads);
  assert(tid_arr != NULL);
  a_arr = malloc(sizeof(thread_arg_t) * num_threads);
  assert(a_arr != NULL);
  s = malloc(sizeof(char) * size);
  assert(s != NULL);
  //spawn threads
  for (int i = 0; i < num_threads; i++){
    a_arr[i].id = i;
    a_arr[i].size = size;
    a_arr[i].iterations = iterations;
    a_arr[i].s = s; //pointer to the parent string
    err = pthread_create(&tid_arr[i], NULL, thread_fn, &a_arr[i]);
    assert(err == 0);
  }
  //join with main
  for (int i = 0; i < num_threads; i++){
    err = pthread_join(tid_arr[i], NULL);
    assert(err == 0);
  }
  free(tid_arr);
  free(a_arr);
  free(s);
  tid_arr = NULL;
  a_arr = NULL;
  s = NULL;
  return 0;
}
