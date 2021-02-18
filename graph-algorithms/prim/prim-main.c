/**
   prim-main.c

   Tests of Prim's algorithm with a hash table parameter across
   i) default, division-based and multiplication-based hash tables, and ii)
   edge weight types.

   Requirements for running tests: 
   - UINT64_MAX must be defined
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "prim.h"
#include "heap.h"
#include "ht-div-uint64.h"
#include "ht-mul-uint64.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

#define RGENS_SEED() do{srandom(time(0)); srand48(random());}while (0)
#define RANDOM() (random())
#define DRAND48() (drand48())

static const size_t NR = SIZE_MAX; //not reached as index

uint64_t pow_two(int k);
void print_uint64_elts(const stack_t *s);
void print_double_elts(const stack_t *s);
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *));
void print_uint64_arr(const uint64_t *arr, uint64_t n);
void print_double_arr(const double *arr, uint64_t n);
void print_test_result(int res);

/**
   Initialize small graphs with uint64_t weights.
*/

void graph_uint64_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  graph_base_init(g, 5, sizeof(uint64_t));
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

void graph_uint64_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(uint64_t));
}

/**
   Run a test on small graphs with uint64_t weights.
*/
  
int cmp_uint64(const void *a, const void *b){
  if (*(uint64_t *)a > *(uint64_t *)b){
    return 1;
  }else if (*(uint64_t *)a < *(uint64_t *)b){
    return -1;
  }else{
    return 0;
  }
}

typedef struct{
  float alpha;
} context_t;

void ht_div_uint64_init_helper(ht_div_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       void (*free_elt)(void *),
			       void *context){
  context_t *c = context;
  ht_div_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     free_elt);
}

void ht_mul_uint64_init_helper(ht_mul_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       void (*free_elt)(void *),
			       void *context){
  context_t * c = context;
  ht_mul_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     NULL, //vertex is hash key
		     free_elt);
}

