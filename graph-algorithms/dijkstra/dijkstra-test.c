/**
   dijkstra-test.c

   Tests of Dijkstra's algorithm with a hash table parameter across
   i) default, division-based and multiplication-based hash tables, and ii)
   edge weight types.

   The following command line arguments can be used to customize tests:
   dijkstra-test:
   -  [0, # bits in size_t / 2] : n for 2^n vertices in the smallest graph
   -  [0, # bits in size_t / 2] : n for 2^n vertices in the largest graph
   -  [0, 1] : small graph test on/off
   -  [0, 1] : bfs comparison test on/off
   -  [0, 1] : test on random graphs with random size_t weights on/off

   usage examples: 
   ./dijkstra-test
   ./dijkstra-test 10 14
   ./dijkstra-test 14 14 0 0 1

   dijkstra-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments, which are 0 for the first argument, 10 for the
   second argument, and 1 for the following arguments.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that CHAR_BIT * sizeof(size_t)
   is greater or equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "dijkstra.h"
#include "bfs.h"
#include "heap.h"
#include "ht-divchn.h"
#include "ht-muloa.h"
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

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "dijkstra-test \n"
  "[0, # bits in size_t / 2] : n for 2^n vertices in smallest graph\n"
  "[0, # bits in size_t / 2] : n for 2^n vertices in largest graph\n"
  "[0, 1] : small graph test on/off\n"
  "[0, 1] : bfs comparison test on/off\n"
  "[0, 1] : random graphs with random size_t weights test on/off\n";
const int C_ARGC_MAX = 6;
const size_t C_ARGS_DEF[5] = {0, 10, 1, 1, 1};

/* hash table load factor upper bounds */
const size_t C_ALPHA_N_DIVCHN = 1;
const size_t C_LOG_ALPHA_D_DIVCHN = 0;
const size_t C_ALPHA_N_MULOA = 13107;
const size_t C_LOG_ALPHA_D_MULOA = 15;

/* small graph tests */
const size_t C_NUM_VTS = 5;
const size_t C_NUM_ES = 4;
const size_t C_U[4] = {0, 0, 0, 1};
const size_t C_V[4] = {1, 2, 3, 3};
const size_t C_WTS_UINT[4] = {4, 3, 2, 1};
const double C_WTS_DOUBLE[4] = {4.0, 3.0, 2.0, 1.0};

/* bfs comparison and random uint graph tests */
const int C_ITER = 10;
const int C_PROBS_COUNT = 7;
const double C_PROBS[7] = {1.000000, 0.250000, 0.062500,
			   0.015625, 0.003906, 0.000977,
			   0.000000};
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_WEIGHT_HIGH = ((size_t)-1 >>
			      ((CHAR_BIT * sizeof(size_t) + 1) / 2));

void print_uint(const void *a);
void print_double(const void *a);
void print_adj_lst(const adj_lst_t *a, void (*print_wt)(const void *));
void print_uint_arr(const size_t *arr, size_t n);
void print_double_arr(const double *arr, size_t n);
void print_test_result(int res);

/**
   Initialize small graphs with size_t weights.
*/

void graph_uint_wts_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS, sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = malloc_perror(g->num_es, sizeof(size_t));
  g->v = malloc_perror(g->num_es, sizeof(size_t));
  g->wts = malloc_perror(g->num_es, g->wt_size);
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U[i];
    g->v[i] = C_V[i];
    *((size_t *)g->wts + i) = C_WTS_UINT[i];
  }
}

void graph_uint_wts_no_edges_init(graph_t *g){
  graph_base_init(g, C_NUM_VTS, sizeof(size_t));
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
  size_t alpha_n;
  size_t log_alpha_d;
} context_divchn_t;

typedef struct{
  size_t alpha_n;
  size_t log_alpha_d;
} context_muloa_t;

void ht_divchn_init_helper(ht_divchn_t *ht,
			   size_t key_size,
			   size_t elt_size,
			   void (*free_elt)(void *),
			   void *context){
  context_divchn_t *c = context;
  ht_divchn_init(ht,
		 key_size,
		 elt_size,
		 0,
		 c->alpha_n,
		 c->log_alpha_d,
		 free_elt);
}

