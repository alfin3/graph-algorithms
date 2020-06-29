/**
   graph-main.c

   Examples of graphs with generic weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "stack.h"
#include "graph.h"
#include "utilities-ds.h"

void print_test_result(int result);
int int_sum(int *a, int num_elts);

/** 
   Test adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with integer weights.
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
   Runs a test of adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with integer weights. The test relies on the 
   construction order in adj_lst_dir_build and adj_lst_undir_build.
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
   Test adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with double weights.
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
   Prints the adjacency list of a graph with double weights.
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
   Runs a test of adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with double weights. The test relies on the 
   construction order in adj_lst_dir_build and adj_lst_undir_build.
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
   Test adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with no edges.
*/

/**
   Initializes a graph with no edges.
*/
void no_edges_graph_init(graph_t *g, int n){
  g->num_vts = n;
  g->num_es = 0;
  g->wt_size = 0;
  g->wts = NULL;
  g->u = NULL;
  g->v = NULL;
}

/**
   Prints the adjacency list of a graph with no weights, used to print the
   adjacency list of a graph with no edges.
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
   Runs a test of adj_lst_init, adj_lst_dir_build, adj_lst_undir_build, and 
   adj_lst_free on a graph with no edges.
*/
static void no_edges_graph_test_helper(adj_lst_t *a, int split[]);

void run_no_edges_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split[] = {0, 0, 0, 0, 0};
  int num_vts = 5;
  no_edges_graph_init(&g, num_vts);
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
   Test adj_lst_undir_build.
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
   Runs a adj_lst_undir_build test on complete graphs.
*/
void run_adj_lst_undir_build_test(){
  graph_t g;
  adj_lst_t a;
  int n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  clock_t t;
  printf("Test adj_lst_undir_build on complete graphs without weights \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) directed "
	 "edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g, n);
    adj_lst_init(&g, &a);
    t = clock();
    adj_lst_undir_build(&g, &a);
    t = clock() - t;
    printf("\t\tvertices: %d, directed edges: %d, build time: %.6f seconds\n",
	   n, n * (n - 1), (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

/** 
   Test adj_lst_add_dir_edge.
*/

/**
   Runs a adj_lst_add_dir_edge test on DAGs.
*/
void run_adj_lst_add_dir_edge_test(){
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  int n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  int result = 1;
  clock_t t;
  printf("Test adj_lst_add_dir_edge on DAGs \n");
  printf("\tn vertices, 0 as source, n(n - 1)/2 directed edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g_blt, n);
    no_edges_graph_init(&g_bld, n);
    adj_lst_init(&g_blt, &a_blt);
    adj_lst_init(&g_bld, &a_bld);
    adj_lst_dir_build(&g_blt, &a_blt);
    adj_lst_dir_build(&g_bld, &a_bld);
    t = clock();
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_dir_edge(&a_bld, i, j, 1, 1);
      }
    }
    t = clock() - t;
    printf("\t\tvertices: %d, directed edges: %d, build time: %.6f seconds\n",
	   n, (n * (n - 1)) / 2, (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    //int_sum for < 2^16 vertices
    for (int i = 0; i < n; i++){
      result *= (a_blt.vts[i]->num_elts == a_bld.vts[i]->num_elts);
      result *= (int_sum((int *)a_blt.vts[i]->elts, a_blt.vts[i]->num_elts) ==
		 int_sum((int *)a_bld.vts[i]->elts, a_bld.vts[i]->num_elts));
    }
    result *= (a_blt.num_es == a_bld.num_es);
    adj_lst_free(&a_blt);
    adj_lst_free(&a_bld);
    graph_free(&g_blt);
    graph_free(&g_bld);
  }
  printf("\t\tcorrectness across all builds --> ");
  print_test_result(result);
}

/**
   Runs a adj_lst_add_dir_edge test on the number of edges in expectation.
*/
void run_adj_lst_add_dir_edge_exp_test(){
  graph_t g;
  adj_lst_t a;
  int n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  printf("Test adj_lst_add_dir_edge on the number of edges in expectation\n");
  printf("\tn vertices, E[number of directed edges] = "
	 "n(n - 1)/2 * (0.5 * 1)\n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    no_edges_graph_init(&g, n);
    adj_lst_init(&g, &a);
    adj_lst_dir_build(&g, &a);
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_dir_edge(&a, i, j, 1, 2);
      }
    }
    printf("\t\tvertices: %d, expected directed edges: %.1f, "
	   "directed edges: %d\n",
	   n, 0.5 * (float)(n * (n - 1)) / 2.0, a.num_es);
    fflush(stdout);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

/** 
   Test adj_lst_add_undir_edge.
*/

/**
   Runs a adj_lst_add_undir_edge test on complete graphs.
*/
void run_adj_lst_add_undir_edge_test(){
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  int n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  int result = 1;
  clock_t t;
  printf("Test adj_lst_add_undir_edge on complete graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) directed "
	 "edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g_blt, n);
    no_edges_graph_init(&g_bld, n);
    adj_lst_init(&g_blt, &a_blt);
    adj_lst_init(&g_bld, &a_bld);
    adj_lst_undir_build(&g_blt, &a_blt);
    adj_lst_undir_build(&g_bld, &a_bld);
    t = clock();
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_undir_edge(&a_bld, i, j, 1, 1);
      }
    }
    t = clock() - t;
    printf("\t\tvertices: %d, directed edges: %d, build time: %.6f seconds\n",
	   n, n * (n - 1), (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    //int_sum for < 2^16 vertices
    for (int i = 0; i < n; i++){
      result *= (a_blt.vts[i]->num_elts == a_bld.vts[i]->num_elts);
      result *= (int_sum((int *)a_blt.vts[i]->elts, a_blt.vts[i]->num_elts) ==
		 int_sum((int *)a_bld.vts[i]->elts, a_bld.vts[i]->num_elts));
    }
    result *= (a_blt.num_es == a_bld.num_es);
    adj_lst_free(&a_blt);
    adj_lst_free(&a_bld);
    graph_free(&g_blt);
    graph_free(&g_bld);
  }
  printf("\t\tcorrectness across all builds --> ");
  print_test_result(result);
}

/**
   Runs a adj_lst_add_undir_edge test on the number of edges in expectation.
*/
void run_adj_lst_add_undir_edge_exp_test(){
  graph_t g;
  adj_lst_t a;
  int n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  printf("Test adj_lst_add_undir_edge on the number of edges in "
	 "expectation\n");
  printf("\tn vertices, E[number of directed edges] = "
	 "n(n - 1)/2 * (0.5 * 2)\n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    no_edges_graph_init(&g, n);
    adj_lst_init(&g, &a);
    adj_lst_undir_build(&g, &a);
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_undir_edge(&a, i, j, 1, 2);
      }
    }
    printf("\t\tvertices: %d, expected directed edges: %.1f, "
	   "directed edges: %d\n",
	   n, 0.5 * (float)(n * (n - 1)), a.num_es);
    fflush(stdout);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

int int_sum(int *a, int num_elts){
  int ret = 0;
  for (int i = 0; i < num_elts; i++){
    ret += a[i];
  }
  return ret;
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
  run_adj_lst_undir_build_test();
  run_adj_lst_add_dir_edge_test();
  run_adj_lst_add_undir_edge_test();
  run_adj_lst_add_dir_edge_exp_test();
  run_adj_lst_add_undir_edge_exp_test();
  return 0;
}
