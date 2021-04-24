/**
   graph-test.c

   Tests of graphs with generic weights.

   The following command line arguments can be used to customize tests:
   graph-test
      [0, # bits in size_t / 2] : n for 2^n vertices in smallest graph
      [0, # bits in size_t / 2] : n for 2^n vertices in largest graph
      [0, 1] : small graph test on/off
      [0, 1] : non-random graph test on/off
      [0, 1] : random graph test on/off

   usage examples: 
   ./graph-test
   ./graph-test 10 14
   ./graph-test 0 10 0 1 0
   ./graph-test 15 15 0 0 1

   graph-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the only requirement that CHAR_BIT * sizeof(size_t) is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
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
  "graph-test \n"
  "[0, # bits in size_t / 2] : n for 2^n vertices in smallest graph \n"
  "[0, # bits in size_t / 2] : n for 2^n vertices in largest graph \n"
  "[0, 1] : small graph test on/off \n"
  "[0, 1] : non-random graph test on/off \n"
  "[0, 1] : random graph test on/off \n";
const int C_ARGC_MAX = 6;
const size_t C_ARGS_DEF[5] = {0, 10, 1, 1, 1};
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* small graph test */
const size_t C_NUM_VTS = 5;
const size_t C_NUM_ES = 4;
const size_t C_U[4] = {0, 0, 0, 1};
const size_t C_V[4] = {1, 2, 3, 3};
const size_t C_WTS_UINT[4] = {4, 3, 2, 1};
const double C_WTS_DOUBLE[4] = {4.0, 3.0, 2.0, 1.0};

const size_t C_NUMS_DIR[5] = {3, 1, 0, 0, 0};
const size_t C_VTS_DIR[4] = {1, 2, 3, 3};
const size_t C_WTS_UINT_DIR[4] = {4, 3, 2, 1};
const double C_WTS_DOUBLE_DIR[4] = {4.0, 3.0, 2.0, 1.0};

const size_t C_NUMS_UNDIR[5] = {3, 2, 1, 2, 0};
const size_t C_VTS_UNDIR[8] = {1, 2, 3, 0, 3, 0, 0, 1};
const size_t C_WTS_UINT_UNDIR[8] = {4, 3, 2, 4, 1, 3, 2, 1};
const double C_WTS_DOUBLE_UNDIR[8] = {4.0, 3.0, 2.0, 4.0, 1.0, 3.0, 2.0, 1.0};

/* corner cases test */
const size_t C_CORNER_NUM_VTS_MAX = 100;

/* random graph test */
const double C_PROB_ONE = 1.0;
const double C_PROB_HALF = 0.5;
const double C_PROB_ZERO = 0.0;

size_t sum(const size_t *a, size_t num_elts);
void print_uint_elts(const stack_t *s);
void print_double_elts(const stack_t *s);
void print_adj_lst(const adj_lst_t *a, void (*print_wts)(const stack_t *));
void print_uint_arr(const size_t *arr, size_t n);
void print_double_arr(const double *arr, size_t n);
void print_test_result(int res);

/** 
   Test on small graphs.
*/

