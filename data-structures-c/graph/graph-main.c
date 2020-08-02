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

/** 
   Test adj_lst_{init, dir_build, undir_build, free} on a graph with edges 
   and integer weights.
*/

/**
   Initializes a graph with integer weights.
*/
void int_wts_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  int wts[] = {4, 3, 2, 1};
  int num_vts = 5;
  graph_base_init(g, num_vts);
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
   Runs a test of adj_lst_{init, dir_build, undir_build, free} on a graph
   with edges and integer weights. The test relies on the construction 
   order in adj_lst_{dir_build, undir_build}.
*/
static void int_wts_graph_test_helper(adj_lst_t *a,
				      int split[],
				      int vts[],
				      int wts[]);

void run_int_wts_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split_dir[] = {3, 1, 0, 0, 0};
  int vts_dir[] = {1, 2, 3, 3};
  int wts_dir[] = {4, 3, 2, 1};
  int split_undir[] = {3, 2, 1, 2, 0};
  int vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  int wts_undir[] = {4, 3, 2, 4, 1, 3, 2, 1};
  int_wts_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with edges and integer weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  int_wts_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_int_adj_lst(&a);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected graph "
	 "with edges and integer weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  int_wts_graph_test_helper(&a, split_undir, vts_undir, wts_undir);
  print_int_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

