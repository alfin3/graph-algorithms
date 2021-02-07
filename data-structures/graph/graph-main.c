/**
   graph-main.c

   Tests of graphs with generic weights.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

#define DRAND48() (drand48())

uint64_t sum(const uint64_t *a, uint64_t num_elts);
uint64_t pow_two(int k);
void print_test_result(int res);

/** 
   Test adj_lst_{init, dir_build, undir_build, free} on a small graph
   with edges and uint64_t or double weights.
*/

/**
   Initializes a small graph with uint64_t weights.
*/
void uint64_graph_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  uint64_t num_vts = 5;
  graph_base_init(g, num_vts, sizeof(uint64_t));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

/**
   Initializes a small graph with double weights.
*/
void double_graph_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t num_vts = 5;
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_base_init(g, num_vts, sizeof(double));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

/**
   Printing helper functions.
*/

void print_uint64_elts(const stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(const stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%0.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(const adj_lst_t *a,
		   void (*print_wts)(const stack_t *)){
  printf("\tvertices: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint64_elts(a->vts[i]);
  }
  if (print_wts != NULL){
    printf("\tweights: \n");
    for (uint64_t i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts(a->wts[i]);
    }
  }
  printf("\n");
}

/**
   Runs a test of adj_lst_{init, dir_build, undir_build, free} on a 
   small graph with edges and uint64_t weights. The test relies on the 
   construction order in adj_lst_{dir_build, undir_build}.
*/
void uint64_graph_helper(const adj_lst_t *a,
			 uint64_t split[],
			 uint64_t vts[],
			 uint64_t wts[]);

void run_uint64_graph_test(){
  uint64_t split_dir[] = {3, 1, 0, 0, 0};
  uint64_t vts_dir[] = {1, 2, 3, 3};
  uint64_t wts_dir[] = {4, 3, 2, 1};
  uint64_t split_undir[] = {3, 2, 1, 2, 0};
  uint64_t vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  uint64_t wts_undir[] = {4, 3, 2, 4, 1, 3, 2, 1};
  graph_t g;
  adj_lst_t a;
  uint64_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with uint64_t weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  uint64_graph_helper(&a, split_dir, vts_dir, wts_dir);
  print_adj_lst(&a, print_uint64_elts);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected "
	 "graph with uint64_t weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  uint64_graph_helper(&a, split_undir, vts_undir, wts_undir);
  print_adj_lst(&a, print_uint64_elts);
  adj_lst_free(&a);
  graph_free(&g);
}

void uint64_graph_helper(const adj_lst_t *a,
			 uint64_t split[],
			 uint64_t vts[],
			 uint64_t wts[]){
  int res = 1;
  uint64_t ix = 0;
  for (uint64_t i = 0; i < a->num_vts; i++){
    res *= (split[i] == a->vts[i]->num_elts);
    for (uint64_t j = 0; j < split[i]; j++){
      res *= (*((uint64_t *)a->vts[i]->elts + j) == vts[ix]);
      res *= (*((uint64_t *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(res);
}

/**
   Runs a test of adj_lst_{init, dir_build, undir_build, free} on a small
   graph with edges and double weights. The test relies on the construction
   order in adj_lst_{dir_build, undir_build}.
*/
void double_graph_helper(const adj_lst_t *a,
			 uint64_t split[],
			 uint64_t vts[],
			 double wts[]);

void run_double_graph_test(){
  uint64_t split_dir[] = {3, 1, 0, 0, 0};
  uint64_t vts_dir[] = {1, 2, 3, 3};
  double wts_dir[] = {4.0, 3.0, 2.0, 1.0};
  uint64_t split_undir[] = {3, 2, 1, 2, 0};
  uint64_t vts_undir[] = {1, 2, 3, 0, 3, 0, 0, 1};
  double wts_undir[] = {4.0, 3.0, 2.0, 4.0, 1.0, 3.0, 2.0, 1.0};
  graph_t g;
  adj_lst_t a;
  double_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with double weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  double_graph_helper(&a, split_dir, vts_dir, wts_dir);
  print_adj_lst(&a, print_double_elts);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected "
	 "graph with double weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  double_graph_helper(&a, split_undir, vts_undir, wts_undir); 
  print_adj_lst(&a, print_double_elts);
  adj_lst_free(&a);
  graph_free(&g);
}

void double_graph_helper(const adj_lst_t *a,
			 uint64_t split[],
			 uint64_t vts[],
			 double wts[]){
  int res = 1;
  uint64_t ix = 0;
  for (uint64_t i = 0; i < a->num_vts; i++){
    res *= (split[i] == a->vts[i]->num_elts);
    for (uint64_t j = 0; j < split[i]; j++){
      res *= (*((uint64_t *)a->vts[i]->elts + j) == vts[ix]);
      //== because of the same bit pattern
      res *= (*((double *)a->wts[i]->elts + j) == wts[ix]);
      ix++;
    }
  }
  print_test_result(res);
}

/** 
   Test adj_lst_{init, dir_build, undir_build, free} on corner cases,
   i.e. graphs with no edge weights and edges, and 0 or more vertices.
*/

void corner_cases_helper(const adj_lst_t *a, uint64_t num_vts, int *res);
  
void run_corner_cases_test(){
  int res = 1;
  uint64_t max_num_vts = 100;
  graph_t g;
  adj_lst_t a;
  for (uint64_t i = 0; i < max_num_vts; i++){
    graph_base_init(&g, i, 0);
    adj_lst_init(&a, &g);
    adj_lst_dir_build(&a, &g);
    res *= (a.num_vts == i &&
	    a.num_es == 0 &&
	    a.wt_size == 0 &&
	    a.wts == NULL);
    corner_cases_helper(&a, i, &res);
    adj_lst_free(&a);
    adj_lst_init(&a, &g);
    adj_lst_undir_build(&a, &g);
    res *= (a.num_vts == i &&
	    a.num_es == 0 &&
	    a.wt_size == 0 &&
	    a.wts == NULL);
    corner_cases_helper(&a, i, &res);
    adj_lst_free(&a);
    graph_free(&g);
  }
  printf("Test adj_lst_{init, dir_build, undir_build, free} "
	 "on corner cases --> ");
  print_test_result(res);
}

void corner_cases_helper(const adj_lst_t *a, uint64_t num_vts, int *res){
  if(num_vts){
    *res *= (a->vts != NULL);
    for(uint64_t i = 0; i < num_vts; i++){
      *res *= (a->vts[i]->num_elts == 0);
    }
  }else{
    *res *= (a->vts == NULL);
  }
}

/**
   Test adj_lst_undir_build.
*/

/**
   Initializes an unweighted graph that is i) a DAG with source 0 and 
   n(n - 1) / 2 edges in the directed form, and ii) complete in the 
   undirected form. n is greater than 1.
*/
void complete_graph_init(graph_t *g, uint64_t n){
  uint64_t num_es = (n * (n - 1)) / 2; //n * (n - 1) is even
  uint64_t ix = 0;
  graph_base_init(g, n, 0);
  g->num_es = num_es;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      g->u[ix] = i;
      g->v[ix] = j;
      ix++;
    }
  }
}

/**
   Runs a adj_lst_undir_build test on complete unweighted graphs.
*/
void run_adj_lst_undir_build_test(){
  int pow_end = 15;
  graph_t g;
  adj_lst_t a;
  clock_t t;
  printf("Test adj_lst_undir_build on complete unweighted graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) "
	 "directed edges \n");
  for (int i = 0; i < pow_end; i++){
    complete_graph_init(&g, pow_two(i));
    adj_lst_init(&a, &g);
    t = clock();
    adj_lst_undir_build(&a, &g);
    t = clock() - t;
    printf("\t\tvertices: %lu, "
	   "directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   a.num_vts, a.num_es, (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

/**
   Test adj_lst_add_dir_edge and adj_lst_add_undir_edge.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern_fn(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.0) return 1;
  if (b->p <= 0.0) return 0;
  if (b->p > DRAND48()) return 1;
  return 0;
}

void add_edge_helper(void (*build)(adj_lst_t *,
				   const graph_t *),
		     void (*add_edge)(adj_lst_t *,
				      uint64_t,
				      uint64_t,
				      const void *,
				      int (*)(void *),
				      void *));

void run_adj_lst_add_dir_edge_test(){
  printf("Test adj_lst_add_dir_edge on DAGs \n");
  printf("\tn vertices, 0 as source, n(n - 1)/2 directed edges \n");
  add_edge_helper(adj_lst_dir_build, adj_lst_add_dir_edge);
}

void run_adj_lst_add_undir_edge_test(){
  printf("Test adj_lst_add_undir_edge on complete graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) "
	 "directed edges \n");
  add_edge_helper(adj_lst_undir_build, adj_lst_add_undir_edge);
}

void add_edge_helper(void (*build)(adj_lst_t *,
				   const graph_t *),
		     void (*add_edge)(adj_lst_t *,
				      uint64_t,
				      uint64_t,
				      const void *,
				      int (*)(void *),
				      void *)){
  int res = 1, pow_end = 15;
  uint64_t n;
  bern_arg_t b;
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  clock_t t;
  b.p = 1.0;
  for (int i = 0; i < pow_end; i++){
    n = pow_two(i);
    complete_graph_init(&g_blt, n);
    graph_base_init(&g_bld, n, 0);
    adj_lst_init(&a_blt, &g_blt);
    adj_lst_init(&a_bld, &g_bld);
    build(&a_blt, &g_blt);
    build(&a_bld, &g_bld);
    t = clock();
    for (uint64_t i = 0; i < n - 1; i++){
      for (uint64_t j = i + 1; j < n; j++){
	add_edge(&a_bld, i, j, NULL, bern_fn, &b);
      }
    }
    t = clock() - t;
    printf("\t\tvertices: %lu, "
	   "directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   a_bld.num_vts, a_bld.num_es, (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    //sum test
    for (uint64_t i = 0; i < n; i++){
      res *= (a_blt.vts[i]->num_elts == a_bld.vts[i]->num_elts);
      res *= (sum(a_blt.vts[i]->elts, a_blt.vts[i]->num_elts) ==
	      sum(a_bld.vts[i]->elts, a_bld.vts[i]->num_elts));
    }
    res *= (a_blt.num_vts == a_bld.num_vts);
    res *= (a_blt.num_es == a_bld.num_es);
    adj_lst_free(&a_blt);
    adj_lst_free(&a_bld);
    graph_free(&g_blt);
    graph_free(&g_bld);
  }
  printf("\t\tcorrectness across all builds --> ");
  print_test_result(res);
}


/** 
   Test adj_lst_rand_dir and adj_lst_rand_undir on the number of edges
   in expectation.
*/

void rand_build_helper(void (*rand_build)(adj_lst_t *,
					  uint64_t,
					  int (*)(void *),
					  void *));

void run_adj_lst_rand_dir_test(){
  printf("Test adj_lst_rand_dir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1) * (0.5 * 1)\n");
  rand_build_helper(adj_lst_rand_dir);
}

void run_adj_lst_rand_undir_test(){
  printf("Test adj_lst_rand_undir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1)/2 * (0.5 * 2)\n");
  rand_build_helper(adj_lst_rand_undir);
}

void rand_build_helper(void (*rand_build)(adj_lst_t *,
					  uint64_t,
					  int (*)(void *),
					  void *)){
  int pow_end = 15;
  bern_arg_t b;
  adj_lst_t a;
  b.p = 0.5;
  for (int i = 0; i < pow_end; i++){
    rand_build(&a, pow_two(i), bern_fn, &b);
    printf("\t\tvertices: %lu, "
	   "expected directed edges: %.1f, "
	   "directed edges: %lu\n",
	   a.num_vts, 0.5 * a.num_vts * (a.num_vts - 1), a.num_es);
    fflush(stdout);
    adj_lst_free(&a);
  }
}

/**
   Sums num_elts elements of an uint64_t array.
*/
uint64_t sum(const uint64_t *a, uint64_t num_elts){
  uint64_t ret = 0;
  for (uint64_t i = 0; i < num_elts; i++){
    ret += a[i];
  }
  return ret;
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two(int k){
  uint64_t ret = 1;
  return ret << k;
} 

/**
   Prints a test result.
*/   
void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_graph_test();
  run_double_graph_test();
  run_corner_cases_test();
  run_adj_lst_undir_build_test();
  run_adj_lst_add_dir_edge_test();
  run_adj_lst_add_undir_edge_test();
  run_adj_lst_rand_dir_test();
  run_adj_lst_rand_undir_test();
  return 0;
}
