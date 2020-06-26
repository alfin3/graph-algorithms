/**
   graph-main.c

   Examples of graphs with generic weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "stack.h"
#include "graph.h"
#include "utilities-ds.h"

void print_test_result(int result);

/** 
   Test a graph with integer weights.
*/

/**
   Initializes a graph with integer weights.
*/
void int_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  int wts[] = {4, 3, 2, 1};
  g->num_vts = 5;
  g->num_es = 4;
  g->wt_size = sizeof(int);
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (int i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((int *)(g->wts) + i) = wts[i];
  }
}

/**
   Frees the dynamically allocated arrays of a graph.
*/
void graph_free(graph_t *g){
  free(g->u);
  free(g->v);
  free(g->wts);
  g->u = NULL;
  g->v = NULL;
  g->wts = NULL;
}

/**
   Prints the adjacency list of a graph with integer weights.
*/
void print_all_int_elts(stack_t *s);
  
void print_int_adj_lst(adj_lst_t *a){
  printf("\tvertices: \n");
  for (int i = 0; i < a->num_vts; i++){
    printf("\t%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\tweights: \n");
  for (int i = 0; i < a->num_vts; i++){
    printf("\t%d : ", i);
    print_all_int_elts(a->wts[i]);
  }
  printf("\n");
}

void print_all_int_elts(stack_t *s){
  for (int i = 0; i < s->num_elts; i++){
    printf("%d ", *((int *)s->elts + i));
  }
  printf("\n");
}

/**
   Runs a test of building the adjacency list of a graph with integer 
   weights. The test relies on the construction order in adj_lst_dir_build 
   and adj_lst_undir_build.
*/
static void int_graph_test_helper(adj_lst_t *a,
				  int split[],
				  int vts[],
				  int wts[]);

void run_int_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split_dir[] = {3, 1, 0, 0, 0};
  int vts_dir[] = {1, 2, 3, 3};
  int wts_dir[] = {4, 3, 2, 1};
  int split_undir[] = {3, 2, 1, 2, 0};
  int vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  int wts_undir[] = {4, 3, 2, 4, 1, 3, 2, 1};
  int_graph_init(&g);
  printf("Test the adjacency list of a directed graph with int weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  int_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_int_adj_lst(&a);
  adj_lst_free(&a);
  printf("Test the adjacency list of an undirected graph with "
	 "int weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  int_graph_test_helper(&a, split_undir, vts_undir, wts_undir); 
  print_int_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

static void int_graph_test_helper(adj_lst_t *a,
				  int split[],
				  int vts[],
				  int wts[]){
  int result = 1;
  int ix = 0;
  for (int i = 0; i < a->num_vts; i++){
    result *= (split[i] == a->vts[i]->num_elts);
    for (int j = 0; j < split[i]; j++){
      result *= (*((int *)a->vts[i]->elts + j) == vts[ix]);
      result *= (*((int *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(result);
}

/** 
   Test a graph with double weights.
*/

/**
   Initializes a graph with double weights.
*/
void double_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  g->num_vts = 5;
  g->num_es = 4;
  g->wt_size = sizeof(double);
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (int i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)(g->wts) + i) = wts[i];
  }
}

/**
   Prints adjacency list of a graph with double weights.
*/
void print_all_double_elts(stack_t *s);
  
void print_double_adj_lst(adj_lst_t *a){
  printf("\tvertices: \n");
  for (int i = 0; i < a->num_vts; i++){
    printf("\t%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\tweights: \n");
  for (int i = 0; i < a->num_vts; i++){
    printf("\t%d : ", i);
    print_all_double_elts(a->wts[i]);
  }
  printf("\n");
}

void print_all_double_elts(stack_t *s){
  for (int i = 0; i < s->num_elts; i++){
    printf("%0.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}

/**
   Runs a test of building the adjacency list of a graph with double weights.
   The test relies on the construction order in adj_lst_dir_build and 
   adj_lst_undir_build.
*/
static void double_graph_test_helper(adj_lst_t *a,
				     int split[],
				     int vts[],
				     double wts[]);

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split_dir[] = {3, 1, 0, 0, 0};
  int vts_dir[] = {1, 2, 3, 3};
  double wts_dir[] = {4.0, 3.0, 2.0, 1.0};
  int split_undir[] = {3, 2, 1, 2, 0};
  int vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  double wts_undir[] = {4.0, 3.0, 2.0, 4.0, 1.0, 3.0, 2.0, 1.0};
  double_graph_init(&g);
  printf("Test the adjacency list of a directed graph with double "
	 "weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  double_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_double_adj_lst(&a);
  adj_lst_free(&a);
  printf("Test the adjacency list of an undirected graph with double "
	 "weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  double_graph_test_helper(&a, split_undir, vts_undir, wts_undir); 
  print_double_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

static void double_graph_test_helper(adj_lst_t *a,
				     int split[],
				     int vts[],
				     double wts[]){
  int result = 1;
  int ix = 0;
  for (int i = 0; i < a->num_vts; i++){
    result *= (split[i] == a->vts[i]->num_elts);
    for (int j = 0; j < split[i]; j++){
      result *= (*((int *)a->vts[i]->elts + j) == vts[ix]);
      //comparison of doubles OK, same bit pattern
      result *= (*((double *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(result);
}

/** 
    Test a graph with no edges.
*/

/**
   Initializes a graph with no edges.
*/
void no_edges_graph_init(graph_t *g){
  g->num_vts = 5;
  g->num_es = 0;
  g->wt_size = 0;
  g->wts = NULL;
  g->u = NULL;
  g->v = NULL;
}

/**
   Prints the adjacency list of a graph with no weights.
*/
void print_no_weights_adj_lst(adj_lst_t *a){
  printf("\tvertices: \n");
  for (int i = 0; i < a->num_vts; i++){
    printf("\t%d : ", i);
    print_all_int_elts(a->vts[i]);
  }
  printf("\n");
}

/**
   Runs a test of building the adjacency list of a graph with no edges.
*/
static void no_edges_graph_test_helper(adj_lst_t *a, int split[]);

void run_no_edges_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split[] = {0, 0, 0, 0, 0};
  no_edges_graph_init(&g);
  printf("Test the adjacency list of a directed graph with no edges --> ");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  no_edges_graph_test_helper(&a, split);
  print_no_weights_adj_lst(&a);
  adj_lst_free(&a);
  printf("Test the adjacency list of an undirected graph with no edges --> ");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  no_edges_graph_test_helper(&a, split); 
  print_no_weights_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

static void no_edges_graph_test_helper(adj_lst_t *a, int split[]){
  int result = 1;
  for (int i = 0; i < a->num_vts; i++){
    result *= (split[i] == a->vts[i]->num_elts);
  }
  print_test_result(result);
}

/** 
   Test undirected graphs with n vertices and n(n - 1)/2 edges, represented 
   by n(n - 1) directed edges.
*/

/**
   Initializes a graph that is complete in the undirected form.
*/
void complete_graph_init(graph_t *g, int n){
  assert(n > 1);
  int num_es = (n * (n - 1)) / 2; //n * (n - 1) is even
  int ix = 0;
  g->num_vts = n;
  g->num_es = num_es;
  g->wt_size = 0;
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
  g->wts = NULL;
  for (int i = 0; i < n - 1; i++){
    for (int j = i + 1; j < n; j++){
      g->u[ix] = i;
      g->v[ix] = j;
      ix++;
    }
  }
}

/**
   Runs a test of building the adjacency list of a complete graph.
*/
void run_complete_graph_test(){
  graph_t g;
  adj_lst_t a;
  int n;
  clock_t t;
  printf("Test adj_lst_init and adj_lst_undir_build on undirected graphs "
	 "without weights, \neach with n vertices and n(n - 1)/2 edges, "
	 "represented by n(n - 1) directed edges \n");
  for (int i = 4; i < 16; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g, n);
    fflush(stdout);
    t = clock();
    adj_lst_init(&g, &a);
    adj_lst_undir_build(&g, &a);
    t = clock() - t;
    printf("\tvertices: %d, directed edges: %d, build time: %.4f seconds\n",
	   n, n * (n - 1), (float)t / CLOCKS_PER_SEC);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

void print_test_result(int result){
  if (result){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_int_graph_test();
  run_double_graph_test();
  run_no_edges_graph_test();
  run_complete_graph_test();
  return 0;
}
