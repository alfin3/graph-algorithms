/**
   tsp-test.c

   Tests of an exact solution of TSP without vertex revisiting
   across i) default, division and multiplication-based hash tables, and ii)
   weight types.

   The following command line arguments can be used to customize tests:
   tsp-test:
   -  [1, # bits in size_t) : a
   -  [1, # bits in size_t) : b s.t. a <= |V| <= b for all hash tables test
   -  [1, # bits in size_t) : c
   -  [1, # bits in size_t) : d s.t. c <= |V| <= d for default hash table test
   -  [1, 8 * # bits in size_t]  : e
   -  [1, 8 * # bits in size_t]  : f s.t. e <= |V| <= f for sparse graph test
   -  [0, 1] : on/off for small graph test
   -  [0, 1] : on/off for all hash tables test
   -  [0, 1] : on/off for default hash table test
   -  [0, 1] : on/off for sparse graph test

   usage examples:
   ./tsp-test
   ./tsp-test 12 18 18 22 10 60
   ./tsp-test 12 18 18 22 100 105 0 0 1 1

   tsp-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "tsp.h"
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

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "tsp-test \n"
  "[1, # bits in size_t) : a \n"
  "[1, # bits in size_t) : b s.t. a <= |V| <= b for all hash tables test \n"
  "[1, # bits in size_t) : c \n"
  "[1, # bits in size_t) : d s.t. c <= |V| <= d for default hash table test \n"
  "[1, 8 * # bits in size_t]  : e \n"
  "[1, 8 * # bits in size_t]  : f s.t. e <= |V| <= f for sparse graph test \n"
  "[0, 1] : on/off for small graph test \n"
  "[0, 1] : on/off for all hash tables test \n"
  "[0, 1] : on/off for default hash table test \n"
  "[0, 1] : on/off for sparse graph test \n";
const int C_ARGC_MAX = 11;
const size_t C_ARGS_DEF[10] = {1, 20, 20, 21, 100, 104, 1, 1, 1, 1};
const size_t C_SPARSE_GRAPH_V_MAX = 8 * CHAR_BIT * sizeof(size_t);
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* hash table load factor upper bounds */
const float C_ALPHA_DIV = 1.0;
const float C_ALPHA_MUL = 0.4;

/* small graph test */
const size_t C_NUM_VTS = 4;
const size_t C_NUM_ES = 12;
const size_t C_U[12] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
const size_t C_V[12] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
const size_t C_WTS_UINT[12] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2};
const double C_WTS_DOUBLE[12] = {1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0,
				 2.0, 2.0, 2.0, 2.0, 2.0};

/* random graph tests */
const int C_ITER = 3;
const int C_PROBS_COUNT = 4;
const int C_SPARSE_PROBS_COUNT = 2;
const double C_PROBS[4] = {1.0000, 0.2500, 0.0625, 0.0000};
const double C_SPARSE_PROBS[2] = {0.0050, 0.0025};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_WEIGHT_HIGH = ((size_t)-1 >>
			      ((CHAR_BIT * sizeof(size_t) + 1) / 2));

void print_uint_elts(const stack_t *s);
void print_double_elts(const stack_t *s);
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *));
void print_uint_arr(const size_t *arr, size_t n);
void print_double_arr(const double *arr, size_t n);
void print_test_result(int res);
void fprintf_stderr_exit(const char *s, int line);

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

void graph_uint_single_vt_init(graph_t *g){
  graph_base_init(g, 1, sizeof(size_t));
}

/**
   Run a test on small graphs with size_t weights.
*/

void add_uint(void *sum, const void *a, const void *b){
  *(size_t *)sum = *(size_t *)a + *(size_t *)b;
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
} context_div_t;

typedef struct{
  float alpha;
  size_t (*rdc_key)(const void *, size_t);
} context_mul_t;

void ht_div_init_helper(ht_div_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_div_t *c = context;
  ht_div_init(ht, key_size, elt_size, c->alpha, free_elt);
}

void ht_mul_init_helper(ht_mul_t *ht,
			size_t key_size,
			size_t elt_size,
			void (*free_elt)(void *),
			void *context){
  context_mul_t * c = context;
  ht_mul_init(ht, key_size, elt_size, c->alpha, c->rdc_key, free_elt);
}

