/**
   dfs-test.c

   Tests of the DFS algorithm.

   The following command line arguments can be used to customize tests:
   dfs-test
   -  [0, # bits in size_t / 2] : a
   -  [0, # bits in size_t / 2] : b s.t. a <= |V| <= b for max edges test
   -  [0, # bits in size_t / 2] : c
   -  [0, # bits in size_t / 2] : d s.t. c <= |V| <= d for no edges test
   -  [0, # bits in size_t / 2]  : e
   -  [0, # bits in size_t / 2]  : f s.t. e <= |V| <= f for random graph test
   -  [0, 1] : on/off for small graph tests
   -  [0, 1] : on/off for max edges test
   -  [0, 1] : on/off for no edges test
   -  [0, 1] : on/off for random graph test

   usage examples: 
   ./dfs-test
   ./dfs-test 10 14 10 14 10 14
   ./dfs-test 10 14 10 14 10 14 0 1 1 1

   dfs-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation does not use stdint.h and is portable under C89/C90.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"

/* input handling */
const char *C_USAGE =
  "dfs-test \n"
  "[0, # bits in size_t / 2] : a \n"
  "[0, # bits in size_t / 2] : b s.t. a <= |V| <= b for max edges test \n"
  "[0, # bits in size_t / 2] : c \n"
  "[0, # bits in size_t / 2] : d s.t. c <= |V| <= d for no edges test \n"
  "[0, # bits in size_t / 2]  : e \n"
  "[0, # bits in size_t / 2]  : f s.t. e <= |V| <= f for random graph test \n"
  "[0, 1] : on/off for small graph tests \n"
  "[0, 1] : on/off for max edges test \n"
  "[0, 1] : on/off for no edges test \n"
  "[0, 1] : on/off for random graph test \n";
const int C_ARGC_MAX = 11;
const size_t C_ARGS_DEF[10] = {0, 14, 0, 14, 0, 14, 1, 1, 1, 1};
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* small graph test */
const size_t C_NUM_VTS_FIRST = 6;
const size_t C_NUM_ES_FIRST = 7;
const size_t C_U_FIRST[7] = {0, 1, 2, 3, 0, 4, 4};
const size_t C_V_FIRST[7] = {1, 2, 3, 1, 3, 2, 5};
const size_t C_START_FIRST = 0;
const size_t C_DIR_PRE_FIRST[6] = {0, 1, 2, 3, 8, 9};
const size_t C_DIR_POST_FIRST[6] = {7, 6, 5, 4, 11, 10};
const size_t C_UNDIR_PRE_FIRST[6] = {0, 1, 2, 3, 5, 6};
const size_t C_UNDIR_POST_FIRST[6] = {11, 10, 9, 4, 8, 7};

const size_t C_NUM_VTS_SECOND = 6;
const size_t C_NUM_ES_SECOND = 5;
const size_t C_U_SECOND[5] = {0, 1, 2, 3, 4};
const size_t C_V_SECOND[5] = {1, 2, 3, 4, 5};
const size_t C_START_SECOND = 0;
const size_t C_DIR_PRE_SECOND[6] = {0, 1, 2, 3, 4, 5};
const size_t C_DIR_POST_SECOND[6] = {11, 10, 9, 8, 7, 6};
const size_t C_UNDIR_PRE_SECOND[6] = {0, 1, 2, 3, 4, 5};
const size_t C_UNDIR_POST_SECOND[6] = {11, 10, 9, 8, 7, 6};

/* random graph tests */
const int C_ITER = 10;
const int C_PROBS_COUNT = 5;
const double C_PROBS[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;

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

int cmp_arr(const size_t *a, const size_t *b, size_t n);
size_t pow_two(int k);
void print_test_result(int res);

/**  
   Test dfs on small graphs.
*/

/**
   Initializes the first small graph.
*/
void first_graph_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS_FIRST, 0);
  g->num_es = C_NUM_ES_FIRST;
  g->u = malloc_perror(g->num_es, sizeof(size_t));
  g->v = malloc_perror(g->num_es, sizeof(size_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U_FIRST[i];
    g->v[i] = C_V_FIRST[i];
  }
}

