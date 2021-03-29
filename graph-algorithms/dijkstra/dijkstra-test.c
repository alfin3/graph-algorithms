/**
   dijkstra-test.c

   Tests of Dijkstra's algorithm with a hash table parameter across
   i) default, division-based and multiplication-based hash tables, and ii)
   edge weight types.

   Tests are designed to be run with and without -m32 and -m16 and include
   stdint.h that is required only for test purposes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "dijkstra.h"
#include "bfs.h"
#include "heap.h"
#include "ht-div.h"
#include "ht-mul.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

/* TODO CONDITIONALS HERE BASED ON SIZE_MAX */

void print_uint_elts(const stack_t *s);
void print_double_elts(const stack_t *s);
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *));
void print_uint_arr(const size_t *arr, size_t n);
void print_double_arr(const double *arr, size_t n);
void print_test_result(int res);

/**
   Initialize small graphs with size_t weights.
*/

void graph_uint_wts_init(graph_t *g){
  size_t u[] = {0, 0, 0, 1};
  size_t v[] = {1, 2, 3, 3};
  size_t wts[] = {4, 3, 2, 1};
  size_t i;
  graph_base_init(g, 5, sizeof(size_t));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(size_t));
  g->v = malloc_perror(g->num_es * sizeof(size_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((size_t *)g->wts + i) = wts[i];
  }
}

void graph_uint_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(size_t));
}

/**
   Run a test on small graphs with size_t weights.
*/

void add_uint(void *sum, const void *wt_a, const void *wt_b){
  *(size_t *)sum = *(size_t *)wt_a + *(size_t *)wt_b;
}
  
int cmp_uint(const void *a, const void *b){
  if (*(size_t *)a > *(size_t *)b){
    return 1;
  }else if (*(size_t *)a < *(size_t *)b){
    return -1;
  }else{
    return 0;
  }
}

typedef struct{
  float alpha;
} context_t;

void ht_div_init_helper(ht_div_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_t *c = context;
  ht_div_init(ht, key_size, elt_size, c->alpha, free_elt);
}

void ht_mul_init_helper(ht_mul_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_t * c = context;
  ht_mul_init(ht, key_size, elt_size, c->alpha, NULL, free_elt);
}

void run_default_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  dist = malloc_perror(a->num_vts * sizeof(size_t));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  float alpha = 1.0;
  ht_div_t ht_div;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(size_t));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  float alpha = 0.4;
  ht_mul_t ht_mul;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(size_t));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  context.alpha = alpha;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}
  