void run_def_uint_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t dist;
  size_t i;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, NULL, add_uint, cmp_uint);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_uint_arr(&dist, 1);
  }
  printf("\n");
}

void run_div_uint_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t dist;
  size_t i;
  ht_div_t ht_div;
  context_div_t context_div;
  tsp_ht_t tht;
  context_div.alpha = C_ALPHA_DIV;
  tht.ht = &ht_div;
  tht.context = &context_div;
  tht.init = (tsp_ht_init)ht_div_init_helper;
  tht.insert = (tsp_ht_insert)ht_div_insert;
  tht.search = (tsp_ht_search)ht_div_search;
  tht.remove = (tsp_ht_remove)ht_div_remove;
  tht.free = (tsp_ht_free)ht_div_free;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_uint, cmp_uint);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_uint_arr(&dist, 1);
  }
  printf("\n");
}

void run_mul_uint_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t dist;
  size_t i;
  ht_mul_t ht_mul;
  context_mul_t context_mul;
  tsp_ht_t tht;
  context_mul.alpha = C_ALPHA_MUL;
  context_mul.rdc_key = NULL;
  tht.ht = &ht_mul;
  tht.context = &context_mul;
  tht.init = (tsp_ht_init)ht_mul_init_helper;
  tht.insert = (tsp_ht_insert)ht_mul_insert;
  tht.search = (tsp_ht_search)ht_mul_search;
  tht.remove = (tsp_ht_remove)ht_mul_remove;
  tht.free = (tsp_ht_free)ht_mul_free;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_uint, cmp_uint);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_uint_arr(&dist, 1);
  }
  printf("\n");
}


void run_uint_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint_wts_init(&g);
  printf("Running a test on a size_t graph with a \n"
	 "i) default hash table \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_def_uint_tsp(&a);
  run_div_uint_tsp(&a);
  run_mul_uint_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint_single_vt_init(&g);
  printf("Running a test on a size_t graph with a single vertex, with a \n"
	 "i) default hash table \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint_elts);
  run_def_uint_tsp(&a);
  run_div_uint_tsp(&a);
  run_mul_uint_tsp(&a);
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

void graph_double_single_vt_init(graph_t *g){
  graph_base_init(g, 1, sizeof(double));
}

/**
   Run a test on small graphs with double weights.
*/

void add_double(void *sum, const void *a, const void *b){
  *(double *)sum = *(double *)a + *(double *)b;
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

void run_def_double_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t i;
  double dist;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, NULL, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_double_arr(&dist, 1);
  }
  printf("\n");
}

void run_div_double_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t i;
  double dist;
  ht_div_t ht_div;
  context_div_t context_div;
  tsp_ht_t tht;
  context_div.alpha = C_ALPHA_DIV;
  tht.ht = &ht_div;
  tht.context = &context_div;
  tht.init = (tsp_ht_init)ht_div_init_helper;
  tht.insert = (tsp_ht_insert)ht_div_insert;
  tht.search = (tsp_ht_search)ht_div_search;
  tht.remove = (tsp_ht_remove)ht_div_remove;
  tht.free = (tsp_ht_free)ht_div_free;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_double_arr(&dist, 1);
  }
  printf("\n");
}

void run_mul_double_tsp(const adj_lst_t *a){
  int ret = -1;
  size_t i;
  double dist;
  ht_mul_t ht_mul;
  context_mul_t context_mul;
  tsp_ht_t tht;
  context_mul.alpha = C_ALPHA_MUL;
  context_mul.rdc_key = NULL;
  tht.ht = &ht_mul;
  tht.context = &context_mul;
  tht.init = (tsp_ht_init)ht_mul_init_helper;
  tht.insert = (tsp_ht_insert)ht_mul_insert;
  tht.search = (tsp_ht_search)ht_mul_search;
  tht.remove = (tsp_ht_remove)ht_mul_remove;
  tht.free = (tsp_ht_free)ht_mul_free;
  for (i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, TOLU(i));
    print_double_arr(&dist, 1);
  }
  printf("\n");
}


