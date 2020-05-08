/**
   int-double-ll-main.c

   Use examples of a dynamicaly allocated non-generic doubly linked list of 
   integer entries.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include "int-double-ll.h"

int main(){
  node_t *head = NULL;
  int num_nodes = 10;
  for (int i = 0; i < num_nodes; i++){
    insert(&head, i + 1);
  }
  printf("Before freeing.\n");
  print_ll(head);
  free_ll(head);
  printf("After freeing.\n");
  printf("%d \n", head->data);
  head = NULL;
  return 0;
}