/**
   Initializes a small graph with size_t weights.
*/
void uint_graph_init(graph_t *g){
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

/**
   Initializes a small graph with double weights.
*/
void double_graph_init(graph_t *g){
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

/**
   Runs a test of adj_lst_{init, dir_build, undir_build, free} on a 
   small graph with edges and size_t weights. The test relies on the 
   construction order in adj_lst_{dir_build, undir_build}.
*/
void uint_graph_helper(const adj_lst_t *a,
		       const size_t nums[],
		       const size_t vts[],
		       const size_t wts[]);

void run_uint_graph_test(){
  graph_t g;
  adj_lst_t a;
  uint_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with size_t weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  uint_graph_helper(&a, C_NUMS_DIR, C_VTS_DIR, C_WTS_UINT_DIR);
  print_adj_lst(&a, print_uint_elts);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected "
	 "graph with size_t weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  uint_graph_helper(&a, C_NUMS_UNDIR, C_VTS_UNDIR, C_WTS_UINT_UNDIR);
  print_adj_lst(&a, print_uint_elts);
  adj_lst_free(&a);
  graph_free(&g);
}

void uint_graph_helper(const adj_lst_t *a,
		       const size_t nums[],
		       const size_t vts[],
		       const size_t wts[]){
  int res = 1;
  size_t ix = 0;
  size_t i, j;
  for (i = 0; i < a->num_vts; i++){
    res *= (nums[i] == a->vts[i]->num_elts);
    for (j = 0; j < nums[i]; j++){
      res *= (*((size_t *)a->vts[i]->elts + j) == vts[ix]);
      res *= (*((size_t *)a->wts[i]->elts + j) == wts[ix]);
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
			 const size_t nums[],
			 const size_t vts[],
			 const double wts[]);

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  double_graph_init(&g);
  printf("Test adj_lst_{init, dir_build, free} on a directed graph "
	 "with double weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  double_graph_helper(&a, C_NUMS_DIR, C_VTS_DIR, C_WTS_DOUBLE_DIR);
  print_adj_lst(&a, print_double_elts);
  adj_lst_free(&a);
  printf("Test adj_lst_{init, undir_build, free} on an undirected "
	 "graph with double weights --> ");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  double_graph_helper(&a, C_NUMS_UNDIR, C_VTS_UNDIR, C_WTS_DOUBLE_UNDIR); 
  print_adj_lst(&a, print_double_elts);
  adj_lst_free(&a);
  graph_free(&g);
}

void double_graph_helper(const adj_lst_t *a,
			 const size_t nums[],
			 const size_t vts[],
			 const double wts[]){
  int res = 1;
  size_t ix = 0;
  size_t i, j;
  for (i = 0; i < a->num_vts; i++){
    res *= (nums[i] == a->vts[i]->num_elts);
    for (j = 0; j < nums[i]; j++){
      res *= (*((size_t *)a->vts[i]->elts + j) == vts[ix]);
      /* == because of the same bit pattern */
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

void corner_cases_helper(const adj_lst_t *a, size_t num_vts, int *res);
  
void run_corner_cases_test(){
  int res = 1;
  size_t i;
  graph_t g;
  adj_lst_t a;
  for (i = 0; i < C_CORNER_NUM_VTS_MAX; i++){
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

void corner_cases_helper(const adj_lst_t *a, size_t num_vts, int *res){
  size_t i;
  if (num_vts){
    *res *= (a->vts != NULL);
    for(i = 0; i < num_vts; i++){
      *res *= (a->vts[i]->num_elts == 0);
    }
  }else{
    *res *= (a->vts == NULL);
  }
}

/**
   Test on non-random graphs.
*/

/**
   Initializes an unweighted graph that is i) a DAG with source 0 and 
   n(n - 1) / 2 edges in the directed form, and ii) complete in the 
   undirected form. n is greater than 1.
*/
void complete_graph_init(graph_t *g, size_t n){
  size_t num_es = (n * (n - 1)) / 2; /* n * (n - 1) is even */
  size_t ix = 0;
  size_t i, j;
  graph_base_init(g, n, 0);
  g->num_es = num_es;
  g->u = malloc_perror(g->num_es, sizeof(size_t));
  g->v = malloc_perror(g->num_es, sizeof(size_t));
  for (i = 0; i < n - 1; i++){
    for (j = i + 1; j < n; j++){
      g->u[ix] = i;
      g->v[ix] = j;
      ix++;
    }
  }
}

/**
   Runs a adj_lst_undir_build test on complete unweighted graphs.
*/
void run_adj_lst_undir_build_test(int pow_start, int pow_end){
  int p;
  graph_t g;
  adj_lst_t a;
  clock_t t;
  printf("Test adj_lst_undir_build on complete unweighted graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) "
	 "directed edges \n");
  for (p = pow_start; p <= pow_end; p++){
    complete_graph_init(&g, pow_two(p));
    adj_lst_init(&a, &g);
    t = clock();
    adj_lst_undir_build(&a, &g);
    t = clock() - t;
    printf("\t\tvertices: %lu, "
	   "directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   TOLU(a.num_vts), TOLU(a.num_es), (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    adj_lst_free(&a);
    graph_free(&g);
  }
}

/**
   Test on random graphs.
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

/**
   Test adj_lst_add_dir_edge and adj_lst_add_undir_edge.
*/

void add_edge_helper(int pow_start,
		     int pow_end,
		     void (*build)(adj_lst_t *,
				   const graph_t *),
		     void (*add_edge)(adj_lst_t *,
				      size_t,
				      size_t,
				      const void *,
				      int (*)(void *),
				      void *));

void run_adj_lst_add_dir_edge_test(int pow_start, int pow_end){
  printf("Test adj_lst_add_dir_edge on DAGs \n");
  printf("\tn vertices, 0 as source, n(n - 1)/2 directed edges \n");
  add_edge_helper(pow_start,
		  pow_end,
		  adj_lst_dir_build,
		  adj_lst_add_dir_edge);
}

void run_adj_lst_add_undir_edge_test(int pow_start, int pow_end){
  printf("Test adj_lst_add_undir_edge on complete graphs \n");
  printf("\tn vertices, n(n - 1)/2 edges represented by n(n - 1) "
	 "directed edges \n");
  add_edge_helper(pow_start,
		  pow_end,
		  adj_lst_undir_build,
		  adj_lst_add_undir_edge);
}

void add_edge_helper(int pow_start,
		     int pow_end,
		     void (*build)(adj_lst_t *,
				   const graph_t *),
		     void (*add_edge)(adj_lst_t *,
				      size_t,
				      size_t,
				      const void *,
				      int (*)(void *),
				      void *)){
  int res = 1;
  int p;
  size_t i, j, n;
  bern_arg_t b;
  graph_t g_blt, g_bld;
  adj_lst_t a_blt, a_bld;
  clock_t t;
  b.p = C_PROB_ONE;
  for (p = pow_start; p <= pow_end; p++){
    n = pow_two(p);
    complete_graph_init(&g_blt, n);
    graph_base_init(&g_bld, n, 0);
    adj_lst_init(&a_blt, &g_blt);
    adj_lst_init(&a_bld, &g_bld);
    build(&a_blt, &g_blt);
    build(&a_bld, &g_bld);
    t = clock();
    for (i = 0; i < n - 1; i++){
      for (j = i + 1; j < n; j++){
	add_edge(&a_bld, i, j, NULL, bern, &b);
      }
    }
    t = clock() - t;
    printf("\t\tvertices: %lu, "
	   "directed edges: %lu, "
	   "build time: %.6f seconds\n",
	   TOLU(a_bld.num_vts),
	   TOLU(a_bld.num_es),
	   (float)t / CLOCKS_PER_SEC);
    fflush(stdout);
    /* sum test; wraps around */
    for (i = 0; i < n; i++){
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
   Test adj_lst_rand_dir and adj_lst_rand_undir.
*/

void rand_build_helper(int pow_start,
		       int pow_end,
		       double prob,
		       void (*rand_build)(adj_lst_t *,
					  size_t,
					  int (*)(void *),
					  void *));

void run_adj_lst_rand_dir_test(int pow_start, int pow_end){
  printf("Test adj_lst_rand_dir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1) * (%.1f * 1)\n",
	 C_PROB_HALF);
  rand_build_helper(pow_start, pow_end, C_PROB_HALF, adj_lst_rand_dir);
}

void run_adj_lst_rand_undir_test(int pow_start, int pow_end){
  printf("Test adj_lst_rand_undir on the number of edges in expectation\n");
  printf("\tn vertices, E[# of directed edges] = n(n - 1)/2 * (%.1f * 2)\n",
	 C_PROB_HALF);
  rand_build_helper(pow_start, pow_end, C_PROB_HALF, adj_lst_rand_undir);
}

void rand_build_helper(int pow_start,
		       int pow_end,
		       double prob,
		       void (*rand_build)(adj_lst_t *,
					  size_t,
					  int (*)(void *),
					  void *)){
  int p;
  bern_arg_t b;
  adj_lst_t a;
  b.p = prob;
  for (p = pow_start; p <= pow_end; p++){
    rand_build(&a, pow_two(p), bern, &b);
    printf("\t\tvertices: %lu, "
	   "expected directed edges: %.1f, "
	   "directed edges: %lu\n",
	   TOLU(a.num_vts),
	   prob * a.num_vts * (a.num_vts - 1),
	   TOLU(a.num_es));
    fflush(stdout);
    adj_lst_free(&a);
  }
}

/**
   Auxiliary functions.
*/

/**
   Sums num_elts elements of an size_t array. Wraps around and
   does not check for overflow.
*/
size_t sum(const size_t *a, size_t num_elts){
  size_t ret = 0;
  size_t i;
  for (i = 0; i < num_elts; i++){
    ret += a[i];
  }
  return ret;
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
    run_corner_cases_test();
  }
  if (args[3]){
    run_adj_lst_undir_build_test(args[0], args[1]);
  }
  if (args[4]){
    run_adj_lst_add_dir_edge_test(args[0], args[1]);
    run_adj_lst_add_undir_edge_test(args[0], args[1]);
    run_adj_lst_rand_dir_test(args[0], args[1]);
    run_adj_lst_rand_undir_test(args[0], args[1]);
  }
  free(args);
  args = NULL;
  return 0;
}