void run_uint_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint_wts_init(&g);
  printf("Running a test on a directed size_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_default_uint_dijkstra(&a);
  run_div_uint_dijkstra(&a);
  run_mul_uint_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected size_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_default_uint_dijkstra(&a);
  run_div_uint_dijkstra(&a);
  run_mul_uint_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint_wts_no_edges_init(&g);
  printf("Running a test on a directed size_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_default_uint_dijkstra(&a);
  run_div_uint_dijkstra(&a);
  run_mul_uint_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected size_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_default_uint_dijkstra(&a);
  run_div_uint_dijkstra(&a);
  run_mul_uint_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Initialize small graphs with double weights.
*/

void graph_double_wts_init(graph_t *g){
  size_t u[] = {0, 0, 0, 1};
  size_t v[] = {1, 2, 3, 3};
  size_t i;
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_base_init(g, 5, sizeof(double));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(size_t));
  g->v = malloc_perror(g->num_es * sizeof(size_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (i = 0; i < g->num_es; i++){
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

void add_double(void *sum, const void *wt_a, const void *wt_b){
  *(double *)sum = *(double *)wt_a + *(double *)wt_b;
}
  
int cmp_double(const void *a, const void *b){
  if (*(double *)a > *(double *)b){
    return 1;
  }else if (*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  } 
}

void run_default_double_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *prev = NULL;
  double *dist = NULL;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_double_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *prev = NULL;
  float alpha = 1.0;
  double *dist = NULL;
  ht_div_t ht_div;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  context.alpha = alpha;
  hht.ht = &ht_div;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_div_init_helper;
  hht.insert = (heap_ht_insert)ht_div_insert;
  hht.search = (heap_ht_search)ht_div_search;
  hht.remove = (heap_ht_remove)ht_div_remove;
  hht.free = (heap_ht_free)ht_div_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_double_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *prev = NULL;
  float alpha = 0.4;
  double *dist = NULL;
  ht_mul_t ht_mul;
  context_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(size_t));
  context.alpha = alpha;
  hht.ht = &ht_mul;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_mul_init_helper;
  hht.insert = (heap_ht_insert)ht_mul_insert;
  hht.search = (heap_ht_search)ht_mul_search;
  hht.remove = (heap_ht_remove)ht_mul_remove;
  hht.free = (heap_ht_free)ht_mul_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
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
  printf("Running a test on a directed double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_wts_no_edges_init(&g);
  printf("Running a test on a directed double graph with no edges, with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected double graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/** 
    Construct adjacency lists of random directed graphs with random 
    weights.
*/

typedef struct{
  double p;
} bern_arg_t;

int bern(void *arg){
  bern_arg_t *b = arg;
  if (b->p >= 1.0) return 1;
  if (b->p <= 0.0) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
}

void add_dir_uint_edge(adj_lst_t *a,
		       size_t u,
		       size_t v,
		       size_t wt_l,
		       size_t wt_h,
		       int (*bern)(void *),
		       void *arg){
  size_t rand_val = wt_l + DRAND() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void add_dir_double_edge(adj_lst_t *a,
			 size_t u,
			 size_t v,
			 size_t wt_l,
			 size_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  double rand_val = wt_l + DRAND() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void adj_lst_rand_dir_wts(adj_lst_t *a,
			  size_t n,
			  size_t wt_size,
			  size_t wt_l,
			  size_t wt_h,
			  int (*bern)(void *),
			  void *arg,
			  void (*add_dir_edge)(adj_lst_t *,
					       size_t,
					       size_t,
					       size_t,
					       size_t,
					       int (*)(void *),
					       void *)){
  size_t i, j;
  graph_t g;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  for (i = 0; i < n - 1; i++){
    for (j = i + 1; j < n; j++){
      add_dir_edge(a, i, j, wt_l, wt_h, bern, arg);
      add_dir_edge(a, j, i, wt_l, wt_h, bern, arg);
    }
  }
  graph_free(&g);
}

/**
   Run a test of distance equivalence of bfs and dijkstra on random
   directed graphs with the same size_t weight across edges, across
   default, division-based and multiplication-based hash tables.
*/

void norm_uint_arr(size_t *a, size_t norm, size_t n){
  size_t i;
  for (i = 0; i < n; i++){
    a[i] = a[i] / norm;
  }
}

void run_bfs_dijkstra_test(){
  int p, num_probs = 7;
  int i, pow_two_start = 0, pow_two_end = 14;
  int j, iter = 10;
  int res = 1;
  size_t n, rand_start[10];
  size_t *dist_bfs = NULL, *prev_bfs = NULL;
  size_t *dist = NULL, *prev = NULL;
  float alpha_div = 1.0, alpha_mul = 0.4;
  double probs[7] = {1.000000, 0.250000, 0.062500,
		     0.015625, 0.003906, 0.000977,
		     0.000000};
  adj_lst_t a;
  bern_arg_t b;
  ht_div_t ht_div;
  ht_mul_t ht_mul;
  context_t context_div, context_mul;
  heap_ht_t hht_div, hht_mul;
  clock_t t_bfs, t_def, t_div, t_mul;
  context_div.alpha = alpha_div;
  hht_div.ht = &ht_div;
  hht_div.context = &context_div;
  hht_div.init = (heap_ht_init)ht_div_init_helper;
  hht_div.insert = (heap_ht_insert)ht_div_insert;
  hht_div.search = (heap_ht_search)ht_div_search;
  hht_div.remove = (heap_ht_remove)ht_div_remove;
  hht_div.free = (heap_ht_free)ht_div_free;
  context_mul.alpha = alpha_mul;
  hht_mul.ht = &ht_mul;
  hht_mul.context = &context_mul;
  hht_mul.init = (heap_ht_init)ht_mul_init_helper;
  hht_mul.insert = (heap_ht_insert)ht_mul_insert;
  hht_mul.search = (heap_ht_search)ht_mul_search;
  hht_mul.remove = (heap_ht_remove)ht_mul_remove;
  hht_mul.free = (heap_ht_free)ht_mul_free;
  printf("Run a bfs and dijkstra test on random directed "
	 "graphs with the same weight across edges\n");
  fflush(stdout);
  for (p = 0; p < num_probs; p++){
    b.p = probs[p];
    printf("\tP[an edge is in a graph] = %.4f\n", probs[p]);
    for (i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two(i); /* 0 < n */
      dist_bfs = malloc_perror(n * sizeof(size_t));
      prev_bfs = malloc_perror(n * sizeof(size_t));
      dist = malloc_perror(n * sizeof(size_t));
      prev = malloc_perror(n * sizeof(size_t));
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(size_t),
			   i + 1, /* > 0 for normalization */
			   i + 1,
			   bern,
			   &b,
			   add_dir_uint_edge);
      for (j = 0; j < iter; j++){
	rand_start[j] = RANDOM() % n;
      }
      t_bfs = clock();
      for (j = 0; j < iter; j++){
	bfs(&a, rand_start[j], dist_bfs, prev_bfs);
      }
      t_bfs = clock() - t_bfs;
      t_def = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 NULL,
		 add_uint,
		 cmp_uint);
      }
      t_def = clock() - t_def;
      norm_uint_arr(dist, i + 1, n);
      res *= (memcmp(dist_bfs, dist, n * sizeof(size_t)) == 0);
      t_div = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_div,
		 add_uint,
		 cmp_uint);
      }
      t_div = clock() - t_div;
      norm_uint_arr(dist, i + 1, n);
      res *= (memcmp(dist_bfs, dist, n * sizeof(size_t)) == 0);
      t_mul = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_mul,
		 add_uint,
		 cmp_uint);
      }
      t_mul = clock() - t_mul;
      norm_uint_arr(dist, i + 1, n);
      res *= (memcmp(dist_bfs, dist, n * sizeof(size_t)) == 0);
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tbfs ave runtime:                     %.8f seconds\n"
	     "\t\t\tdijkstra default ht ave runtime:     %.8f seconds\n"
	     "\t\t\tdijkstra ht_div ave runtime:         %.8f seconds\n"
	     "\t\t\tdijkstra ht_mul ave runtime:         %.8f seconds\n",
	     (float)t_bfs / iter / CLOCKS_PER_SEC,
	     (float)t_def / iter / CLOCKS_PER_SEC,
	     (float)t_div / iter / CLOCKS_PER_SEC,
	     (float)t_mul / iter / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                         ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
      free(dist_bfs);
      free(prev_bfs);
      free(dist);
      free(prev);
      dist_bfs = NULL;
      prev_bfs = NULL;
      dist = NULL;
      prev = NULL;
    }
  }
}

/**
   Runs a test on random directed graphs with random size_t weights,
   across default, division-based and multiplication-based hash tables.
*/

void sum_paths(size_t *wt_paths,
	       size_t *num_paths,
	       size_t num_vts,
	       const size_t *dist,
	       const size_t *prev){
  size_t i;
  *wt_paths = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    if (prev[i] != SIZE_MAX){
      *wt_paths += dist[i];
      (*num_paths)++;
    }
  }
}

