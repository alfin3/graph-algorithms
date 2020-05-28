/**
   graph-main.c

   Examples of graph with generic weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graph.h"

/** 
    Graph of with integer weights.
*/

/**
   Prints all integer elements in element array of stack.
*/
void print_all_int_elts(stack_t *s){
  for (int i = 0; i < s->num_elts; i++){
    printf("%d ", *((int *)s->elts + i));
  }
  printf("\n");
}

/**
   Initializes an instance of a graph with integer weights.
*/
void int_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  int wts[] = {4, 3, 2, 1};
  g->num_vts = 5;
  g->num_e = 4;
  g->wt_size = sizeof(int);
  g->u = malloc(g->num_e * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_e * sizeof(int));
  assert(g->v != NULL);
  g->wts = malloc(g->num_e * g->wt_size);
  assert(g->wts != NULL);
  for (int i = 0; i < g->num_e; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((int *)(g->wts) + i) = wts[i];
  }
}

/**
   Frees the dynamically allocated arrays of a graph.
*/
void int_graph_free(graph_t *g){
  free(g->u);
  free(g->v);
  free(g->wts);
}

/**
   Runs a test example of a graph with integer weights.
*/
void run_int_graph_test(){
  graph_t g;
  adj_lst_t a;
  int_graph_init(&g);
  
  printf("Running int directed graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  printf("Print vertices... \n\n");
  for (int i = 0; i < a.num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a.vts[i]);
  }
  printf("\n");
  printf("Print weights... \n\n");
  for (int i = 0; i < a.num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a.wts[i]);
  }
  printf("\n");
  
  adj_lst_free(&a);
  printf("Running int undirected graph test... \n\n");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  for (int i = 0; i < a.num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a.vts[i]);
  }
  printf("\n");
  printf("Print weights... \n\n");
  for (int i = 0; i < a.num_vts; i++){
    printf("%d : ", i);
    print_all_int_elts(a.wts[i]);
  }
  printf("\n");
  adj_lst_free(&a);
  int_graph_free(&g);
}

int main(){
  run_int_graph_test();
  return 0;
}