static void int_wts_graph_test_helper(adj_lst_t *a,
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
   Test adj_lst_{init, dir_build, undir_build, free} on a graph with edges
   and double weights.
*/

/**
   Initializes a graph with double weights.
*/
void double_wts_graph_init(graph_t *g){
  int u[] = {0, 0, 0, 1};
  int v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  int num_vts = 5;
  graph_base_init(g, num_vts);
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
   Runs a test of adj_lst_{init, dir_build, undir_build, free} on a graph 
   with edges and double weights. The test relies on the construction order 
   in adj_lst_{dir_build, undir_build}.
*/
static void double_wts_graph_test_helper(adj_lst_t *a,
				         int split[],
				         int vts[],
				         double wts[]);

void run_double_wts_graph_test(){
  graph_t g;
  adj_lst_t a;
  int split_dir[] = {3, 1, 0, 0, 0};
  int vts_dir[] = {1, 2, 3, 3};
  double wts_dir[] = {4.0, 3.0, 2.0, 1.0};
  int split_undir[] = {3, 2, 1, 2, 0};
  int vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  double wts_undir[] = {4.0, 3.0, 2.0, 4.0, 1.0, 3.0, 2.0, 1.0};
  double_wts_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with edges and double weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_dir_build(&g, &a);
  double_wts_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_double_adj_lst(&a);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected graph "
	 "with edges and double weights --> ");
  adj_lst_init(&g, &a);
  adj_lst_undir_build(&g, &a);
  double_wts_graph_test_helper(&a, split_undir, vts_undir, wts_undir); 
  print_double_adj_lst(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

static void double_wts_graph_test_helper(adj_lst_t *a,
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
   Test adj_lst_{init, dir_build, undir_build, free} on graphs with 
   no edges and 0 or more vertices.
*/

/**
   Runs a adj_lst_{init, dir_build, undir_build, free} test on graphs with 
   no edges and 0 or more vertices.
*/
static void corner_cases_graph_test_helper(adj_lst_t *a,
					   int vt_ix,
					   int *result);
  
void run_corner_cases_graph_test(){
  graph_t g;
  adj_lst_t a;
  int max_num_vts = 100;
  int result = 1;
  for (int i = 0; i < max_num_vts; i++){
    graph_base_init(&g, i);
    adj_lst_init(&g, &a);
    adj_lst_dir_build(&g, &a);
    result *= (a.num_vts == i &&
	       a.num_es == 0 &&
	       a.wt_size == 0 &&
	       a.wts == NULL);
    corner_cases_graph_test_helper(&a, i, &result);
    adj_lst_free(&a);
    adj_lst_init(&g, &a);
    adj_lst_undir_build(&g, &a);
    result *= (a.num_vts == i &&
	       a.num_es == 0 &&
	       a.wt_size == 0 &&
	       a.wts == NULL);
    corner_cases_graph_test_helper(&a, i, &result);
    adj_lst_free(&a);
    graph_free(&g);
  }
  printf("Test adj_lst_{init, dir_build, undir_build, free} "
	 "on graphs with no edges and 0 or more vertices --> ");
  print_test_result(result);
}

static void corner_cases_graph_test_helper(adj_lst_t *a,
					   int vt_ix,
					   int *result){
  if(vt_ix){
    *result *= (a->vts != NULL);
    for(int i = 0; i < vt_ix; i++){
      *result *= (a->vts[i]->num_elts == 0);
    }
  }else{
    *result *= (a->vts == NULL);
  }
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
  graph_base_init(g, n);
  g->num_es = num_es;
  g->u = malloc(g->num_es * sizeof(int));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(int));
  assert(g->v != NULL);
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
   Test adj_lst_add_dir_edge and adj_lst_add_undir_edge.
*/

int int_sum(int *a, int num_elts);

/**
   Runs a adj_lst_add_dir_edge test on DAGs.
*/
void run_adj_lst_add_dir_edge_test(){
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  int n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  uint32_t nom = 1;
  uint32_t denom = 1;
  int result = 1;
  clock_t t;
  printf("Test adj_lst_add_dir_edge on DAGs \n");
  printf("\tn vertices, 0 as source, n(n - 1)/2 directed edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g_blt, n);
    graph_base_init(&g_bld, n);
    adj_lst_init(&g_blt, &a_blt);
    adj_lst_init(&g_bld, &a_bld);
    adj_lst_dir_build(&g_blt, &a_blt);
    adj_lst_dir_build(&g_bld, &a_bld);
    t = clock();
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_dir_edge(&a_bld, i, j, nom, denom);
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
   Runs a adj_lst_add_undir_edge test on complete graphs.
*/
void run_adj_lst_add_undir_edge_test(){
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  int n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  uint32_t nom = 1;
  uint32_t denom = 1;
  int result = 1;
  clock_t t;
  printf("Test adj_lst_add_undir_edge on complete graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) directed "
	 "edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    complete_graph_init(&g_blt, n);
    graph_base_init(&g_bld, n);
    adj_lst_init(&g_blt, &a_blt);
    adj_lst_init(&g_bld, &a_bld);
    adj_lst_undir_build(&g_blt, &a_blt);
    adj_lst_undir_build(&g_bld, &a_bld);
    t = clock();
    for (int i = 0; i < n - 1; i++){
      for (int j = i + 1; j < n; j++){
	adj_lst_add_undir_edge(&a_bld, i, j, nom, denom);
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
   Test adj_lst_rand_dir and adj_lst_rand_undir.
*/

/**
   Runs a adj_lst_rand_dir test on the number of edges in expectation.
*/
void run_adj_lst_rand_dir_test(){
  adj_lst_t a;
  int n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  uint32_t nom = 1;
  uint32_t denom = 2;
  printf("Test adj_lst_rand_dir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1) * (0.5 * 1)\n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    adj_lst_rand_dir(&a, n, nom, denom);
    printf("\t\tvertices: %d, expected directed edges: %.1f, "
	   "directed edges: %d\n",
	   n, 0.5 * (float)(n * (n - 1)), a.num_es);
    fflush(stdout);
    adj_lst_free(&a);
  }
}

/**
   Runs a adj_lst_rand_undir test on the number of edges in expectation.
*/
void run_adj_lst_rand_undir_test(){
  adj_lst_t a;
  int n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  uint32_t nom = 1;
  uint32_t denom = 2;
  printf("Test adj_lst_rand_undir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1)/2 * (0.5 * 2)\n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = (int)pow_two_uint64(i);
    adj_lst_rand_undir(&a, n, nom, denom);
    printf("\t\tvertices: %d, expected directed edges: %.1f, "
	   "directed edges: %d\n",
	   n, 0.5 * (float)(n * (n - 1)), a.num_es);
    fflush(stdout);
    adj_lst_free(&a);
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
  run_int_wts_graph_test();
  run_double_wts_graph_test();
  run_corner_cases_graph_test();
  run_adj_lst_undir_build_test();
  run_adj_lst_add_dir_edge_test();
  run_adj_lst_add_undir_edge_test();
  run_adj_lst_rand_dir_test();
  run_adj_lst_rand_undir_test();
  return 0;
}