void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running a test on a double graph with a \n"
	 "i) default hash table \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_def_double_tsp(&a);
  run_div_double_tsp(&a);
  run_mul_double_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_single_vt_init(&g);
  printf("Running a test on a double graph with a single vertex, with a \n"
	 "i) default hash table \n"
	 "ii) ht_div_t hash table \n"
	 "iii) ht_mul_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_def_double_tsp(&a);
  run_div_double_tsp(&a);
  run_mul_double_tsp(&a);
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
  if (b->p >= C_PROB_ONE) return 1;
  if (b->p <= C_PROB_ZERO) return 0;
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
  bern_arg_t arg_true;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  arg_true.p = C_PROB_ONE;
  for (i = 0; i < n - 1; i++){
    for (j = i + 1; j < n; j++){
      if (n == 2){
	add_dir_edge(a, i, j, 1, 1, bern, &arg_true);
	add_dir_edge(a, j, i, 1, 1, bern, &arg_true);
      }else if (j - i == 1){
	add_dir_edge(a, i, j, 1, 1, bern, &arg_true);
	add_dir_edge(a, j, i, wt_l, wt_h, bern, arg);
      }else if (i == 0 && j == n - 1){
	add_dir_edge(a, i, j, wt_l, wt_h, bern, arg);
	add_dir_edge(a, j, i, 1, 1, bern, &arg_true);
      }else{
	add_dir_edge(a, i, j, wt_l, wt_h, bern, arg);
	add_dir_edge(a, j, i, wt_l, wt_h, bern, arg);
      }
    }
  }
  graph_free(&g);
}

/**
   Tests tsp across all hash tables on random directed graphs with random
   size_t non-tour weights and a known tour.
*/
void run_rand_uint_test(int num_vts_start, int num_vts_end){
  int p, i, j;
  int res = 1;
  int ret_def = -1, ret_div = -1, ret_mul = -1;
  size_t n;
  size_t wt_l = 0, wt_h = C_WEIGHT_HIGH;
  size_t dist_def, dist_div, dist_mul;
  size_t *rand_start = NULL;
  adj_lst_t a;
  bern_arg_t b;
  ht_div_t ht_div;
  ht_mul_t ht_mul;
  context_div_t context_div;
  context_mul_t context_mul;
  tsp_ht_t tht_div, tht_mul;
  clock_t t_def, t_div, t_mul;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  context_div.alpha = C_ALPHA_DIV;
  tht_div.ht = &ht_div;
  tht_div.context = &context_div;
  tht_div.init = (tsp_ht_init)ht_div_init_helper;
  tht_div.insert = (tsp_ht_insert)ht_div_insert;
  tht_div.search = (tsp_ht_search)ht_div_search;
  tht_div.remove = (tsp_ht_remove)ht_div_remove;
  tht_div.free = (tsp_ht_free)ht_div_free;
  context_mul.alpha = C_ALPHA_MUL;
  context_mul.rdc_key = NULL;
  tht_mul.ht = &ht_mul;
  tht_mul.context = &context_mul;
  tht_mul.init = (tsp_ht_init)ht_mul_init_helper;
  tht_mul.insert = (tsp_ht_insert)ht_mul_insert;
  tht_mul.search = (tsp_ht_search)ht_mul_search;
  tht_mul.remove = (tsp_ht_remove)ht_mul_remove;
  tht_mul.free = (tsp_ht_free)ht_mul_free;
  printf("Run a tsp test across all hash tables on random directed graphs \n"
	 "with random size_t non-tour weights in [%lu, %lu]\n",
	 TOLU(wt_l), TOLU(wt_h));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = num_vts_start; i <= num_vts_end; i++){
      n = i;
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
	ret_def = tsp(&a,
		      rand_start[j],
		      &dist_def,
		      NULL,
		      add_uint,
		      cmp_uint);
      }
      t_def = clock() - t_def;
      t_div = clock();
      for (j = 0; j < C_ITER; j++){
	ret_div = tsp(&a,
		      rand_start[j],
		      &dist_div,
		      &tht_div,
		      add_uint,
		      cmp_uint);
      }
      t_div = clock() - t_div;
      t_mul = clock();
      for (j = 0; j < C_ITER; j++){
	ret_mul = tsp(&a,
		      rand_start[j],
		      &dist_mul,
		      &tht_mul,
		      add_uint,
		      cmp_uint);
      }
      t_mul = clock() - t_mul;
      if (n == 1){
	res *= (dist_def == 0 && ret_def == 0);
	res *= (dist_div == 0 && ret_div == 0);
	res *= (dist_mul == 0 && ret_mul == 0);
      }else{
	res *= (dist_def == n && ret_def == 0);
	res *= (dist_div == n && ret_div == 0);
	res *= (dist_mul == n && ret_mul == 0);
      }
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     TOLU(a.num_vts), TOLU(a.num_es));
      printf("\t\t\ttsp default ht ave runtime:     %.8f seconds\n"
	     "\t\t\ttsp ht_div ave runtime:         %.8f seconds\n"
	     "\t\t\ttsp ht_mul ave runtime:         %.8f seconds\n",
	     (float)t_def / C_ITER / CLOCKS_PER_SEC,
	     (float)t_div / C_ITER / CLOCKS_PER_SEC,
	     (float)t_mul / C_ITER / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                    ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
    }
  }
  free(rand_start);
  rand_start = NULL;
}

