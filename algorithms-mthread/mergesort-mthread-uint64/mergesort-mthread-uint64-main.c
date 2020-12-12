/**
   mergesort-mthread-uint64-main.c

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "mergesort-mthread-uint64.h"
#include "utilities-mem.h"

/**
   Printing helper function.
*/
static void print_int_elts(void *a, uint64_t count){
  for (uint64_t i = 0; i < count; i++){
    printf("%u ", *((int *)a + i));
  }
  printf("\n");
}


int main(){
  uint64_t count = 50;
  int *a =  malloc_perror(count * sizeof(int));
  for (uint64_t i = 0; i < count; i++){
    a[i] = random() % 100;
  }
  print_int_elts(a, count);
  mergesort_mthread_uint64(a, count, 2);
  print_int_elts(a, count);
  free(a);
  a = NULL;
  return 0;
}