/**
   Initializes the second small graph.
*/
void second_graph_init(graph_t *g){
  size_t i;
  graph_base_init(g, C_NUM_VTS_SECOND, 0);
  g->num_es = C_NUM_ES_SECOND;
  g->u = malloc_perror(g->num_es, sizeof(size_t));
  g->v = malloc_perror(g->num_es, sizeof(size_t));
  for (i = 0; i < g->num_es; i++){
    g->u[i] = C_U_SECOND[i];
    g->v[i] = C_V_SECOND[i];
  }
}

/**
   Run dfs tests on small graphs.
*/

void small_graph_helper(const graph_t *g,
			size_t start,
			const size_t ret_pre[],
			const size_t ret_post[],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res);

void run_first_graph_test(){
  int res = 1;
  graph_t g;
  printf("Run a dfs test on the first small graph instance --> ");
  first_graph_init(&g);
  small_graph_helper(&g,
		     C_START_FIRST,
		     C_DIR_PRE_FIRST,
		     C_DIR_POST_FIRST,
		     adj_lst_dir_build,
		     &res);
  small_graph_helper(&g,
		     C_START_FIRST,
		     C_UNDIR_PRE_FIRST,
		     C_UNDIR_POST_FIRST,
		     adj_lst_undir_build,
		     &res);
  graph_free(&g);
  print_test_result(res);
}

void run_second_graph_test(){
  int res = 1;
  graph_t g;
  printf("Run a dfs test on the second small graph instance --> ");
  second_graph_init(&g);
  small_graph_helper(&g,
		     C_START_SECOND,
		     C_DIR_PRE_SECOND,
		     C_DIR_POST_SECOND,
		     adj_lst_dir_build,
		     &res);
  small_graph_helper(&g,
		     C_START_SECOND,
		     C_UNDIR_PRE_SECOND,
		     C_UNDIR_POST_SECOND,
		     adj_lst_undir_build,
		     &res);
  graph_free(&g);
  print_test_result(res);
}

