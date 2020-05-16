/**
    Lecture 8 code snippets.
  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(){
  /**
     first allocation: 24 bytes + 8 byte long part + 1 byte
     thereafter allocates in 16 byte increments
     this malloc heuristic may differ across implementations
  */
  printf("malloc header's allocation record size: %ld bytes \n", sizeof(long));
  for (int i = 0; i < 32; i++){
    long to_alloc = i * sizeof(int);
    void *a = malloc(to_alloc);
    long alloc = *((long *)a - 1);
    free(a);
    a = NULL;
    printf("allocate: %ld bytes, allocated: %ld bytes\n", to_alloc, alloc);
  }
  printf("\n");
  
  /**
     start with a much larger starting block; the same/similar heuristic 
     is used except that the first allocation is larger than the second 
     in this malloc implementation
  */
  for (int i = (int)pow(2, 16); i < 32 + (int)pow(2, 16); i++){
    long to_alloc = i * sizeof(int);
    void *a = malloc(to_alloc);
    long alloc = *((long *)a - 1);
    free(a);
    a = NULL;
    printf("allocate: %ld bytes, allocated: %ld bytes\n", to_alloc, alloc);
  }
}