void ht_muloa_init_helper(ht_muloa_t *ht,
			  size_t key_size,
			  size_t elt_size,
			  void (*free_elt)(void *),
			  void *context){
  context_muloa_t * c = context;
  ht_muloa_init(ht,
		key_size,
		elt_size,
		0,
		c->alpha_n,
		c->log_alpha_d,
		NULL,
		free_elt);
}

void run_default_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  dist = malloc_perror(a->num_vts, sizeof(size_t));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
    print_uint_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_divchn_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  ht_divchn_t ht_divchn;
  context_divchn_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts, sizeof(size_t));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  context.alpha_n = C_ALPHA_N_DIVCHN;
  context.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  hht.ht = &ht_divchn;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_divchn_init_helper;
  hht.insert = (heap_ht_insert)ht_divchn_insert;
  hht.search = (heap_ht_search)ht_divchn_search;
  hht.remove = (heap_ht_remove)ht_divchn_remove;
  hht.free = (heap_ht_free)ht_divchn_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
    print_uint_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_muloa_uint_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *dist = NULL;
  size_t *prev = NULL;
  ht_muloa_t ht_muloa;
  context_muloa_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts, sizeof(size_t));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  context.alpha_n = C_ALPHA_N_MULOA;
  context.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  hht.ht = &ht_muloa;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_muloa_init_helper;
  hht.insert = (heap_ht_insert)ht_muloa_insert;
  hht.search = (heap_ht_search)ht_muloa_search;
  hht.remove = (heap_ht_remove)ht_muloa_remove;
  hht.free = (heap_ht_free)ht_muloa_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_uint, cmp_uint);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
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
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint);
  run_default_uint_dijkstra(&a);
  run_divchn_uint_dijkstra(&a);
  run_muloa_uint_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected size_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint);
  run_default_uint_dijkstra(&a);
  run_divchn_uint_dijkstra(&a);
  run_muloa_uint_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint_wts_no_edges_init(&g);
  printf("Running a test on a directed size_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint);
  run_default_uint_dijkstra(&a);
  run_divchn_uint_dijkstra(&a);
  run_muloa_uint_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected size_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint);
  run_default_uint_dijkstra(&a);
  run_divchn_uint_dijkstra(&a);
  run_muloa_uint_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Initialize small graphs with double weights.
*/

void graph_double_wts_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS, sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = malloc_perror(g->num_es, sizeof(size_t));
  g->v = malloc_perror(g->num_es, sizeof(size_t));
  g->wts = malloc_perror(g->num_es, g->wt_size);
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U[i];
    g->v[i] = C_V[i];
    *((double *)g->wts + i) = C_WTS_DOUBLE[i];
  }
}