void small_graph_helper(const graph_t *g,
			size_t start,
			const size_t ret_pre[],
			const size_t ret_post[],
			void (*build)(adj_lst_t *, const graph_t *),
			int *res){
  size_t i;
  size_t *pre = NULL, *post = NULL;
  adj_lst_t a;
  adj_lst_init(&a, g);
  build(&a, g);
  pre = malloc_perror(a.num_vts, sizeof(size_t));
  post = malloc_perror(a.num_vts, sizeof(size_t));
  for (i = 0; i < a.num_vts; i++){
    dfs(&a, start, pre, post);
    *res *= cmp_arr(pre, ret_pre, a.num_vts);
    *res *= cmp_arr(post, ret_post, a.num_vts);
  }
  adj_lst_free(&a);
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**  
   Test dfs on large graphs.
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
   Runs a dfs test on directed graphs with n(n - 1) edges. The test relies
   on the construction order in adj_lst_rand_dir.
*/
void run_max_edges_graph_test(int pow_start, int pow_end){
  int res = 1;
  int i;
  size_t j, n, start;
  size_t *pre = NULL, *post = NULL;
  bern_arg_t b;
  adj_lst_t a;
  pre = malloc_perror(pow_two(pow_end), sizeof(size_t));
  post = malloc_perror(pow_two(pow_end), sizeof(size_t));
  b.p = C_PROB_ONE;
  printf("Run a dfs test on graphs with n vertices, where "
	 "2^%d <= n <= 2^%d, and n(n - 1) edges --> ", pow_start, pow_end);
  fflush(stdout);
  for (i = pow_start; i <= pow_end; i++){
    n = pow_two(i); /* n > 0 */
    adj_lst_rand_dir(&a, n, bern, &b);
    start =  RANDOM() % n;
    dfs(&a, start, pre, post);
    for (j = 0; j < n; j++){
      if (j == start){
	res *= (pre[j] == 0);
	res *= (post[j] == 2 * n - 1);
      }else if (j < start){
	/* n >= 2 */
	res *= (pre[j] == j + 1);
	res *= (post[j] == 2 * n - 2 - j);
      }else{
	res *= (pre[j] == j);
	res *= (post[j] == 2 * n - 1 - j);
      }
    }
    adj_lst_free(&a);
  }
  print_test_result(res);
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**
   Runs a dfs test on graphs with no edges.
*/
void run_no_edges_graph_test(int pow_start, int pow_end){
  int res = 1;
  int i;
  size_t j, n, start;
  size_t *pre = NULL, *post = NULL;
  bern_arg_t b;
  adj_lst_t a;
  pre = malloc_perror(pow_two(pow_end), sizeof(size_t));
  post = malloc_perror(pow_two(pow_end), sizeof(size_t));
  b.p = C_PROB_ZERO;
  printf("Run a dfs test on graphs with n vertices, where "
	 "2^%d <= n <= 2^%d, and no edges --> ", pow_start, pow_end);
  fflush(stdout);
  for (i = pow_start; i <= pow_end; i++){
    n = pow_two(i);
    adj_lst_rand_dir(&a, n, bern, &b);
    start =  RANDOM() % n;
    dfs(&a, start, pre, post);
    for (j = 0; j < n; j++){
      res *= (post[j] - pre[j] == 1);
    }
    adj_lst_free(&a);
  }
  print_test_result(res);
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**
   Runs a dfs test on random directed graphs.
*/
void run_random_dir_graph_test(int pow_start, int pow_end){
  int i, j, k;
  size_t n;
  size_t *start = NULL;
  size_t *pre = NULL, *post = NULL;
  bern_arg_t b;
  adj_lst_t a;
  clock_t t;
  printf("Run a dfs test on random directed graphs from %d random "
	 "start vertices in each graph \n", C_ITER);
  fflush(stdout);
  start = malloc_perror(C_ITER, sizeof(size_t));
  pre = malloc_perror(pow_two(pow_end), sizeof(size_t));
  post = malloc_perror(pow_two(pow_end), sizeof(size_t));
  for (i = 0; i < C_PROBS_COUNT; i++){
    b.p = C_PROBS[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (j = pow_start; j <= pow_end; j++){
      n = pow_two(j);
      adj_lst_rand_dir(&a, n, bern, &b);
      for (k = 0; k < C_ITER; k++){
	start[k] =  RANDOM() % n;
      }
      t = clock();
      for (k = 0; k < C_ITER; k++){
	dfs(&a, start[k], pre, post);
      }
      t = clock() - t;
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f, "
	     "average runtime: %.6f seconds\n",
	     TOLU(n), b.p * n * (n - 1), (float)t / C_ITER / CLOCKS_PER_SEC);
      fflush(stdout);
      adj_lst_free(&a);
    }
  }
  free(start);
  free(pre);
  free(post);
  start = NULL;
  pre = NULL;
  post = NULL;
}


/**
   Auxiliary functions.
*/

/**
   Compares the elements of two size_t arrays.
*/
int cmp_arr(const size_t *a, const size_t *b, size_t n){
  int res = 1;
  size_t i;
  for (i = 0; i < n; i++){
    res *= (a[i] == b[i]);
  }
  return res;
}

/**
   Returns the kth power of 2, where 0 <= k <= CHAR_BIT * sizeof(size_t) - 1.
*/
size_t pow_two(int k){
  size_t ret = 1;
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
      args[2] > C_FULL_BIT / 2 ||
      args[3] > C_FULL_BIT / 2 ||
      args[4] > C_FULL_BIT / 2 ||
      args[5] > C_FULL_BIT / 2 ||
      args[1] < args[0] ||
      args[3] < args[2] ||
      args[5] < args[4] ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[6]){
    run_first_graph_test();
    run_second_graph_test();
  }
  if (args[7]) run_max_edges_graph_test(args[0], args[1]);
  if (args[8]) run_no_edges_graph_test(args[2], args[3]);
  if (args[9]) run_random_dir_graph_test(args[4], args[5]);
  free(args);
  args = NULL;
  return 0;
}

