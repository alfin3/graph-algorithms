/**
   dfs-test.c

   Tests of the DFS algorithm across graphs with different integer types
   of vertices.

   The following command line arguments can be used to customize tests:
   dfs-test
     [0, ushort width - 1) : a
     [0, ushort width - 1) : b s.t. 2**a <= V <= 2**b for max edges test
     [0, ushort width - 1) : c
     [0, ushort width - 1) : d s.t. 2**c <= V <= 2**d for no edges test
     [0, ushort width - 1) : e
     [0, ushort width - 1) : f s.t. 2**e <= V <= 2**f for rand graph test
     [0, 1] : on/off for small graph tests
     [0, 1] : on/off for max edges test
     [0, 1] : on/off for no edges test
     [0, 1] : on/off for rand graph test

   usage examples: 
   ./dfs-test
   ./dfs-test 10 14 10 14 10 14
   ./dfs-test 10 14 10 14 10 14 0 1 1 1

   dfs-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that the number of value 
   bits (precision == width) of unsigned short is not less than 16 and is
   even *. 

   * currently CHAR_BIT * sizeof(unsigned short) is used to get the width
     of an unsigned short integer under the assumption that all bits
     participate in the value.
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
#include "utilities-mod.h"
#include "utilities-lim.h"

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
  "dfs-test\n"
  "[0, ushort width - 1) : a\n"
  "[0, ushort width - 1) : b s.t. 2**a <= V <= 2**b for max edges test\n"
  "[0, ushort width - 1) : c\n"
  "[0, ushort width - 1) : d s.t. 2**c <= V <= 2**d for no edges test\n"
  "[0, ushort width - 1) : e\n"
  "[0, ushort width - 1) : f s.t. 2**e <= V <= 2**f for rand graph test\n"
  "[0, 1] : on/off for small graph tests\n"
  "[0, 1] : on/off for max edges test\n"
  "[0, 1] : on/off for no edges test\n"
  "[0, 1] : on/off for rand graph test\n";
const int C_ARGC_MAX = 11;
const size_t C_ARGS_DEF[10] = {0u, 6u, 0u, 6u, 0u, 14u, 1u, 1u, 1u, 1u};
const size_t C_USHORT_BIT = UINT_WIDTH_FROM_MAX((unsigned short)-1);

/* small graph test */
const size_t C_NUM_VTS_A = 6u;
const size_t C_NUM_ES_A = 7u;
const size_t C_START_A = 0u;
const unsigned short C_USHORT_U_A[7] = {0u, 1u, 2u, 3u, 0u, 4u, 4u};
const unsigned short C_USHORT_V_A[7] = {1u, 2u, 3u, 1u, 3u, 2u, 5u};
const unsigned short C_USHORT_WTS_A[7] = {(unsigned short)-1, 1u,
					  (unsigned short)-1, 2u,
					  (unsigned short)-1, 3u,
					  (unsigned short)-1};
const unsigned short C_USHORT_DIR_PRE_A[6] = {0u, 1u, 2u, 3u, 8u, 9u};
const unsigned short C_USHORT_DIR_POST_A[6] = {7u, 6u, 5u, 4u, 11u, 10u};
const unsigned short C_USHORT_UNDIR_PRE_A[6] = {0u, 1u, 2u, 3u, 5u, 6u};
const unsigned short C_USHORT_UNDIR_POST_A[6] = {11u, 10u, 9u, 4u, 8u, 7u};

const unsigned long C_ULONG_U_A[7] = {0u, 1u, 2u, 3u, 0u, 4u, 4u};
const unsigned long C_ULONG_V_A[7] = {1u, 2u, 3u, 1u, 3u, 2u, 5u};
const unsigned long C_ULONG_WTS_A[7] = {(unsigned long)-1, 1u,
					(unsigned long)-1, 2u,
					(unsigned long)-1, 3u,
					(unsigned long)-1};