void graph_double_wts_no_edges_init(graph_t *g){
  graph_base_init(g, C_NUM_VTS, sizeof(double));
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
  dist = malloc_perror(a->num_vts, sizeof(double));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
    print_double_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_divchn_double_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *prev = NULL;
  double *dist = NULL;
  ht_divchn_t ht_divchn;
  context_divchn_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts, sizeof(double));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  context.alpha_n = C_ALPHA_N_DIVCHN;
  context.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  hht.ht = &ht_divchn;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_divchn_init_helper;
  hht.insert = (heap_ht_insert)ht_divchn_insert;
  hht.search = (heap_ht_search)ht_divchn_search;
  hht.remove = (heap_ht_remove)ht_divchn_remove;
  hht.free = (heap_ht_free)ht_divchn_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
    print_double_arr(dist, a->num_vts);
    print_uint_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_muloa_double_dijkstra(const adj_lst_t *a){
  size_t i;
  size_t *prev = NULL;
  double *dist = NULL;
  ht_muloa_t ht_muloa;
  context_muloa_t context;
  heap_ht_t hht;
  dist = malloc_perror(a->num_vts, sizeof(double));
  prev = malloc_perror(a->num_vts, sizeof(size_t));
  context.alpha_n = C_ALPHA_N_MULOA;
  context.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  hht.ht = &ht_muloa;
  hht.context = &context;
  hht.init = (heap_ht_init)ht_muloa_init_helper;
  hht.insert = (heap_ht_insert)ht_muloa_insert;
  hht.search = (heap_ht_search)ht_muloa_search;
  hht.remove = (heap_ht_remove)ht_muloa_remove;
  hht.free = (heap_ht_free)ht_muloa_free;
  for (i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &hht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", TOLU(i));
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
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double);
  run_default_double_dijkstra(&a);
  run_divchn_double_dijkstra(&a);
  run_muloa_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double);
  run_default_double_dijkstra(&a);
  run_divchn_double_dijkstra(&a);
  run_muloa_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_wts_no_edges_init(&g);
  printf("Running a test on a directed double graph with no edges, with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double);
  run_default_double_dijkstra(&a);
  run_divchn_double_dijkstra(&a);
  run_muloa_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected double graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_divchn_t hash table \n"
	 "iii) ht_muloa_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double);
  run_default_double_dijkstra(&a);
  run_divchn_double_dijkstra(&a);
  run_muloa_double_dijkstra(&a);
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

void run_bfs_dijkstra_test(int pow_start, int pow_end){
  int p, i, j;
  int res = 1;
  size_t n;
  size_t *rand_start = NULL;
  size_t *dist_bfs = NULL, *prev_bfs = NULL;
  size_t *dist = NULL, *prev = NULL;
  adj_lst_t a;
  bern_arg_t b;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  context_divchn_t context_divchn;
  context_muloa_t context_muloa;
  heap_ht_t hht_divchn, hht_muloa;
  clock_t t_bfs, t_def, t_divchn, t_muloa;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  dist_bfs = malloc_perror(pow_two(pow_end), sizeof(size_t));
  prev_bfs = malloc_perror(pow_two(pow_end), sizeof(size_t));
  dist = malloc_perror(pow_two(pow_end), sizeof(size_t));
  prev = malloc_perror(pow_two(pow_end), sizeof(size_t));
  context_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  context_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  hht_divchn.ht = &ht_divchn;
  hht_divchn.context = &context_divchn;
  hht_divchn.init = (heap_ht_init)ht_divchn_init_helper;
  hht_divchn.insert = (heap_ht_insert)ht_divchn_insert;
  hht_divchn.search = (heap_ht_search)ht_divchn_search;
  hht_divchn.remove = (heap_ht_remove)ht_divchn_remove;
  hht_divchn.free = (heap_ht_free)ht_divchn_free;
  context_muloa.alpha_n = C_ALPHA_N_MULOA;
  context_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  hht_muloa.ht = &ht_muloa;
  hht_muloa.context = &context_muloa;
  hht_muloa.init = (heap_ht_init)ht_muloa_init_helper;
  hht_muloa.insert = (heap_ht_insert)ht_muloa_insert;
  hht_muloa.search = (heap_ht_search)ht_muloa_search;
  hht_muloa.remove = (heap_ht_remove)ht_muloa_remove;
  hht_muloa.free = (heap_ht_free)ht_muloa_free;
  printf("Run a bfs and dijkstra test on random directed "
	 "graphs with the same weight across edges\n");
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = pow_start; i <= pow_end; i++){
      n = pow_two(i); /* 0 < n */
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(size_t),
			   i + 1, /* > 0 for normalization */
			   i + 1,
			   bern,
			   &b,
			   add_dir_uint_edge);
      for (j = 0; j < C_ITER; j++){
	rand_start[j] = RANDOM() % n;
      }
      t_bfs = clock();
      for (j = 0; j < C_ITER; j++){
	bfs(&a, rand_start[j], dist_bfs, prev_bfs);
      }
      t_bfs = clock() - t_bfs;
      t_def = clock();
      for (j = 0; j < C_ITER; j++){
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
      t_divchn = clock();
      for (j = 0; j < C_ITER; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_divchn,
		 add_uint,
		 cmp_uint);
      }
      t_divchn = clock() - t_divchn;
      norm_uint_arr(dist, i + 1, n);
      res *= (memcmp(dist_bfs, dist, n * sizeof(size_t)) == 0);
      t_muloa = clock();
      for (j = 0; j < C_ITER; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_muloa,
		 add_uint,
		 cmp_uint);
      }
      t_muloa = clock() - t_muloa;
      norm_uint_arr(dist, i + 1, n);
      res *= (memcmp(dist_bfs, dist, n * sizeof(size_t)) == 0);
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     TOLU(a.num_vts), TOLU(a.num_es));
      printf("\t\t\tbfs ave runtime:                     %.8f seconds\n"
	     "\t\t\tdijkstra default ht ave runtime:     %.8f seconds\n"
	     "\t\t\tdijkstra ht_divchn ave runtime:      %.8f seconds\n"
	     "\t\t\tdijkstra ht_muloa ave runtime:       %.8f seconds\n",
	     (float)t_bfs / C_ITER / CLOCKS_PER_SEC,
	     (float)t_def / C_ITER / CLOCKS_PER_SEC,
	     (float)t_divchn / C_ITER / CLOCKS_PER_SEC,
	     (float)t_muloa / C_ITER / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                         ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
    }
  }
  free(rand_start);
  free(dist_bfs);
  free(prev_bfs);
  free(dist);
  free(prev);
  rand_start = NULL;
  dist_bfs = NULL;
  prev_bfs = NULL;
  dist = NULL;
  prev = NULL;
}