void run_default_uint64_prim(const adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, NULL, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_uint64_prim(const adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_uint64_init_helper;
  hht.insert = (heap_ht_insert)ht_div_uint64_insert;
  hht.search = (heap_ht_search)ht_div_uint64_search;
  hht.remove = (heap_ht_remove)ht_div_uint64_remove;
  hht.free = (heap_ht_free)ht_div_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, &hht, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_uint64_prim(const adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  hht.search = (heap_ht_search)ht_mul_uint64_search;
  hht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  hht.free = (heap_ht_free)ht_mul_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, &hht, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}
  
void run_uint64_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint64_wts_init(&g);
  printf("Running a test on an undirected uint64_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_prim(&a);
  run_div_uint64_prim(&a);
  run_mul_uint64_prim(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint64_wts_no_edges_init(&g);
  printf("Running a test on a undirected uint64_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_prim(&a);
  run_div_uint64_prim(&a);
  run_mul_uint64_prim(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Initialize small graphs with double weights.
*/

void graph_double_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_base_init(g, 5, sizeof(double));
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

void graph_double_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(double));
}

/**
   Run a test on small graphs with double weights.
*/
  
int cmp_double(const void *a, const void *b){
  if (*(double *)a > *(double *)b){
    return 1;
  }else if (*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  } 
}

void run_default_double_prim(const adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (size_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, NULL, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_double_prim(const adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_uint64_init_helper;
  hht.insert = (heap_ht_insert)ht_div_uint64_insert;
  hht.search = (heap_ht_search)ht_div_uint64_search;
  hht.remove = (heap_ht_remove)ht_div_uint64_remove;
  hht.free = (heap_ht_free)ht_div_uint64_free;
  for (size_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, &hht, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_double_prim(const adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  hht.search = (heap_ht_search)ht_mul_uint64_search;
  hht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  hht.free = (heap_ht_free)ht_mul_uint64_free;
  for (size_t i = 0; i < a->num_vts; i++){
    prim(a, i, dist, prev, &hht, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running a test on an undirected double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_prim(&a);
  run_div_double_prim(&a);
  run_mul_double_prim(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_wts_no_edges_init(&g);
  printf("Running a test on a undirected double graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_prim(&a);
  run_div_double_prim(&a);
  run_mul_double_prim(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Construct adjacency lists of random undirected graphs with random 
    weights.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.000000) return 1;
  if (b->p <= 0.000000) return 0;
  if (b->p > DRAND48()) return 1;
  return 0;
}

void add_undir_uint64_edge(adj_lst_t *a,
			   uint64_t u,
			   uint64_t v,
			   uint64_t wt_l,
			   uint64_t wt_h,
			   int (*bern)(void *),
			   void *arg){
  uint64_t rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_undir_edge(a, u, v, &rand_val, bern, arg);
}

void add_undir_double_edge(adj_lst_t *a,
			   uint64_t u,
			   uint64_t v,
			   uint64_t wt_l,
			   uint64_t wt_h,
			   int (*bern)(void *),
			   void *arg){
  double rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_undir_edge(a, u, v, &rand_val, bern, arg);
}

void adj_lst_rand_undir_wts(adj_lst_t *a,
			    uint64_t n,
			    int wt_size,
			    uint64_t wt_l,
			    uint64_t wt_h,
			    int (*bern)(void *),
			    void *arg,
			    void (*add_undir_edge)(adj_lst_t *,
						   uint64_t,
						   uint64_t,
						   uint64_t,
						   uint64_t,
						   int (*)(void *),
						   void *)){
  graph_t g;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
      add_undir_edge(a, i, j, wt_l, wt_h, bern, arg);
    }
  }
  graph_free(&g);
}

/**
   Runs a test on random undirected graphs with random uint64_t weights,
   across default, division-based and multiplication-based hash tables.
*/

void sum_mst_edges(uint64_t *wt_mst,
		   uint64_t *num_mst_vts,
		   uint64_t num_vts,
		   const uint64_t *dist,
		   const uint64_t *prev){
  *wt_mst = 0;
  *num_mst_vts = 0;
  for(uint64_t i = 0; i < num_vts; i++){
    if (prev[i] != NR){
      *wt_mst += dist[i];
      (*num_mst_vts)++;
    }
  }
}

void run_rand_uint64_test(){
  int pow_two_start = 0, pow_two_end = 14;
  int iter = 10;
  int res = 1;
  int num_p = 7;
  uint64_t wt_mst_def, wt_mst_div, wt_mst_mul;
  uint64_t num_mst_vts_def, num_mst_vts_div, num_mst_vts_mul;
  uint64_t n, rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two(32) - 1;
  uint64_t *dist = NULL, *prev = NULL;
  float alpha_div = 1.0, alpha_mul = 0.4;
  double p[7] = {1.000000, 0.250000, 0.062500,
		 0.015625, 0.003906, 0.000977,
		 0.000000};
  adj_lst_t a;
  bern_arg_t b;
  ht_div_uint64_t ht_div;
  ht_mul_uint64_t ht_mul;
  context_t context_div, context_mul;
  heap_ht_t hht_div, hht_mul;
  clock_t t_def, t_div, t_mul;
  context_div.alpha = alpha_div;
  hht_div.ht = &ht_div;
  hht_div.context = &context_div;
  hht_div.init = (heap_ht_init)ht_div_uint64_init_helper;
  hht_div.insert = (heap_ht_insert)ht_div_uint64_insert;
  hht_div.search = (heap_ht_search)ht_div_uint64_search;
  hht_div.remove = (heap_ht_remove)ht_div_uint64_remove;
  hht_div.free = (heap_ht_free)ht_div_uint64_free;
  context_mul.alpha = alpha_mul;
  hht_mul.ht = &ht_mul;
  hht_mul.context = &context_mul;
  hht_mul.init = (heap_ht_init)ht_mul_uint64_init_helper;
  hht_mul.insert = (heap_ht_insert)ht_mul_uint64_insert;
  hht_mul.search = (heap_ht_search)ht_mul_uint64_search;
  hht_mul.remove = (heap_ht_remove)ht_mul_uint64_remove;
  hht_mul.free = (heap_ht_free)ht_mul_uint64_free;
  printf("Run a prim test on random undirected graphs with random "
	 "uint64_t weights;\nan edge is represented by two directed edges "
	 "with a weight in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two(i); //0 < n
      dist = malloc_perror(n * sizeof(uint64_t));
      prev = malloc_perror(n * sizeof(uint64_t));
      adj_lst_rand_undir_wts(&a,
			     n,
			     sizeof(uint64_t),
			     wt_l,
			     wt_h,
			     bern,
			     &b,
			     add_undir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = DRAND48() * (n - 1);
      }
      t_def = clock();
      for(int j = 0; j < iter; j++){
	prim(&a,
	     rand_start[j],
	     dist,
	     prev,
	     NULL,
	     cmp_uint64);
      }
      t_def = clock() - t_def;
      sum_mst_edges(&wt_mst_def, &num_mst_vts_def, a.num_vts, dist, prev);
      t_div = clock();
      for(int j = 0; j < iter; j++){
	prim(&a,
	     rand_start[j],
	     dist,
	     prev,
	     &hht_div,
	     cmp_uint64);
      }
      t_div = clock() - t_div;
      sum_mst_edges(&wt_mst_div, &num_mst_vts_div, a.num_vts, dist, prev);
      t_mul = clock();
      for(int j = 0; j < iter; j++){
	prim(&a,
	     rand_start[j],
	     dist,
	     prev,
	     &hht_mul,
	     cmp_uint64);
      }
      t_mul = clock() - t_mul;
      sum_mst_edges(&wt_mst_mul, &num_mst_vts_mul, a.num_vts, dist, prev);
      res *= (wt_mst_def == wt_mst_div &&
	      wt_mst_div == wt_mst_mul);
      res *= (num_mst_vts_def == num_mst_vts_div &&
	      num_mst_vts_div == num_mst_vts_mul);
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tprim default ht ave runtime:     %.8f seconds\n"
	     "\t\t\tprim ht_div_uint64 ave runtime:  %.8f seconds\n"
	     "\t\t\tprim ht_mul_uint64 ave runtime:  %.8f seconds\n",
	     (float)t_def / iter / CLOCKS_PER_SEC,
	     (float)t_div / iter / CLOCKS_PER_SEC,
	     (float)t_mul / iter / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                     ");
      print_test_result(res);
      printf("\t\t\tlast mst # edges:                %lu\n",
	     num_mst_vts_def - 1);
      if (num_mst_vts_def > 1){
	printf("\t\t\tlast mst ave edge weight:        %.1lf\n",
	       (double)wt_mst_def / (num_mst_vts_def - 1));
      }else{
	printf("\t\t\tlast mst ave edge weight:        none\n");
      }
      res = 1;
      adj_lst_free(&a);
      free(dist);
      free(prev);
      dist = NULL;
      prev = NULL;
    }
  }
}

/* Helper functions */

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two(int k){
  uint64_t ret = 1;
  return ret << k;
}

/**
   Printing functions.
*/

void print_uint64_elts(const stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(const stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *)){
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

void print_uint64_arr(const uint64_t *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (arr[i] == NR){
      printf("NR ");
    }else{
      printf("%lu ", arr[i]);
    }
  }
  printf("\n");
} 

void print_double_arr(const double *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    printf("%.2lf ", arr[i]);
  }
  printf("\n");
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_uint64_graph_test();
  run_double_graph_test();
  run_rand_uint64_test();
  return 0;
}