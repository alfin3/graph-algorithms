/**
   race-condition-1.c

   usage: race-condition-1 nthreads stringsize iterations
   count: the number of random values to generate

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

void *infloop(void *arg){
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
  void *r = NULL; //pointer buffer for pthread_join
  int nthreads, size, iterations, err;
  char *s = NULL;
  if (argc != 4){
    fprintf(stderr,"%s\n", usage);
    exit(1);
  }
  nthreads = atoi(argv[1]);
  size = atoi(argv[2]) + 1;
  iterations = atoi(argv[3]);
  tid_arr = malloc(sizeof(pthread_t) * nthreads);
  assert(tid_arr != NULL);
  a_arr = malloc(sizeof(thread_arg_t) * nthreads);
  assert(a_arr != NULL);
  s = malloc(sizeof(char) * size);
  assert(s != NULL);
  for (int i = 0; i < nthreads; i++){
    a_arr[i].id = i;
    a_arr[i].size = size;
    a_arr[i].iterations = iterations;
    a_arr[i].s = s; //pointer to the parent string
    err = pthread_create(&(tid_arr[i]), NULL, infloop, &(a_arr[i]));
    assert(err == 0);
  }
  for (int i = 0; i < nthreads; i++){
    err = pthread_join(tid_arr[i], &r);
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