/**
   Runs a test on random directed graphs with random size_t weights,
   across default, division-based and multiplication-based hash tables.
*/

/**
   Computes the sum across non-negative integer weights in an overflow-safe
   fashion by counting wrap-arounds. The total sum is # wraps * C_SIZE_MAX +
   # wraps + sum, which is amenable to division for averaging purposes.
*/
void wrap_sum(size_t *num_wraps,
	      size_t *sum,
	      size_t *num_paths,
	      size_t num_vts,
	      const size_t *dist,
	      const size_t *prev){
  size_t i;
  *num_wraps = 0;
  *sum = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    if (prev[i] != C_SIZE_MAX){
      if (C_SIZE_MAX - *sum < dist[i]) (*num_wraps)++;
      *sum += dist[i];
      (*num_paths)++;
    }
  }
}

void run_rand_uint_test(int pow_start, int pow_end){
  int p, i, j;
  int res = 1;
  size_t num_wraps_def, num_wraps_divchn, num_wraps_muloa;
  size_t sum_def, sum_divchn, sum_muloa;
  size_t num_paths_def, num_paths_divchn, num_paths_muloa;
  size_t n;
  size_t wt_l = 0, wt_h = C_WEIGHT_HIGH;
  size_t *rand_start = NULL;
  size_t *dist = NULL, *prev = NULL;
  adj_lst_t a;
  bern_arg_t b;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  context_divchn_t context_divchn;
  context_muloa_t context_muloa;
  heap_ht_t hht_divchn, hht_muloa;
  clock_t t_def, t_divchn, t_muloa;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  dist = malloc_perror(pow_two(pow_end), sizeof(size_t));
  prev = malloc_perror(pow_two(pow_end), sizeof(size_t));
  context_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  context_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  hht_divchn.ht = &ht_divchn;
  hht_divchn.context = &context_divchn;
  hht_divchn.init = (heap_ht_init)ht_divchn_init_helper;
  hht_divchn.insert = (heap_ht_insert)ht_divchn_insert;
  hht_divchn.search = (heap_ht_search)ht_divchn_search;
  hht_divchn.remove = (heap_ht_remove)ht_divchn_remove;
  hht_divchn.free = (heap_ht_free)ht_divchn_free;
  context_muloa.alpha_n = C_ALPHA_N_MULOA;
  context_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  hht_muloa.ht = &ht_muloa;
  hht_muloa.context = &context_muloa;
  hht_muloa.init = (heap_ht_init)ht_muloa_init_helper;
  hht_muloa.insert = (heap_ht_insert)ht_muloa_insert;
  hht_muloa.search = (heap_ht_search)ht_muloa_search;
  hht_muloa.remove = (heap_ht_remove)ht_muloa_remove;
  hht_muloa.free = (heap_ht_free)ht_muloa_free;
  printf("Run a dijkstra test on random directed graphs with random "
	 "size_t weights in [%lu, %lu]\n", TOLU(wt_l), TOLU(wt_h));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = pow_start; i <= pow_end; i++){
      n = pow_two(i); /* 0 < n */
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(size_t),
			   wt_l,
			   wt_h,
			   bern,
			   &b,
			   add_dir_uint_edge);
      for (j = 0; j < C_ITER; j++){
	rand_start[j] = RANDOM() % n;
      }
      t_def = clock();
      for (j = 0; j < C_ITER; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 NULL,
		 add_uint,
		 cmp_uint);
      }
      t_def = clock() - t_def;
      wrap_sum(&num_wraps_def,
	       &sum_def,
	       &num_paths_def,
	       a.num_vts,
	       dist,
	       prev);
      t_divchn = clock();
      for (j = 0; j < C_ITER; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_divchn,
		 add_uint,
		 cmp_uint);
      }
      t_divchn = clock() - t_divchn;
      wrap_sum(&num_wraps_divchn,
	       &sum_divchn,
	       &num_paths_divchn,
	       a.num_vts,
	       dist,
	       prev);
      t_muloa = clock();
      for (j = 0; j < C_ITER; j++){
	dijkstra(&a,
		 rand_start[j],
		 dist,
		 prev,
		 &hht_muloa,
		 add_uint,
		 cmp_uint);
      }
      t_muloa = clock() - t_muloa;
      wrap_sum(&num_wraps_muloa,
	       &sum_muloa,
	       &num_paths_muloa,
	       a.num_vts,
	       dist,
	       prev);
      res *= (num_wraps_def == num_wraps_divchn &&
	      num_wraps_divchn == num_wraps_muloa);
      res *= (sum_def == sum_divchn &&
	      sum_divchn == sum_muloa);
      res *= (num_paths_def == num_paths_divchn &&
	      num_paths_divchn == num_paths_muloa);
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     TOLU(a.num_vts), TOLU(a.num_es));
      printf("\t\t\tdijkstra default ht ave runtime:     %.8f seconds\n"
	     "\t\t\tdijkstra ht_divchn ave runtime:      %.8f seconds\n"
	     "\t\t\tdijkstra ht_muloa ave runtime:       %.8f seconds\n",
	     (float)t_def / C_ITER / CLOCKS_PER_SEC,
	     (float)t_divchn / C_ITER / CLOCKS_PER_SEC,
	     (float)t_muloa / C_ITER / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                         ");
      print_test_result(res);
      printf("\t\t\tlast run # paths:                    %lu\n",
	     TOLU(num_paths_def) - 1);
      if (num_paths_def > 1){
	printf("\t\t\tlast run ave path weight:            %.1f\n",
	       (num_wraps_def * ((double)C_SIZE_MAX / (num_paths_def - 1)) +
		(double)num_wraps_def / (num_paths_def - 1) +
		(double)sum_def / (num_paths_def - 1)));
      }else{
	printf("\t\t\tlast run ave path weight:            none\n");
      }
      res = 1;
      adj_lst_free(&a);
    }
  }
  free(rand_start);
  free(dist);
  free(prev);
  rand_start = NULL;
  dist = NULL;
  prev = NULL;
}

