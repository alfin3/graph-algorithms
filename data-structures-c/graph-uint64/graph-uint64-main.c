/**
   graph-uint64-main.c

   Examples of graphs with generic weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "graph-uint64.h"
#include "stack-uint64.h"
#include "utilities-ds.h"

void print_test_result(int result);
uint64_t uint64_sum(uint64_t *a, uint64_t num_elts);

/** 
   Test adj_lst_uint64_{init, dir_build, undir_build, free} on a small graph
   with edges and uint64_t and double weights.
*/

/**
   Initializes a small graph with uint64_t weights.
*/
void uint64_wts_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  uint64_t num_vts = 5;
  graph_uint64_base_init(g, num_vts, sizeof(uint64_t));
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

/**
   Initializes a small graph with double weights.
*/
void double_wts_graph_init(graph_uint64_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  int num_vts = 5;
  graph_uint64_base_init(g, num_vts, sizeof(double));
  g->num_es = 4;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  g->wts = malloc(g->num_es * g->wt_size);
  assert(g->wts != NULL);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

/**
   Printing helper functions.
*/
void print_uint64_elts(stack_uint64_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(stack_uint64_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%0.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(adj_lst_uint64_t *a,
		   void (*print_wts_fn)(stack_uint64_t *)){
  printf("\tvertices: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint64_elts(a->vts[i]);
  }
  if (print_wts_fn != NULL){
    printf("\tweights: \n");
    for (uint64_t i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts_fn(a->wts[i]);
    }
  }
  printf("\n");
}

/**
   Runs a test of adj_lst_uint64_{init, dir_build, undir_build, free} on a 
   small graph with edges and uint64_t weights. The test relies on the 
   construction order in adj_lst_uint64_{dir_build, undir_build}.
*/
static void uint64_wts_graph_test_helper(adj_lst_uint64_t *a,
					 uint64_t split[],
					 uint64_t vts[],
					 uint64_t wts[]);

void run_uint64_wts_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  uint64_t split_dir[] = {3, 1, 0, 0, 0};
  uint64_t vts_dir[] = {1, 2, 3, 3};
  uint64_t wts_dir[] = {4, 3, 2, 1};
  uint64_t split_undir[] = {3, 2, 1, 2, 0};
  uint64_t vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  uint64_t wts_undir[] = {4, 3, 2, 4, 1, 3, 2, 1};
  uint64_wts_graph_init(&g);
  printf("Test adj_lst_uint64_{init, dir_build, free} on a directed "
	 "graph with uint64_t weights --> ");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  uint64_wts_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_adj_lst(&a, print_uint64_elts);
  adj_lst_uint64_free(&a);
  printf("Test adj_lst_uint64_{init, undir_build, free} on an undirected "
	 "graph with uint64_t weights --> ");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  uint64_wts_graph_test_helper(&a, split_undir, vts_undir, wts_undir);
  print_adj_lst(&a, print_uint64_elts);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

static void uint64_wts_graph_test_helper(adj_lst_uint64_t *a,
					 uint64_t split[],
					 uint64_t vts[],
					 uint64_t wts[]){
  int result = 1;
  uint64_t ix = 0;
  for (uint64_t i = 0; i < a->num_vts; i++){
    result *= (split[i] == a->vts[i]->num_elts);
    for (uint64_t j = 0; j < split[i]; j++){
      result *= (*((uint64_t *)a->vts[i]->elts + j) == vts[ix]);
      result *= (*((uint64_t *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(result);
}

/**
   Runs a test of adj_lst_uint64_{init, dir_build, undir_build, free} on a 
   small graph with edges and double weights. The test relies on the 
   construction order in adj_lst_uint64_{dir_build, undir_build}.
*/
static void double_wts_graph_test_helper(adj_lst_uint64_t *a,
				         uint64_t split[],
				         uint64_t vts[],
				         double wts[]);

void run_double_wts_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  uint64_t split_dir[] = {3, 1, 0, 0, 0};
  uint64_t vts_dir[] = {1, 2, 3, 3};
  double wts_dir[] = {4.0, 3.0, 2.0, 1.0};
  uint64_t split_undir[] = {3, 2, 1, 2, 0};
  uint64_t vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  double wts_undir[] = {4.0, 3.0, 2.0, 4.0, 1.0, 3.0, 2.0, 1.0};
  double_wts_graph_init(&g);
  printf("Test adj_lst_uint64_{init, dir_build, free} on a directed "
	 "graph with double weights --> ");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_dir_build(&a, &g);
  double_wts_graph_test_helper(&a, split_dir, vts_dir, wts_dir);
  print_adj_lst(&a, print_double_elts);
  adj_lst_uint64_free(&a);
  printf("Test adj_lst_uint64_{init, undir_build, free} on an undirected "
	 "graph with double weights --> ");
  adj_lst_uint64_init(&a, &g);
  adj_lst_uint64_undir_build(&a, &g);
  double_wts_graph_test_helper(&a, split_undir, vts_undir, wts_undir); 
  print_adj_lst(&a, print_double_elts);
  adj_lst_uint64_free(&a);
  graph_uint64_free(&g);
}

static void double_wts_graph_test_helper(adj_lst_uint64_t *a,
				         uint64_t split[],
				         uint64_t vts[],
				         double wts[]){
  int result = 1;
  uint64_t ix = 0;
  for (uint64_t i = 0; i < a->num_vts; i++){
    result *= (split[i] == a->vts[i]->num_elts);
    for (uint64_t j = 0; j < split[i]; j++){
      result *= (*((uint64_t *)a->vts[i]->elts + j) == vts[ix]);
      //comparison of doubles OK here, same bit pattern
      result *= (*((double *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(result);
}

/** 
   Test adj_lst_uint64_{init, dir_build, undir_build, free} on corner cases,
   i.e. graphs with no edge weights and edges, and 0 or more vertices.
*/

static void corner_cases_graph_test_helper(adj_lst_uint64_t *a,
					   uint64_t num_vts,
					   int *result);
  
void run_corner_cases_graph_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  uint64_t max_num_vts = 100;
  int result = 1;
  for (uint64_t i = 0; i < max_num_vts; i++){
    graph_uint64_base_init(&g, i, 0);
    adj_lst_uint64_init(&a, &g);
    adj_lst_uint64_dir_build(&a, &g);
    result *= (a.num_vts == i &&
	       a.num_es == 0 &&
	       a.wt_size == 0 &&
	       a.wts == NULL);
    corner_cases_graph_test_helper(&a, i, &result);
    adj_lst_uint64_free(&a);
    adj_lst_uint64_init(&a, &g);
    adj_lst_uint64_undir_build(&a, &g);
    result *= (a.num_vts == i &&
	       a.num_es == 0 &&
	       a.wt_size == 0 &&
	       a.wts == NULL);
    corner_cases_graph_test_helper(&a, i, &result);
    adj_lst_uint64_free(&a);
    graph_uint64_free(&g);
  }
  printf("Test adj_lst_uint64_{init, dir_build, undir_build, free} on "
	 "corner cases --> ");
  print_test_result(result);
}

static void corner_cases_graph_test_helper(adj_lst_uint64_t *a,
					   uint64_t num_vts,
					   int *result){
  if(num_vts){
    *result *= (a->vts != NULL);
    for(uint64_t i = 0; i < num_vts; i++){
      *result *= (a->vts[i]->num_elts == 0);
    }
  }else{
    *result *= (a->vts == NULL);
  }
}

/**
   Test adj_lst_uint64_undir_build.
*/

/**
   Initializes an unweighted graph that is i) a DAG with source 0 and 
   n(n - 1) / 2 edges in the directed form, and ii) complete in the 
   undirected form.
*/
void complete_graph_init(graph_uint64_t *g, uint64_t n){
  assert(n > 1);
  uint64_t num_es = (n * (n - 1)) / 2; //n * (n - 1) is even
  uint64_t ix = 0;
  graph_uint64_base_init(g, n, 0);
  g->num_es = num_es;
  g->u = malloc(g->num_es * sizeof(uint64_t));
  assert(g->u != NULL);
  g->v = malloc(g->num_es * sizeof(uint64_t));
  assert(g->v != NULL);
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      g->u[ix] = i;
      g->v[ix] = j;
      ix++;
    }
  }
}

/**
   Runs a adj_lst_uint64_undir_build test on complete unweighted graphs.
*/
void run_adj_lst_uint64_undir_build_test(){
  graph_uint64_t g;
  adj_lst_uint64_t a;
  uint64_t n;
  int pow_two_start = 4;
  int pow_two_end = 15;
  clock_t t;
  printf("Test adj_lst_uint64_undir_build on complete unweighted graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) directed "
	 "edges \n");
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = pow_two_uint64(i);
    complete_graph_init(&g, n);
    adj_lst_uint64_init(&a, &g);
    t = clock();
    adj_lst_uint64_undir_build(&a, &g);
    t = clock() - t;
    printf("\t\tvertices: %lu, directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   a.num_vts, a.num_es, (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    adj_lst_uint64_free(&a);
    graph_uint64_free(&g);
  }
}

/**
   Test adj_lst_uint64_add_dir_edge and adj_lst_uint64_add_undir_edge.
*/

void add_edge_test_helper(void (*build_fn)(adj_lst_uint64_t *,
					   graph_uint64_t *),
			  void (*add_edge_fn)(adj_lst_uint64_t *,
					      uint64_t,
					      uint64_t,
					      uint32_t,
					      uint32_t));

void run_adj_lst_uint64_add_dir_edge_test(){
  printf("Test adj_lst_uint64_add_dir_edge on DAGs \n");
  printf("\tn vertices, 0 as source, n(n - 1)/2 directed edges \n");
  add_edge_test_helper(adj_lst_uint64_dir_build,
		       adj_lst_uint64_add_dir_edge);
}

void run_adj_lst_uint64_add_undir_edge_test(){
  printf("Test adj_lst_uint64_add_undir_edge on complete graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) directed "
	 "edges \n");
  add_edge_test_helper(adj_lst_uint64_undir_build,
		       adj_lst_uint64_add_undir_edge);
}

void add_edge_test_helper(void (*build_fn)(adj_lst_uint64_t *,
					   graph_uint64_t *),
			  void (*add_edge_fn)(adj_lst_uint64_t *,
					      uint64_t,
					      uint64_t,
					      uint32_t,
					      uint32_t)){
  graph_uint64_t g_blt, g_bld;
  adj_lst_uint64_t a_blt, a_bld;
  uint64_t n;
  int pow_two_start = 4; //> 0
  int pow_two_end = 15;
  uint32_t num = 1;
  uint32_t denom = 1;
  int result = 1;
  clock_t t;
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = pow_two_uint64(i);
    complete_graph_init(&g_blt, n);
    graph_uint64_base_init(&g_bld, n, 0);
    adj_lst_uint64_init(&a_blt, &g_blt);
    adj_lst_uint64_init(&a_bld, &g_bld);
    build_fn(&a_blt, &g_blt);
    build_fn(&a_bld, &g_bld);
    t = clock();
    for (uint64_t i = 0; i < n - 1; i++){
      for (uint64_t j = i + 1; j < n; j++){
	add_edge_fn(&a_bld, i, j, num, denom);
      }
    }
    t = clock() - t;
    printf("\t\tvertices: %lu, directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   a_bld.num_vts, a_bld.num_es, (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    //uint64_sum test
    for (uint64_t i = 0; i < n; i++){
      result *= (a_blt.vts[i]->num_elts == a_bld.vts[i]->num_elts);
      result *= (uint64_sum((uint64_t *)a_blt.vts[i]->elts,
			    a_blt.vts[i]->num_elts) ==
		 uint64_sum((uint64_t *)a_bld.vts[i]->elts,
			    a_bld.vts[i]->num_elts));
    }
    result *= (a_blt.num_vts == a_bld.num_vts);
    result *= (a_blt.num_es == a_bld.num_es);
    adj_lst_uint64_free(&a_blt);
    adj_lst_uint64_free(&a_bld);
    graph_uint64_free(&g_blt);
    graph_uint64_free(&g_bld);
  }
  printf("\t\tcorrectness across all builds --> ");
  print_test_result(result);
}


/** 
   Test adj_lst_uint64_rand_dir and adj_lst_uint64_rand_undir on the number 
   of edges in expectation.
*/

void rand_build_test_helper(void (*rand_build_fn)(adj_lst_uint64_t *,
						  uint64_t,
						  uint32_t,
						  uint32_t));

void run_adj_lst_uint64_rand_dir_test(){
  printf("Test adj_lst_uint64_rand_dir on the number of edges in "
	 "expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1) * (0.5 * 1)\n");
  rand_build_test_helper(adj_lst_uint64_rand_dir);
}

void run_adj_lst_uint64_rand_undir_test(){
  printf("Test adj_lst_uint64_rand_undir on the number of edges in "
	 "expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1)/2 * (0.5 * 2)\n");
  rand_build_test_helper(adj_lst_uint64_rand_undir);
}

void rand_build_test_helper(void (*rand_build_fn)(adj_lst_uint64_t *,
						  uint64_t,
						  uint32_t,
						  uint32_t)){
  adj_lst_uint64_t a;
  uint64_t n;
  int pow_two_start = 10;
  int pow_two_end = 15;
  uint32_t num = 1;
  uint32_t denom = 2;
  for (int i = pow_two_start; i < pow_two_end; i++){
    n = pow_two_uint64(i);
    rand_build_fn(&a, n, num, denom);
    printf("\t\tvertices: %lu, expected directed edges: %.1f, "
	   "directed edges: %lu\n",
	   a.num_vts, 0.5 * (float)(a.num_vts * (a.num_vts - 1)), a.num_es);
    fflush(stdout);
    adj_lst_uint64_free(&a);
  }
}

uint64_t uint64_sum(uint64_t *a, uint64_t num_elts){
  uint64_t ret = 0;
  for (uint64_t i = 0; i < num_elts; i++){
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
  run_uint64_wts_graph_test();
  run_double_wts_graph_test();
  run_corner_cases_graph_test();
  run_adj_lst_uint64_undir_build_test();
  run_adj_lst_uint64_add_dir_edge_test();
  run_adj_lst_uint64_add_undir_edge_test();
  run_adj_lst_uint64_rand_dir_test();
  run_adj_lst_uint64_rand_undir_test();
  return 0;
}
