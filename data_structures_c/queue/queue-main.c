/**
   queue-main.c

   Examples of generic dynamicaly allocated queue.

   Through a user-defined deallocation function, the implementation provides 
   a dynamic set of any objects, in fifo queue form.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

/** 
    Dynamically allocated queue of integer elements. In this test example, 
    a pointer to element is passed to queue_push that fully copies the 
    element to element array of queue.
*/

void free_int_fn(void *a){} //every element is fully copied to element array

/**
   Prints a specified number of integer elements in element array of queue.
*/
void print_int_elts(void *elts, int start_ix, int n){
  for (int i = 0; i < n; i++){
    printf("%d ", *((int *)elts + start_ix + i));
  }
  printf("\n");
}

/**
   Prints all current integer elements in element array of queue.
*/
void print_all_int_elts(queue_t *q){
  printf("Element array: ");
  print_int_elts(q->elts, q->num_popped_elts, q->num_elts);
}

/**
   Pushes n elements into queue.
*/
void push_int_elts(queue_t *q, int n){
  print_all_int_elts(q);
  for (int i = 0; i < n; i++){
    queue_push(q, &i);
    print_all_int_elts(q);
  }
  printf("\n");
}

/**
   Pops all elements of queue.
*/
void pop_all_int_elts(queue_t *q){
  int a;
  while(q->num_elts > 0){
    queue_pop(q, &a);
    printf("E: %d \n", a);
    print_all_int_elts(q);
  }
  printf("\n");
}
/**
   Runs a test example of queue of integer elements.
*/
void run_int_queue_test(){
  printf("Running int queue test... \n\n");
  //initialize a queue
  queue_t q;
  int queue_init_size = 1;
  queue_init(&q,
	     queue_init_size,
	     sizeof(int),
	     free_int_fn);
  //push elements 
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  push_int_elts(&q, num_push);
  //pop all elements
  printf("Popping all elements... \n\n");
  pop_all_int_elts(&q);
  //push elements and free queue
  printf("Pushing %d elements again... \n\n", num_push);
  push_int_elts(&q, num_push);
  printf("Freeing queue... \n\n");
  queue_free(&q);
}

/** 
    Dynamically allocated queue of int_ptr_t elements. In this test example, 
    a pointer to an element pointer is passed to queue_push that copies 
    the element pointer to element array of queue.
*/

typedef struct{
  int *val;
} int_ptr_t;

void free_int_ptr_t_fn(void *a){
  int_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
  s = NULL;
}

/**
   Prints a specified number of integer values across int_ptr_t elements
   in element array of queue.
*/
void print_int_ptr_t_elts(void *elts, int start_ix, int n){
  int_ptr_t **s = elts;
  for (int i = 0; i < n; i++){
    int *val = (*(s + start_ix + i))->val;
    printf("%d ", *val);
  }
  printf("\n");
}

/**
   Prints integer values across current int_ptr_t elements in element array.
*/
void print_all_int_ptr_t_elts(queue_t *q){
  printf("Element array: ");
  print_int_ptr_t_elts(q->elts, q->num_popped_elts, q->num_elts);
}

/**
   Dynamically allocates and pushes n elements into queue.
*/
void push_int_ptr_t_elts(queue_t *q, int n){
  int_ptr_t *a;
  print_all_int_ptr_t_elts(q);
  for (int i = 0; i < n; i++){
    a = malloc(sizeof(int_ptr_t));
    assert(a != NULL);
    a->val = malloc(sizeof(int));
    assert(a->val != NULL);
    *(a->val) = i;
    queue_push(q, &a);
    print_all_int_ptr_t_elts(q);
    a = NULL;
  }
  printf("\n");
}

/**
   Pops all elements of the queue.
*/
void pop_all_int_ptr_t_elts(queue_t *q){
  int_ptr_t *a;
  while(q->num_elts > 0){
    queue_pop(q, &a);
    printf("E: %d \n", *(a->val));
    print_all_int_ptr_t_elts(q);
    free_int_ptr_t_fn(&a);
    a = NULL;
  }
  printf("\n"); 
}

/**
   Runs a test example of queue of int_ptr_t elements.
*/
void run_int_ptr_t_queue_test(){
  printf("Running int_ptr_t queue test... \n\n");
  //initialize a queue
  queue_t q;
  int queue_init_size = 1;
  queue_init(&q,
	     queue_init_size,
	     sizeof(int_ptr_t *),
	     free_int_ptr_t_fn);
  //dynamically allocate and push elements
  int num_push = 10;
  printf("Pushing %d elements... \n\n", num_push);
  push_int_ptr_t_elts(&q, num_push);
  //pop all elements
  printf("Popping all elements... \n\n");
  pop_all_int_ptr_t_elts(&q);
  //dynamically allocate, push elements, and free queue
  printf("Pushing %d elements again... \n\n", num_push);
  push_int_ptr_t_elts(&q, num_push);
  printf("Freeing queue... \n\n");
  queue_free(&q);
}

int main(){
  run_int_queue_test();
  run_int_ptr_t_queue_test();
  return 0;
}