/**
   Printing functions.
*/

void print_uint(const void *a){
  printf("%lu ", TOLU(*(size_t *)a));
}

void print_double(const void *a){
  printf("%.2f ", *(double *)a);
}
  
void print_adj_lst(const adj_lst_t *a, void (*print_wt)(const void *)){
  const char *p = NULL, *p_start = NULL, *p_end = NULL;
  size_t i;
  printf("\tvertices: \n");
  for (i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", TOLU(i));
    p_start = a->vt_wts[i]->elts;
    p_end = p_start + a->vt_wts[i]->num_elts * a->step_size;
    for (p = p_start; p != p_end; p += a->step_size){
      printf("%lu ", TOLU(*(const size_t *)p));
    }
    printf("\n");
  }
  if (a->wt_size > 0 && print_wt != NULL){
    printf("\tweights: \n");
    for (i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", TOLU(i));
      p_start = a->vt_wts[i]->elts;
      p_end = p_start + a->vt_wts[i]->num_elts * a->step_size;
      for (p = p_start; p != p_end; p += a->step_size){
	print_wt(p + sizeof(size_t));
      }
      printf("\n");
    }
  }
}

void print_uint_arr(const size_t *arr, size_t n){
  size_t i;
  for (i = 0; i < n; i++){
    if (arr[i] == C_SIZE_MAX){
      printf("NR ");
    }else{
      printf("%lu ", TOLU(arr[i]));
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

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_MAX){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_FULL_BIT / 2 ||
      args[1] > C_FULL_BIT / 2 ||
      args[1] < args[0] ||
      args[2] > 1 ||
      args[3] > 1 ||
      args[4] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[2]){
    run_uint_graph_test();
    run_double_graph_test();
  }
  if (args[3]) run_bfs_dijkstra_test(args[0], args[1]);
  if (args[4]) run_rand_uint_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