const unsigned long C_ULONG_DIR_PRE_A[6] = {0u, 1u, 2u, 3u, 8u, 9u};
const unsigned long C_ULONG_DIR_POST_A[6] = {7u, 6u, 5u, 4u, 11u, 10u};
const unsigned long C_ULONG_UNDIR_PRE_A[6] = {0u, 1u, 2u, 3u, 5u, 6u};
const unsigned long C_ULONG_UNDIR_POST_A[6] = {11u, 10u, 9u, 4u, 8u, 7u};

const size_t C_NUM_VTS_B = 6u;
const size_t C_NUM_ES_B = 5u;
const size_t C_START_B = 0u;
const unsigned short C_USHORT_U_B[5] = {0u, 1u, 2u, 3u, 4u};
const unsigned short C_USHORT_V_B[5] = {1u, 2u, 3u, 4u, 5u};
const unsigned long C_USHORT_WTS_B[5] = {1u, (unsigned short)-1,
					 2u, (unsigned short)-1,
					 3u};
const unsigned short C_USHORT_DIR_PRE_B[6] = {0u, 1u, 2u, 3u, 4u, 5u};
const unsigned short C_USHORT_DIR_POST_B[6] = {11u, 10u, 9u, 8u, 7u, 6u};
const unsigned short C_USHORT_UNDIR_PRE_B[6] = {0u, 1u, 2u, 3u, 4u, 5u};
const unsigned short C_USHORT_UNDIR_POST_B[6] = {11u, 10u, 9u, 8u, 7u, 6u};
const unsigned long C_ULONG_U_B[5] = {0u, 1u, 2u, 3u, 4u};
const unsigned long C_ULONG_V_B[5] = {1u, 2u, 3u, 4u, 5u};
const unsigned long C_ULONG_WTS_B[5] = {1u, (unsigned long)-1,
					2u, (unsigned long)-1,
					3u};
const unsigned long C_ULONG_DIR_PRE_B[6] = {0u, 1u, 2u, 3u, 4u, 5u};
const unsigned long C_ULONG_DIR_POST_B[6] = {11u, 10u, 9u, 8u, 7u, 6u};
const unsigned long C_ULONG_UNDIR_PRE_B[6] = {0u, 1u, 2u, 3u, 4u, 5u};
const unsigned long C_ULONG_UNDIR_POST_B[6] = {11u, 10u, 9u, 8u, 7u, 6u};

/* random graph tests */

const size_t C_FN_COUNT = 4;
size_t (* const C_READ[4])(const void *) ={
  graph_read_ushort,
  graph_read_uint,
  graph_read_ulong,
  graph_read_sz};
void (* const C_WRITE[4])(void *, size_t) ={
  graph_write_ushort,
  graph_write_uint,
  graph_write_ulong,
  graph_write_sz};
void *(* const C_AT[4])(const void *, const void *) ={
  graph_at_ushort,
  graph_at_uint,
  graph_at_ulong,
  graph_at_sz};
int (* const C_CMP[4])(const void *, const void *) ={
  graph_cmp_ushort,
  graph_cmp_uint,
  graph_cmp_ulong,
  graph_cmp_sz};
void (* const C_INCR[4])(void *) ={
  graph_incr_ushort,
  graph_incr_uint,
  graph_incr_ulong,
  graph_incr_sz};
const size_t C_VT_SIZES[4] = {
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t)};
const char *C_VT_TYPES[4] = {"ushort", "uint  ", "ulong ", "sz    "};
const size_t C_ITER = 10u;
const size_t C_PROBS_COUNT = 5u;
const double C_PROBS[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;

int cmp_arr(const void *a,
	    const void *b,
	    size_t size,
	    size_t n,
	    int (*cmp)(const void *, const void *));
static void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**
   Initialize small graphs.
*/

void ushort_none_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned short),
		  0);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned short *)C_USHORT_U_A;
  g->v = (unsigned short *)C_USHORT_V_A;
}