void run_rand_uint_test(){
  int p, num_probs = 7;
  int i, pow_two_start = 10, pow_two_end = 14;
  int j, iter = 10;
  int res = 1;
  size_t wt_paths_def, wt_paths_div, wt_paths_mul;
  size_t num_paths_def, num_paths_div, num_paths_mul;
  size_t n, rand_start[10];
  size_t wt_l = 0, wt_h = pow_two(32) - 1;
  size_t *dist = NULL, *prev = NULL;
  float alpha_div = 1.0, alpha_mul = 0.4;
  double probs[7] = {1.000000, 0.250000, 0.062500,
		     0.015625, 0.003906, 0.000977,
		     0.000000};
  adj_lst_t a;
  bern_arg_t b;
  ht_div_t ht_div;
  ht_mul_t ht_mul;
  context_t context_div, context_mul;
  heap_ht_t hht_div, hht_mul;
  clock_t t_def, t_div, t_mul;
  context_div.alpha = alpha_div;
  hht_div.ht = &ht_div;
  hht_div.context = &context_div;
  hht_div.init = (heap_ht_init)ht_div_init_helper;
  hht_div.insert = (heap_ht_insert)ht_div_insert;
  hht_div.search = (heap_ht_search)ht_div_search;
  hht_div.remove = (heap_ht_remove)ht_div_remove;
  hht_div.free = (heap_ht_free)ht_div_free;
  context_mul.alpha = alpha_mul;
  hht_mul.ht = &ht_mul;
  hht_mul.context = &context_mul;
  hht_mul.init = (heap_ht_init)ht_mul_init_helper;
  hht_mul.insert = (heap_ht_insert)ht_mul_insert;
  hht_mul.search = (heap_ht_search)ht_mul_search;
  hht_mul.remove = (heap_ht_remove)ht_mul_remove;
  hht_mul.free = (heap_ht_free)ht_mul_free;
  printf("Run a dijkstra test on random directed graphs with random "
	 "size_t weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  for (p = 0; p < num_probs; p++){
    b.p = probs[p];
    printf("\tP[an edge is in a graph] = %.4f\n", probs[p]);
    for (i = pow_two_start; i <  pow_two_end; i++){
      n = pow_two(i); /* 0 < n */
      dist = malloc_perror(n * sizeof(size_t));
      prev = malloc_perror(n * sizeof(size_t));
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(size_t),
			   wt_l,
			   wt_h,
			   bern,
			   &b,
			   add_dir_uint_edge);
      for (j = 0; j < iter; j++){
	rand_start[j] = RANDOM() % n;
      }
      t_def = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 NULL,
		 add_uint,
		 cmp_uint);
      }
      t_def = clock() - t_def;
      sum_paths(&wt_paths_def, &num_paths_def, a.num_vts, dist, prev);
      t_div = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_div,
		 add_uint,
		 cmp_uint);
      }
      t_div = clock() - t_div;
      sum_paths(&wt_paths_div, &num_paths_div, a.num_vts, dist, prev);
      t_mul = clock();
      for (j = 0; j < iter; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_mul,
		 add_uint,
		 cmp_uint);
      }
      t_mul = clock() - t_mul;
      sum_paths(&wt_paths_mul, &num_paths_mul, a.num_vts, dist, prev);
      res *= (wt_paths_def == wt_paths_div &&
	      wt_paths_div == wt_paths_mul);
      res *= (num_paths_def == num_paths_div &&
	      num_paths_div == num_paths_mul);
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     a.num_vts, a.num_es);
      printf("\t\t\tdijkstra default ht ave runtime:     %.8f seconds\n"
	     "\t\t\tdijkstra ht_div ave runtime:         %.8f seconds\n"
	     "\t\t\tdijkstra ht_mul ave runtime:         %.8f seconds\n",
	     (float)t_def / iter / CLOCKS_PER_SEC,
	     (float)t_div / iter / CLOCKS_PER_SEC,
	     (float)t_mul / iter / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                         ");
      print_test_result(res);
      printf("\t\t\tlast run # paths:                    %lu\n",
	     num_paths_def - 1);
      if (num_paths_def > 1){
	printf("\t\t\tlast run ave path weight:            %.1f\n",
	       (double)wt_paths_def / (num_paths_def - 1));
      }else{
	printf("\t\t\tlast run ave path weight:            none\n");
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

/**
   Printing functions.
*/

void print_uint_elts(const stack_t *s){
  size_t i;
  for (i = 0; i < s->num_elts; i++){
    printf("%lu ", *((size_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(const stack_t *s){
  size_t i;
  for (i = 0; i < s->num_elts; i++){
    printf("%.2f ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *)){
  size_t i;
  printf("\tvertices: \n");
  for (i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint_elts(a->vts[i]);
  }
  if (print_wts != NULL){
    printf("\tweights: \n");
    for (i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts(a->wts[i]);
    }
  }
  printf("\n");
}

void print_uint_arr(const size_t *arr, size_t n){
  size_t i;
  for (i = 0; i < n; i++){
    if (arr[i] == SIZE_MAX){
      printf("NR ");
    }else{
      printf("%lu ", arr[i]);
    }
  }
  printf("\n");
} 

void print_double_arr(const double *arr, size_t n){
  size_t i;
  for (i = 0; i < n; i++){
    printf("%.2f ", arr[i]);
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
  run_uint_graph_test();
  run_double_graph_test();
  run_bfs_dijkstra_test();
  run_rand_uint_test();
  return 0;
}