/**
   Tests tsp with a default hash table on directed graphs with
   random size_t non-tour weights and a known tour.
*/
void run_def_rand_uint_test(int num_vts_start, int num_vts_end){
  int i, j;
  int res = 1;
  int ret_def = -1;
  size_t n;
  size_t wt_l = 0, wt_h = C_WEIGHT_HIGH;
  size_t dist_def;
  size_t *rand_start = NULL;
  adj_lst_t a;
  bern_arg_t b;
  clock_t t_def;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  printf("Run a tsp test with a default hash table on directed graphs \n"
	 "with random size_t non-tour weights in [%lu, %lu]\n",
	 TOLU(wt_l), TOLU(wt_h));
  fflush(stdout);
  b.p = C_PROB_ONE;
  printf("\tP[an edge is in a graph] = %.4f\n", C_PROB_ONE);
  for (i = num_vts_start; i <= num_vts_end; i++){
    n = i;
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
      ret_def = tsp(&a,
		    rand_start[j],
		    &dist_def,
		    NULL,
		    add_uint,
		    cmp_uint);
    }
    t_def = clock() - t_def;
    if (n == 1){
      res *= (dist_def == 0 && ret_def == 0);
    }else{
      res *= (dist_def == n && ret_def == 0);
    }
    printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	   TOLU(a.num_vts), TOLU(a.num_es));
    printf("\t\t\ttsp default ht ave runtime:     %.8f seconds\n",
	   (float)t_def / C_ITER / CLOCKS_PER_SEC);
    printf("\t\t\tcorrectness:                    ");
    print_test_result(res);
    res = 1;
    adj_lst_free(&a);
  }
  free(rand_start);
  rand_start = NULL;
}