void ulong_none_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned long),
		  0);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned long *)C_ULONG_U_A;
  g->v = (unsigned long *)C_ULONG_V_A;
}

void ushort_ulong_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned short),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned short *)C_USHORT_U_A;
  g->v = (unsigned short *)C_USHORT_V_A;
  g->wts = (unsigned long *)C_ULONG_WTS_A;
}

void ulong_ushort_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned long),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned long *)C_ULONG_U_A;
  g->v = (unsigned long *)C_ULONG_V_A;
  g->wts = (unsigned short *)C_USHORT_WTS_A;
}

void ushort_none_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned short),
		  0);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned short *)C_USHORT_U_B;
  g->v = (unsigned short *)C_USHORT_V_B;
}

void ulong_none_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned long),
		  0);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned long *)C_ULONG_U_B;
  g->v = (unsigned long *)C_ULONG_V_B;
}

void ushort_ulong_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned short),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned short *)C_USHORT_U_B;
  g->v = (unsigned short *)C_USHORT_V_B;
  g->wts = (unsigned long *)C_ULONG_WTS_B;
}

void ulong_ushort_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned long),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned long *)C_ULONG_U_B;
  g->v = (unsigned long *)C_ULONG_V_B;
  g->wts = (unsigned short *)C_USHORT_WTS_B;
}

/**
   Run dfs tests on small graphs.
*/

void small_graph_helper(const graph_t *g,
			size_t start,
			const void *ret_pre,
			const void *ret_post,
			void (*build)(adj_lst_t *,
				      const graph_t *,
				      size_t (*)(const void *)),
			size_t (*read_vt)(const void *),
			void (*write_vt)(void *, size_t),
			void *(*at_vt)(const void *, const void *),
			int (*cmp_vt)(const void *, const void *),
			void (*incr_vt)(void *),
			int *res);

