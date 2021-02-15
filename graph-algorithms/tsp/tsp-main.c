/**
   tsp-main.c

   Tests of a an exact solution of TSP without vertex revisiting
   across i) division and multiplication-based hash tables, and ii)
   weight types.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "tsp.h"
#include "ht-div-uint64.h"
#include "ht-mul-uint64.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-rand-mod.h"

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
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  uint64_t wts[] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2};
  graph_base_init(g, 4, sizeof(uint64_t));
  g->num_es = 12;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

void graph_uint64_single_vt_init(graph_t *g){
  graph_base_init(g, 1, sizeof(uint64_t));
}

/**
   Run a test on small graphs with uint64_t weights.
*/

void add_uint64(void *sum, const void *a, const void *b){
  *(uint64_t *)sum = *(uint64_t *)a + *(uint64_t *)b;
}
  
int cmp_uint64(const void *a, const void *b){
  if (*(uint64_t *)a > *(uint64_t *)b){
    return 1;
  }else if (*(uint64_t *)a < *(uint64_t *)b){
    return -1;
  }else{
    return 0;
  }
}

static void rdc_key_2blocks(void *t, const void *s){
  size_t r = 0;
  size_t *s_arr = (size_t *)s;
  for(size_t i = 0; i < 2; i++){
    r = sum_mod_uint64(r, s_arr[i], SIZE_MAX);
  }
  *(size_t *)t = r;
}

static void rdc_key_3blocks(void *t, const void *s){
  size_t r = 0;
  size_t *s_arr = (size_t *)s;
  for(size_t i = 0; i < 3; i++){
    r = sum_mod_uint64(r, s_arr[i], SIZE_MAX);
  }
  *(size_t *)t = r;
}

typedef struct{
  float alpha;
} context_div_t;

typedef struct{
  float alpha;
  void (*rdc_key)(void *, const void *);
} context_mul_t;

void ht_div_uint64_init_helper(ht_div_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       void (*free_elt)(void *),
			       void *context){
  context_div_t *c = context;
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
  context_mul_t * c = context;
  ht_mul_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     c->rdc_key,
		     free_elt);
}

void run_div_uint64_tsp(const adj_lst_t *a){
  int ret = -1;
  uint64_t dist;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  context_div_t context_div;
  tsp_ht_t tht;
  context_div.alpha = alpha;
  tht.ht = &ht_div;
  tht.context = &context_div;
  tht.init = (tsp_ht_init)ht_div_uint64_init_helper;
  tht.insert = (tsp_ht_insert)ht_div_uint64_insert;
  tht.search = (tsp_ht_search)ht_div_uint64_search;
  tht.remove = (tsp_ht_remove)ht_div_uint64_remove;
  tht.free = (tsp_ht_free)ht_div_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_uint64, cmp_uint64);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, i);
    print_uint64_arr(&dist, 1);
  }
  printf("\n");
}

void run_mul_uint64_tsp(const adj_lst_t *a){
  int ret = -1;
  uint64_t dist;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  context_mul_t context_mul;
  tsp_ht_t tht;
  context_mul.alpha = alpha;
  context_mul.rdc_key = rdc_key_2blocks;
  tht.ht = &ht_mul;
  tht.context = &context_mul;
  tht.init = (tsp_ht_init)ht_mul_uint64_init_helper;
  tht.insert = (tsp_ht_insert)ht_mul_uint64_insert;
  tht.search = (tsp_ht_search)ht_mul_uint64_search;
  tht.remove = (tsp_ht_remove)ht_mul_uint64_remove;
  tht.free = (tsp_ht_free)ht_mul_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_uint64, cmp_uint64);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, i);
    print_uint64_arr(&dist, 1);
  }
  printf("\n");
}


void run_uint64_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint64_wts_init(&g);
  printf("Running a test on a uint64_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_div_uint64_tsp(&a);
  run_mul_uint64_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint64_single_vt_init(&g);
  printf("Running a test on a uint64_t graph with a single vertex, with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_div_uint64_tsp(&a);
  run_mul_uint64_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Initialize small graphs with double weights.
*/

void graph_double_wts_init(graph_t *g){
  uint64_t u[] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 2, 1, 3};
  uint64_t v[] = {1, 2, 3, 0, 0, 1, 2, 3, 2, 0, 3, 1};
  double wts[] = {1.0, 1.0, 1.0, 1.0,
		  2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};
  graph_base_init(g, 4, sizeof(double));
  g->num_es = 12;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
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

void run_div_double_tsp(const adj_lst_t *a){
  int ret = -1;
  double dist;
  float alpha = 1.0;
  ht_div_uint64_t ht_div;
  context_div_t context_div;
  tsp_ht_t tht;
  context_div.alpha = alpha;
  tht.ht = &ht_div;
  tht.context = &context_div;
  tht.init = (tsp_ht_init)ht_div_uint64_init_helper;
  tht.insert = (tsp_ht_insert)ht_div_uint64_insert;
  tht.search = (tsp_ht_search)ht_div_uint64_search;
  tht.remove = (tsp_ht_remove)ht_div_uint64_remove;
  tht.free = (tsp_ht_free)ht_div_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, i);
    print_double_arr(&dist, 1);
  }
  printf("\n");
}