/**
   Tests tsp on sparse random directed graphs with random size_t non-tour 
   weights and a known tour.
*/
void run_sparse_rand_uint_test(int num_vts_start, int num_vts_end){
  int p, i, j;
  int res = 1;
  int ret_div = -1, ret_mul = -1;
  size_t n;
  size_t wt_l = 0, wt_h = C_WEIGHT_HIGH;
  size_t dist_div, dist_mul;
  size_t *rand_start = NULL;
  adj_lst_t a;
  bern_arg_t b;
  ht_div_t ht_div;
  ht_mul_t ht_mul;
  context_div_t context_div;
  context_mul_t context_mul;
  tsp_ht_t tht_div, tht_mul;
  clock_t t_div, t_mul;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  context_div.alpha = C_ALPHA_DIV;
  tht_div.ht = &ht_div;
  tht_div.context = &context_div;
  tht_div.init = (tsp_ht_init)ht_div_init_helper;
  tht_div.insert = (tsp_ht_insert)ht_div_insert;
  tht_div.search = (tsp_ht_search)ht_div_search;
  tht_div.remove = (tsp_ht_remove)ht_div_remove;
  tht_div.free = (tsp_ht_free)ht_div_free;
  context_mul.alpha = C_ALPHA_MUL;
  context_mul.rdc_key = NULL;
  tht_mul.ht = &ht_mul;
  tht_mul.context = &context_mul;
  tht_mul.init = (tsp_ht_init)ht_mul_init_helper;
  tht_mul.insert = (tsp_ht_insert)ht_mul_insert;
  tht_mul.search = (tsp_ht_search)ht_mul_search;
  tht_mul.remove = (tsp_ht_remove)ht_mul_remove;
  tht_mul.free = (tsp_ht_free)ht_mul_free;
  printf("Run a tsp test on sparse random directed graphs with random "
	 "size_t non-tour weights in [%lu, %lu]\n", TOLU(wt_l), TOLU(wt_h));
  fflush(stdout);
  for (p = 0; p < C_SPARSE_PROBS_COUNT; p++){
    b.p = C_SPARSE_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_SPARSE_PROBS[p]);
    for (i = num_vts_start; i <= num_vts_end; i++){
      n = i;
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
      t_div = clock();
      for (j = 0; j < C_ITER; j++){
	ret_div = tsp(&a,
		      rand_start[j],
		      &dist_div,
		      &tht_div,
		      add_uint,
		      cmp_uint);
      }
      t_div = clock() - t_div;
      t_mul = clock();
      for (j = 0; j < C_ITER; j++){
	ret_mul = tsp(&a,
		      rand_start[j],
		      &dist_mul,
		      &tht_mul,
		      add_uint,
		      cmp_uint);
      }
      t_mul = clock() - t_mul;
      if (n == 1){
	res *= (dist_div == 0 && ret_div == 0);
	res *= (dist_mul == 0 && ret_mul == 0);
      }else{
	res *= (dist_div == n && ret_div == 0);
	res *= (dist_mul == n && ret_mul == 0);
      }
      printf("\t\tvertices: %lu, # of directed edges: %lu\n",
	     TOLU(a.num_vts), TOLU(a.num_es));
      printf("\t\t\ttsp ht_div ave runtime:         %.8f seconds\n"
	     "\t\t\ttsp ht_mul ave runtime:         %.8f seconds\n",
	     (float)t_div / C_ITER / CLOCKS_PER_SEC,
	     (float)t_mul / C_ITER / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                    ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
    }
  }
  free(rand_start);
  rand_start = NULL;
}

/**
   Printing functions.
*/

void print_uint_elts(const stack_t *s){
  size_t i;
  for (i = 0; i < s->num_elts; i++){
    printf("%lu ", TOLU(*((size_t *)s->elts + i)));
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
    printf("\t%lu : ", TOLU(i));
    print_uint_elts(a->vts[i]);
  }
  if (print_wts != NULL){
    printf("\tweights: \n");
    for (i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", TOLU(i));
      print_wts(a->wts[i]);
    }
  }
  printf("\n");
}

void print_uint_arr(const size_t *arr, size_t n){
  size_t i;
  for (i = 0; i < n; i++){
    printf("%lu ", TOLU(arr[i]));
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

void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_MAX){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] < 1 ||
      args[0] > C_FULL_BIT - 1 ||
      args[1] < 1 ||
      args[1] > C_FULL_BIT - 1 ||
      args[2] < 1 ||
      args[2] > C_FULL_BIT - 1 ||
      args[3] < 1 ||
      args[3] > C_FULL_BIT - 1 ||
      args[4] < 1 ||
      args[4] > C_SPARSE_GRAPH_V_MAX ||
      args[5] < 1 ||
      args[5] > C_SPARSE_GRAPH_V_MAX ||
      args[0] > args[1] ||
      args[2] > args[3] ||
      args[4] > args[5] ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[6]){
    run_uint_graph_test();
    run_double_graph_test();
  }
  if (args[7]) run_rand_uint_test(args[0], args[1]);
  if (args[8]) run_def_rand_uint_test(args[2], args[3]);
  if (args[9]) run_sparse_rand_uint_test(args[4], args[5]);
  free(args);
  args = NULL;
  return 0;
}