void run_graph_a_test(){
  int res = 1;
  graph_t g;
  printf("Run a dfs test on the first small graph with ushort "
	 "vertices --> ");
  ushort_none_graph_a_init(&g);
  small_graph_helper(&g,
		     C_START_A,
		     C_USHORT_DIR_PRE_A,
		     C_USHORT_DIR_POST_A,
		     adj_lst_dir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_START_A,
		     C_USHORT_UNDIR_PRE_A,
		     C_USHORT_UNDIR_POST_A,
		     adj_lst_undir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  ushort_ulong_graph_a_init(&g);
  small_graph_helper(&g,
		     C_START_A,
		     C_USHORT_DIR_PRE_A,
		     C_USHORT_DIR_POST_A,
		     adj_lst_dir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_START_A,
		     C_USHORT_UNDIR_PRE_A,
		     C_USHORT_UNDIR_POST_A,
		     adj_lst_undir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  print_test_result(res);
  res = 1;
  printf("Run a dfs test on the first small graph with ulong "
	 "vertices --> ");
  ulong_none_graph_a_init(&g);
  small_graph_helper(&g,
		     C_START_A,
		     C_ULONG_DIR_PRE_A,
		     C_ULONG_DIR_POST_A,
		     adj_lst_dir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_START_A,
		     C_ULONG_UNDIR_PRE_A,
		     C_ULONG_UNDIR_POST_A,
		     adj_lst_undir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  ulong_ushort_graph_a_init(&g);
  small_graph_helper(&g,
		     C_START_A,
		     C_ULONG_DIR_PRE_A,
		     C_ULONG_DIR_POST_A,
		     adj_lst_dir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_START_A,
		     C_ULONG_UNDIR_PRE_A,
		     C_ULONG_UNDIR_POST_A,
		     adj_lst_undir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  print_test_result(res);
}

void run_graph_b_test(){
  int res = 1;
  graph_t g;
  printf("Run a dfs test on the second small graph with ushort "
	 "vertices --> ");
  ushort_none_graph_b_init(&g);
  small_graph_helper(&g,
		     C_START_B,
		     C_USHORT_DIR_PRE_B,
		     C_USHORT_DIR_POST_B,
		     adj_lst_dir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_START_B,
		     C_USHORT_UNDIR_PRE_B,
		     C_USHORT_UNDIR_POST_B,
		     adj_lst_undir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  ushort_ulong_graph_b_init(&g);
  small_graph_helper(&g,
		     C_START_B,
		     C_USHORT_DIR_PRE_B,
		     C_USHORT_DIR_POST_B,
		     adj_lst_dir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_START_B,
		     C_USHORT_UNDIR_PRE_B,
		     C_USHORT_UNDIR_POST_B,
		     adj_lst_undir_build,
		     graph_read_ushort,
		     graph_write_ushort,
		     graph_at_ushort,
		     graph_cmp_ushort,
		     graph_incr_ushort,
		     &res);
  print_test_result(res);
  res = 1;
  printf("Run a dfs test on the second small graph with ulong "
	 "vertices --> ");
  ulong_none_graph_b_init(&g);
  small_graph_helper(&g,
		     C_START_B,
		     C_ULONG_DIR_PRE_B,
		     C_ULONG_DIR_POST_B,
		     adj_lst_dir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_START_B,
		     C_ULONG_UNDIR_PRE_B,
		     C_ULONG_UNDIR_POST_B,
		     adj_lst_undir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  ulong_ushort_graph_b_init(&g);
  small_graph_helper(&g,
		     C_START_B,
		     C_ULONG_DIR_PRE_B,
		     C_ULONG_DIR_POST_B,
		     adj_lst_dir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_START_B,
		     C_ULONG_UNDIR_PRE_B,
		     C_ULONG_UNDIR_POST_B,
		     adj_lst_undir_build,
		     graph_read_ulong,
		     graph_write_ulong,
		     graph_at_ulong,
		     graph_cmp_ulong,
		     graph_incr_ulong,
		     &res);
   print_test_result(res);
}

void small_graph_helper(const graph_t *g,
			size_t start,
			const void *ret_pre,
			const void *ret_post,
			void (*build)(adj_lst_t *,
				      const graph_t *,
				      size_t (*)(const void *)),
			size_t (*read_vt)(const void *),
			void (*write_vt)(void *, size_t),
			void *(*at_vt)(const void *, const void *),
			int (*cmp_vt)(const void *, const void *),
			void (*incr_vt)(void *),
			int *res){
  void *pre = NULL, *post = NULL;
  adj_lst_t a;
  adj_lst_base_init(&a, g);
  build(&a, g, read_vt);
  pre = malloc_perror(a.num_vts, a.vt_size);
  post = malloc_perror(a.num_vts, a.vt_size);
  dfs(&a, start, pre, post, read_vt, write_vt, at_vt, cmp_vt, incr_vt);
  *res *= cmp_arr(pre, ret_pre, a.vt_size, a.num_vts, cmp_vt);
  *res *= cmp_arr(post, ret_post, a.vt_size, a.num_vts, cmp_vt);
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
void run_max_edges_graph_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t i, j, k;
  size_t num_vts;
  size_t start;
  size_t *pre = NULL, *post = NULL;
  graph_t g;
  adj_lst_t a;
  bern_arg_t b;
  b.p = C_PROB_ONE;
  printf("Run a dfs test on graphs with n vertices, where "
	 "2**%lu <= n <= 2**%lu, and n(n - 1) edges\n",
	 TOLU(log_start),  TOLU(log_end));
  for (i = log_start; i <= log_end; i++){
    num_vts = pow_two_perror(i); /* num_vts > 0 */
    printf("\t\tvertices: %lu\n", TOLU(num_vts));
    for (j = 0; j < C_FN_COUNT; j++){
      /* no declared type after realloc; effective type is set by dfs */
      pre = realloc_perror(pre, num_vts, C_VT_SIZES[j]);
      post = realloc_perror(post, num_vts, C_VT_SIZES[j]);
      graph_base_init(&g, num_vts, C_VT_SIZES[j], 0);
      adj_lst_base_init(&a, &g);
      adj_lst_rand_dir(&a, C_WRITE[j], bern, &b);
      start =  RANDOM() % num_vts;
      dfs(&a,
	  start,
	  pre,
	  post,
	  C_READ[j],
	  C_WRITE[j],
	  C_AT[j],
	  C_CMP[j],
	  C_INCR[j]);
      for (k = 0; k < num_vts; k++){
	if (k == start){
	  res *=
	    (C_READ[j](ptr(pre, k, C_VT_SIZES[j])) == 0 &&
	     C_READ[j](ptr(post, k, C_VT_SIZES[j])) == 2 * num_vts - 1);
	}else if (k < start){
	  res *=
	    (C_READ[j](ptr(pre, k, C_VT_SIZES[j])) == k + 1 &&
	     C_READ[j](ptr(post, k, C_VT_SIZES[j])) == 2 * num_vts - 2 - k);
	}else{
	  res *=
	    (C_READ[j](ptr(pre, k, C_VT_SIZES[j])) == k &&
	     C_READ[j](ptr(post, k, C_VT_SIZES[j])) == 2 * num_vts - 1 - k);
	}
      }
      printf("\t\t\t%s correctness:     ", C_VT_TYPES[j]);
      print_test_result(res);
      res = 1;
      adj_lst_free(&a); /* deallocates blocks with effective vertex type */
    }
  }
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**
   Runs a dfs test on graphs with no edges.
*/
void run_no_edges_graph_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t i, j, k;
  size_t num_vts;
  size_t start;
  void *pre = NULL, *post = NULL;
  graph_t g;
  adj_lst_t a;
  bern_arg_t b;
  b.p = C_PROB_ZERO;
  printf("Run a dfs test on graphs with no edges\n");
  for (i = log_start; i <= log_end; i++){
    num_vts = pow_two_perror(i);
    printf("\t\tvertices: %lu\n", TOLU(num_vts));
    for (j = 0; j < C_FN_COUNT; j++){
      /* no declared type after realloc; effective type is set by dfs */
      pre = realloc_perror(pre, num_vts, C_VT_SIZES[j]);
      post = realloc_perror(post, num_vts, C_VT_SIZES[j]);
      graph_base_init(&g, num_vts, C_VT_SIZES[j], 0);
      adj_lst_base_init(&a, &g);
      adj_lst_rand_dir(&a, C_WRITE[j], bern, &b);
      start =  RANDOM() % num_vts;
      dfs(&a,
	  start,
	  pre,
	  post,
	  C_READ[j],
	  C_WRITE[j],
	  C_AT[j],
	  C_CMP[j],
	  C_INCR[j]);
      for (k = 0; k < num_vts; k++){
        res *= (C_READ[j](ptr(post, k, C_VT_SIZES[j])) -
		C_READ[j](ptr(pre, k, C_VT_SIZES[j])) == 1);
      }
      printf("\t\t\t%s correctness:     ", C_VT_TYPES[j]);
      print_test_result(res);
      res = 1;
      adj_lst_free(&a); /* deallocates blocks with effective vertex type */
    }
  }
  free(pre);
  free(post);
  pre = NULL;
  post = NULL;
}

/**
   Run a dfs test on random directed graphs.
*/

void run_random_dir_graph_helper(size_t num_vts,
				 size_t vt_size,
				 const char *type_string,
				 size_t (*read_vt)(const void *),
				 void (*write_vt)(void *, size_t),
				 void *(*at_vt)(const void *, const void *),
				 int (*cmp_vt)(const void *, const void *),
				 void (*incr_vt)(void *),
				 int bern(void *),
				 bern_arg_t *b);

void run_random_dir_graph_test(size_t log_start, size_t log_end){
  size_t i, j;
  size_t num_vts;
  bern_arg_t b;
  printf("Run a dfs test on random directed graphs from %lu random "
	 "start vertices in each graph\n",  TOLU(C_ITER));
  for (i = 0; i < C_PROBS_COUNT; i++){
    b.p = C_PROBS[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (j = log_start; j <= log_end; j++){
      num_vts = pow_two_perror(j);
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f\n",
	     TOLU(num_vts), b.p * num_vts * (num_vts - 1));
      run_random_dir_graph_helper(num_vts,
				  C_VT_SIZES[0],
				  C_VT_TYPES[0],
				  C_READ[0],
				  C_WRITE[0],
				  C_AT[0],
				  C_CMP[0],
				  C_INCR[0],
				  bern,
				  &b);
      run_random_dir_graph_helper(num_vts,
				  C_VT_SIZES[1],
				  C_VT_TYPES[1],
				  C_READ[1],
				  C_WRITE[1],
				  C_AT[1],
				  C_CMP[1],
				  C_INCR[1],
				  bern,
				  &b);
    }
  }
}

void run_random_dir_graph_helper(size_t num_vts,
				 size_t vt_size,
				 const char *type_string,
				 size_t (*read_vt)(const void *),
				 void (*write_vt)(void *, size_t),
				 void *(*at_vt)(const void *, const void *),
				 int (*cmp_vt)(const void *, const void *),
				 void (*incr_vt)(void *),
				 int bern(void *),
				 bern_arg_t *b){
  size_t i;
  size_t *start = NULL;
  void *pre = NULL, *post = NULL;
  graph_t g;
  adj_lst_t a;
  clock_t t;
  /* no declared type after realloc; effective type is set by dfs */
  start = malloc_perror(C_ITER, sizeof(size_t));
  pre = malloc_perror(num_vts, vt_size);
  post = malloc_perror(num_vts, vt_size);
  graph_base_init(&g, num_vts, vt_size, 0);
  adj_lst_base_init(&a, &g);
  adj_lst_rand_dir(&a, write_vt, bern, b);
  for (i = 0; i < C_ITER; i++){
    start[i] =  RANDOM() % num_vts;
  }
  t = clock();
  for (i = 0; i < C_ITER; i++){
    dfs(&a, start[i], pre, post, read_vt, write_vt, at_vt, cmp_vt, incr_vt);
  }
  t = clock() - t;
  printf("\t\t\t%s ave runtime:     %.6f seconds\n",
	 type_string, (float)t / C_ITER / CLOCKS_PER_SEC);
  adj_lst_free(&a); /* deallocates blocks with effective vertex type */
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
int cmp_arr(const void *a,
	    const void *b,
	    size_t size,
	    size_t n,
	    int (*cmp)(const void *, const void *)){
  int res = 1;
  size_t i;
  const char *ap = a;
  const char *bp = b;
  for (i = 0; i < n; i++){
    res *= (cmp(ap, bp) == 0);
    ap += size;
    bp += size;
  }
  return res;
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
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
  if (args[0] > C_USHORT_BIT - 2 ||
      args[1] > C_USHORT_BIT - 2 ||
      args[2] > C_USHORT_BIT - 2 ||
      args[3] > C_USHORT_BIT - 2 ||
      args[4] > C_USHORT_BIT - 2 ||
      args[5] > C_USHORT_BIT - 2 ||
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
    run_graph_a_test();
    run_graph_b_test();
  }
  if (args[7]) run_max_edges_graph_test(args[0], args[1]);
  if (args[8]) run_no_edges_graph_test(args[2], args[3]);
  if (args[9]) run_random_dir_graph_test(args[4], args[5]);
  free(args);
  args = NULL;
  return 0;
}