void run_mul_double_tsp(const adj_lst_t *a){
  int ret = -1;
  double dist;
  float alpha = 0.4;
  ht_mul_uint64_t ht_mul;
  context_mul_t context_mul;
  tsp_ht_t tht;
  context_mul.alpha = alpha;
  context_mul.rdc_key = rdc_key_2blocks;
  tht.ht = &ht_mul;
  tht.context = &context_mul;
  tht.init = (tsp_ht_init)ht_mul_uint64_init_helper;
  tht.insert = (tsp_ht_insert)ht_mul_uint64_insert;
  tht.search = (tsp_ht_search)ht_mul_uint64_search;
  tht.remove = (tsp_ht_remove)ht_mul_uint64_remove;
  tht.free = (tsp_ht_free)ht_mul_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    ret = tsp(a, i, &dist, &tht, add_double, cmp_double);
    printf("tsp ret: %d, tour length with %lu as start: ", ret, i);
    print_double_arr(&dist, 1);
  }
  printf("\n");
}


void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running a test on a double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_div_double_tsp(&a);
  run_mul_double_tsp(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_single_vt_init(&g);
  printf("Running a test on a double graph with a single vertex, with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
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
  if (b->p >= 1.000000) return 1;
  if (b->p <= 0.000000) return 0;
  if (b->p > DRAND48()) return 1;
  return 0;
}

void add_dir_uint64_edge(adj_lst_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint64_t wt_l,
			 uint64_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  uint64_t rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void add_dir_double_edge(adj_lst_t *a,
			 uint64_t u,
			 uint64_t v,
			 uint64_t wt_l,
			 uint64_t wt_h,
			 int (*bern)(void *),
			 void *arg){
  double rand_val = wt_l + DRAND48() * (wt_h - wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, bern, arg);
}

void adj_lst_rand_dir_wts(adj_lst_t *a,
			  uint64_t n,
			  int wt_size,
			  uint64_t wt_l,
			  uint64_t wt_h,
			  int (*bern)(void *),
			  void *arg,
			  void (*add_dir_edge)(adj_lst_t *,
					       uint64_t,
					       uint64_t,
					       uint64_t,
					       uint64_t,
					       int (*)(void *),
					       void *)){
  graph_t g;
  bern_arg_t arg_true;
  graph_base_init(&g, n, wt_size);
  adj_lst_init(a, &g);
  arg_true.p = 2.0;
  for (uint64_t i = 0; i < n - 1; i++){
    for (uint64_t j = i + 1; j < n; j++){
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
   Test tsp on random directed graphs with random uint64_t non-tour 
   weights and a known tour.
*/
void run_rand_uint64_test(){
  int num_vts_max = 21;
  int iter = 3;
  int res = 1;
  int ret_div = -1, ret_mul = -1;
  int num_p = 5;
  uint64_t n, rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two(32) - 1;
  uint64_t dist_div, dist_mul;
  float alpha_div = 1.0, alpha_mul = 0.4;
  double p[5] = {1.000000, 0.250000, 0.062500, 0.015625, 0.000000};
  adj_lst_t a;
  bern_arg_t b;
  ht_div_uint64_t ht_div;
  ht_mul_uint64_t ht_mul;
  context_div_t context_div;
  context_mul_t context_mul;
  tsp_ht_t tht_div, tht_mul;
  clock_t t_div, t_mul;
  context_div.alpha = alpha_div;
  tht_div.ht = &ht_div;
  tht_div.context = &context_div;
  tht_div.init = (tsp_ht_init)ht_div_uint64_init_helper;
  tht_div.insert = (tsp_ht_insert)ht_div_uint64_insert;
  tht_div.search = (tsp_ht_search)ht_div_uint64_search;
  tht_div.remove = (tsp_ht_remove)ht_div_uint64_remove;
  tht_div.free = (tsp_ht_free)ht_div_uint64_free;
  context_mul.alpha = alpha_mul;
  context_mul.rdc_key = rdc_key_2blocks;
  tht_mul.ht = &ht_mul;
  tht_mul.context = &context_mul;
  tht_mul.init = (tsp_ht_init)ht_mul_uint64_init_helper;
  tht_mul.insert = (tsp_ht_insert)ht_mul_uint64_insert;
  tht_mul.search = (tsp_ht_search)ht_mul_uint64_search;
  tht_mul.remove = (tsp_ht_remove)ht_mul_uint64_remove;
  tht_mul.free = (tsp_ht_free)ht_mul_uint64_free;
  printf("Run a tsp test on random directed graphs with random "
	 "uint64_t non-tour weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = 1; i < num_vts_max; i++){
      n = i;
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   wt_l,
			   wt_h,
			   bern,
			   &b,
			   add_dir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = DRAND48() * (n - 1);
      }
      t_div = clock();
      for(int j = 0; j < iter; j++){
	ret_div = tsp(&a,
		      rand_start[j],
		      &dist_div,
		      &tht_div,
		      add_uint64,
		      cmp_uint64);
      }
      t_div = clock() - t_div;
      t_mul = clock();
      for(int j = 0; j < iter; j++){
	ret_mul = tsp(&a,
		      rand_start[j],
		      &dist_mul,
		      &tht_mul,
		      add_uint64,
		      cmp_uint64);
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
	     a.num_vts, a.num_es);
      printf("\t\t\ttsp ht_div_uint64 ave runtime:  %.8f seconds\n"
	     "\t\t\ttsp ht_mul_uint64 ave runtime:  %.8f seconds\n",
	     (float)t_div / iter / CLOCKS_PER_SEC,
	     (float)t_mul / iter / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                    ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
    }
  }
}

/**
   Test tsp on sparse random directed graphs with random uint64_t non-tour 
   weights and a known tour.
*/
void run_sparse_rand_uint64_test(){
  int num_vts_start = 100, num_vts_end = 105;
  int iter = 3;
  int res = 1;
  int ret_div = -1, ret_mul = -1;
  int num_p = 2;
  uint64_t n, rand_start[iter];
  uint64_t wt_l = 0, wt_h = pow_two(32) - 1;
  uint64_t dist_div, dist_mul;
  float alpha_div = 1.0, alpha_mul = 0.4;
  double p[2] = {0.005000, 0.002500};
  adj_lst_t a;
  bern_arg_t b;
  ht_div_uint64_t ht_div;
  ht_mul_uint64_t ht_mul;
  context_div_t context_div;
  context_mul_t context_mul;
  tsp_ht_t tht_div, tht_mul;
  clock_t t_div, t_mul;
  context_div.alpha = alpha_div;
  tht_div.ht = &ht_div;
  tht_div.context = &context_div;
  tht_div.init = (tsp_ht_init)ht_div_uint64_init_helper;
  tht_div.insert = (tsp_ht_insert)ht_div_uint64_insert;
  tht_div.search = (tsp_ht_search)ht_div_uint64_search;
  tht_div.remove = (tsp_ht_remove)ht_div_uint64_remove;
  tht_div.free = (tsp_ht_free)ht_div_uint64_free;
  context_mul.alpha = alpha_mul;
  context_mul.rdc_key = rdc_key_3blocks;
  tht_mul.ht = &ht_mul;
  tht_mul.context = &context_mul;
  tht_mul.init = (tsp_ht_init)ht_mul_uint64_init_helper;
  tht_mul.insert = (tsp_ht_insert)ht_mul_uint64_insert;
  tht_mul.search = (tsp_ht_search)ht_mul_uint64_search;
  tht_mul.remove = (tsp_ht_remove)ht_mul_uint64_remove;
  tht_mul.free = (tsp_ht_free)ht_mul_uint64_free;
  printf("Run a tsp test on sparse random directed graphs with random "
	 "uint64_t non-tour weights in [%lu, %lu]\n", wt_l, wt_h);
  fflush(stdout);
  for (int pi = 0; pi < num_p; pi++){
    b.p = p[pi];
    printf("\tP[an edge is in a graph] = %.4f\n", p[pi]);
    for (int i = num_vts_start; i < num_vts_end; i++){
      n = i;
      adj_lst_rand_dir_wts(&a,
			   n,
			   sizeof(uint64_t),
			   wt_l,
			   wt_h,
			   bern,
			   &b,
			   add_dir_uint64_edge);
      for(int j = 0; j < iter; j++){
	rand_start[j] = DRAND48() * (n - 1);
      }
      t_div = clock();
      for(int j = 0; j < iter; j++){
	ret_div = tsp(&a,
		      rand_start[j],
		      &dist_div,
		      &tht_div,
		      add_uint64,
		      cmp_uint64);
      }
      t_div = clock() - t_div;
      t_mul = clock();
      for(int j = 0; j < iter; j++){
	ret_mul = tsp(&a,
		      rand_start[j],
		      &dist_mul,
		      &tht_mul,
		      add_uint64,
		      cmp_uint64);
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
	     a.num_vts, a.num_es);
      printf("\t\t\ttsp ht_div_uint64 ave runtime:  %.8f seconds\n"
	     "\t\t\ttsp ht_mul_uint64 ave runtime:  %.8f seconds\n",
	     (float)t_div / iter / CLOCKS_PER_SEC,
	     (float)t_mul / iter / CLOCKS_PER_SEC);
      printf("\t\t\tcorrectness:                    ");
      print_test_result(res);
      res = 1;
      adj_lst_free(&a);
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
  run_sparse_rand_uint64_test();
  return 0;
}
