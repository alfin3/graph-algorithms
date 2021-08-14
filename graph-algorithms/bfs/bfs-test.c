/**
   bfs-test.c

   Tests of the BFS algorithm.

   The following command line arguments can be used to customize tests:
   bfs-test
     [0, ushort width - 1] : a
     [0, ushort width - 1] : b s.t. 2**a <= V <= 2**b for max edges test
     [0, ushort width - 1] : c
     [0, ushort width - 1] : d s.t. 2**c <= V <= 2**d for no edges test
     [0, ushort width - 1] : e
     [0, ushort width - 1] : f s.t. 2**e <= V <= 2**f for rand graph test
     [0, 1] : on/off for small graph tests
     [0, 1] : on/off for max edges test
     [0, 1] : on/off for no edges test
     [0, 1] : on/off for random graph test

   usage examples: 
   ./bfs-test
   ./bfs-test 10 14 10 14 10 14
   ./bfs-test 10 14 10 14 10 14 0 1 1 1

   bfs-test can be run with any subset of command line arguments in the
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
#include "bfs.h"
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
  "bfs-test \n"
  "[0, ushort width - 1] : a\n"
  "[0, ushort width - 1] : b s.t. 2**a <= V <= 2**b for max edges test\n"
  "[0, ushort width - 1] : c\n"
  "[0, ushort width - 1] : d s.t. 2**c <= V <= 2**d for no edges test\n"
  "[0, ushort width - 1] : e\n"
  "[0, ushort width - 1] : f s.t. 2**e <= V <= 2**f for rand graph test\n"
  "[0, 1] : on/off for small graph tests\n"
  "[0, 1] : on/off for max edges test\n"
  "[0, 1] : on/off for no edges test\n"
  "[0, 1] : on/off for random graph test\n";
const int C_ARGC_MAX = 11;
const size_t C_ARGS_DEF[10] = {0, 6, 0, 6, 0, 14, 1, 1, 1, 1};
const size_t C_USHORT_BIT = CHAR_BIT * sizeof(unsigned short);

/* first small graph test */
const size_t C_NUM_VTS_A = 5;
const size_t C_NUM_ES_A = 4;
const unsigned short C_USHORT_U_A[4] = {0, 0, 0, 1};
const unsigned short C_USHORT_V_A[4] = {1, 2, 3, 3};
const unsigned short C_USHORT_WTS_A[4] = {(unsigned short)-1, 1,
					  (unsigned short)-1, 2};
const unsigned short C_USHORT_DIR_DIST_A[25] =
  {0, 1, 1, 1, 0,
   0, 0, 0, 1, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0};
const unsigned short C_USHORT_DIR_PREV_A[25] =
  {0, 0, 0, 0, 5,
   5, 1, 5, 1, 5,
   5, 5, 2, 5, 5,
   5, 5, 5, 3, 5,
   5, 5, 5, 5, 4};
const unsigned short C_USHORT_UNDIR_DIST_A[25] =
  {0, 1, 1, 1, 0,
   1, 0, 2, 1, 0,
   1, 2, 0, 2, 0,
   1, 1, 2, 0, 0,
   0, 0, 0, 0, 0};
const unsigned short C_USHORT_UNDIR_PREV_A[25] =
  {0, 0, 0, 0, 5,
   1, 1, 0, 1, 5,
   2, 0, 2, 0, 5,
   3, 3, 0, 3, 5,
   5, 5, 5, 5, 4};

const unsigned long C_ULONG_U_A[4] = {0, 0, 0, 1};
const unsigned long C_ULONG_V_A[4] = {1, 2, 3, 3};
const unsigned long C_ULONG_WTS_A[4] = {(unsigned long)-1, 1,
					(unsigned long)-1, 2};
const unsigned long C_ULONG_DIR_DIST_A[25] =
  {0, 1, 1, 1, 0,
   0, 0, 0, 1, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0};
const unsigned long C_ULONG_DIR_PREV_A[25] =
  {0, 0, 0, 0, 5,
   5, 1, 5, 1, 5,
   5, 5, 2, 5, 5,
   5, 5, 5, 3, 5,
   5, 5, 5, 5, 4};
const unsigned long C_ULONG_UNDIR_DIST_A[25] =
  {0, 1, 1, 1, 0,
   1, 0, 2, 1, 0,
   1, 2, 0, 2, 0,
   1, 1, 2, 0, 0,
   0, 0, 0, 0, 0};
const unsigned long C_ULONG_UNDIR_PREV_A[25] =
  {0, 0, 0, 0, 5,
   1, 1, 0, 1, 5,
   2, 0, 2, 0, 5,
   3, 3, 0, 3, 5,
   5, 5, 5, 5, 4};

/* second small graph test */
const size_t C_NUM_VTS_B = 5;
const size_t C_NUM_ES_B = 4;
const unsigned short C_USHORT_U_B[4] = {0, 1, 2, 3};
const unsigned short C_USHORT_V_B[4] = {1, 2, 3, 4};
const unsigned short C_USHORT_WTS_B[4] = {(unsigned short)-1, 1,
					  (unsigned short)-1, 2};
const unsigned short C_USHORT_DIR_DIST_B[25] =
  {0, 1, 2, 3, 4,
   0, 0, 1, 2, 3,
   0, 0, 0, 1, 2,
   0, 0, 0, 0, 1,
   0, 0, 0, 0, 0};
const unsigned short C_USHORT_DIR_PREV_B[25] =
  {0, 0, 1, 2, 3,
   5, 1, 1, 2, 3,
   5, 5, 2, 2, 3,
   5, 5, 5, 3, 3,
   5, 5, 5, 5, 4};
const unsigned short C_USHORT_UNDIR_DIST_B[25] =
  {0, 1, 2, 3, 4,
   1, 0, 1, 2, 3,
   2, 1, 0, 1, 2,
   3, 2, 1, 0, 1,
   4, 3, 2, 1, 0};
const unsigned short C_USHORT_UNDIR_PREV_B[25] =
  {0, 0, 1, 2, 3,
   1, 1, 1, 2, 3,
   1, 2, 2, 2, 3,
   1, 2, 3, 3, 3,
   1, 2, 3, 4, 4};

const unsigned long C_ULONG_U_B[4] = {0, 1, 2, 3};
const unsigned long C_ULONG_V_B[4] = {1, 2, 3, 4};
const unsigned long C_ULONG_WTS_B[4] = {(unsigned long)-1, 1,
					(unsigned long)-1, 2};
const unsigned long C_ULONG_DIR_DIST_B[25] =
  {0, 1, 2, 3, 4,
   0, 0, 1, 2, 3,
   0, 0, 0, 1, 2,
   0, 0, 0, 0, 1,
   0, 0, 0, 0, 0};
const unsigned long C_ULONG_DIR_PREV_B[25] =
  {0, 0, 1, 2, 3,
   5, 1, 1, 2, 3,
   5, 5, 2, 2, 3,
   5, 5, 5, 3, 3,
   5, 5, 5, 5, 4};
const unsigned long C_ULONG_UNDIR_DIST_B[25] =
  {0, 1, 2, 3, 4,
   1, 0, 1, 2, 3,
   2, 1, 0, 1, 2,
   3, 2, 1, 0, 1,
   4, 3, 2, 1, 0};
const unsigned long C_ULONG_UNDIR_PREV_B[25] =
  {0, 0, 1, 2, 3,
   1, 1, 1, 2, 3,
   1, 2, 2, 2, 3,
   1, 2, 3, 3, 3,
   1, 2, 3, 4, 4};

/* random graph tests */
int cmp_ushort(const void *a, const void *b);
int cmp_uint(const void *a, const void *b);
int cmp_ulong(const void *a, const void *b);
int cmp_sz(const void *a, const void *b);

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
int (* const C_CMP[4])(const void *, const void *) ={
  cmp_ushort,
  cmp_uint,
  cmp_ulong,
  cmp_sz};
int (* const C_CMPAT[4])(const void *, const void *, const void *) ={
  bfs_cmpat_ushort,
  bfs_cmpat_uint,
  bfs_cmpat_ulong,
  bfs_cmpat_sz};
void (* const C_INCR[4])(void *) ={
  bfs_incr_ushort,
  bfs_incr_uint,
  bfs_incr_ulong,
  bfs_incr_sz};
const size_t C_VT_SIZES[4] = {
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t)};
const char *C_VT_TYPES[4] = {"ushort", "uint  ", "ulong ", "sz    "};
const size_t C_ITER = 10;
const size_t C_PROBS_COUNT = 5;
const double C_PROBS[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;

static void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

/**  
   Test bfs on small graphs.
*/

int cmp_ushort(const void *a, const void *b){
  return *(const unsigned short *)a != *(const unsigned short *)b;
}

int cmp_uint(const void *a, const void *b){
  return *(const unsigned int *)a != *(const unsigned int *)b;
}

int cmp_ulong(const void *a, const void *b){
  return *(const unsigned long *)a != *(const unsigned long *)b;
}

int cmp_sz(const void *a, const void *b){
  return  *(const size_t *)a != *(const size_t *)b;
}

/**
   Initialize small graphs.
*/

void ushort_none_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned short),
		  0,
		  graph_read_ushort,
		  graph_write_ushort);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned short *)C_USHORT_U_A;
  g->v = (unsigned short *)C_USHORT_V_A;
}

void ulong_none_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned long),
		  0,
		  graph_read_ulong,
		  graph_write_ulong);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned long *)C_ULONG_U_A;
  g->v = (unsigned long *)C_ULONG_V_A;
}

void ushort_ulong_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned short),
		  sizeof(unsigned long),
		  graph_read_ushort,
		  graph_write_ushort);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned short *)C_USHORT_U_A;
  g->v = (unsigned short *)C_USHORT_V_A;
  g->wts = (unsigned long *)C_ULONG_WTS_A;
}

void ulong_ushort_graph_a_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_A,
		  sizeof(unsigned long),
		  sizeof(unsigned short),
		  graph_read_ulong,
		  graph_write_ulong);
  g->num_es = C_NUM_ES_A;
  g->u = (unsigned long *)C_ULONG_U_A;
  g->v = (unsigned long *)C_ULONG_V_A;
  g->wts = (unsigned short *)C_USHORT_WTS_A;
}

void ushort_none_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned short),
		  0,
		  graph_read_ushort,
		  graph_write_ushort);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned short *)C_USHORT_U_B;
  g->v = (unsigned short *)C_USHORT_V_B;
}

void ulong_none_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned long),
		  0,
		  graph_read_ulong,
		  graph_write_ulong);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned long *)C_ULONG_U_B;
  g->v = (unsigned long *)C_ULONG_V_B;
}

void ushort_ulong_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned short),
		  sizeof(unsigned long),
		  graph_read_ushort,
		  graph_write_ushort);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned short *)C_USHORT_U_B;
  g->v = (unsigned short *)C_USHORT_V_B;
  g->wts = (unsigned long *)C_ULONG_WTS_B;
}

void ulong_ushort_graph_b_init(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS_B,
		  sizeof(unsigned long),
		  sizeof(unsigned short),
		  graph_read_ulong,
		  graph_write_ulong);
  g->num_es = C_NUM_ES_B;
  g->u = (unsigned long *)C_ULONG_U_B;
  g->v = (unsigned long *)C_ULONG_V_B;
  g->wts = (unsigned short *)C_USHORT_WTS_B;
}

/**
   Run bfs tests on small graphs.
*/

void small_graph_helper(const graph_t *g,
			const void *ret_dist,
			const void *ret_prev,
			void (*build)(adj_lst_t *, const graph_t *),
			int (*cmp)(const void *, const void *),
			int (*cmpat)(const void *,
				     const void *,
				     const void *),
			void (*incr)(void *),
			int *res);

void run_graph_a_test(){
  int res = 1;
  graph_t g;
  printf("Run a bfs test on the first small graph with ushort "
	 "vertices --> ");
  fflush(stdout);
  ushort_none_graph_a_init(&g);
  small_graph_helper(&g,
		     C_USHORT_DIR_DIST_A,
		     C_USHORT_DIR_PREV_A,
		     adj_lst_dir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_USHORT_UNDIR_DIST_A,
		     C_USHORT_UNDIR_PREV_A,
		     adj_lst_undir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  ushort_ulong_graph_a_init(&g);
  small_graph_helper(&g,
		     C_USHORT_DIR_DIST_A,
		     C_USHORT_DIR_PREV_A,
		     adj_lst_dir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_USHORT_UNDIR_DIST_A,
		     C_USHORT_UNDIR_PREV_A,
		     adj_lst_undir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  print_test_result(res);
  res = 1;
  printf("Run a bfs test on the first small graph with ulong "
	 "vertices --> ");
  ulong_none_graph_a_init(&g);
  small_graph_helper(&g,
		     C_ULONG_DIR_DIST_A,
		     C_ULONG_DIR_PREV_A,
		     adj_lst_dir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_ULONG_UNDIR_DIST_A,
		     C_ULONG_UNDIR_PREV_A,
		     adj_lst_undir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  ulong_ushort_graph_a_init(&g);
  small_graph_helper(&g,
		     C_ULONG_DIR_DIST_A,
		     C_ULONG_DIR_PREV_A,
		     adj_lst_dir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_ULONG_UNDIR_DIST_A,
		     C_ULONG_UNDIR_PREV_A,
		     adj_lst_undir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  print_test_result(res);
}

void run_graph_b_test(){
  int res = 1;
  graph_t g;
  printf("Run a bfs test on the second small graph with ushort "
	 "vertices --> ");
  ushort_none_graph_b_init(&g);
  small_graph_helper(&g,
		     C_USHORT_DIR_DIST_B,
		     C_USHORT_DIR_PREV_B,
		     adj_lst_dir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_USHORT_UNDIR_DIST_B,
		     C_USHORT_UNDIR_PREV_B,
		     adj_lst_undir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  ushort_ulong_graph_b_init(&g);
  small_graph_helper(&g,
		     C_USHORT_DIR_DIST_B,
		     C_USHORT_DIR_PREV_B,
		     adj_lst_dir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  small_graph_helper(&g,
		     C_USHORT_UNDIR_DIST_B,
		     C_USHORT_UNDIR_PREV_B,
		     adj_lst_undir_build,
		     cmp_ushort,
		     bfs_cmpat_ushort,
		     bfs_incr_ushort,
		     &res);
  print_test_result(res);
  res = 1;
  printf("Run a bfs test on the second small graph with ulong "
	 "vertices --> ");
  ulong_none_graph_b_init(&g);
  small_graph_helper(&g,
		     C_ULONG_DIR_DIST_B,
		     C_ULONG_DIR_PREV_B,
		     adj_lst_dir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_ULONG_UNDIR_DIST_B,
		     C_ULONG_UNDIR_PREV_B,
		     adj_lst_undir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  ulong_ushort_graph_b_init(&g);
  small_graph_helper(&g,
		     C_ULONG_DIR_DIST_B,
		     C_ULONG_DIR_PREV_B,
		     adj_lst_dir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
  small_graph_helper(&g,
		     C_ULONG_UNDIR_DIST_B,
		     C_ULONG_UNDIR_PREV_B,
		     adj_lst_undir_build,
		     cmp_ulong,
		     bfs_cmpat_ulong,
		     bfs_incr_ulong,
		     &res);
   print_test_result(res);
}

void small_graph_helper(const graph_t *g,
			const void *ret_dist,
			const void *ret_prev,
			void (*build)(adj_lst_t *, const graph_t *),
			int (*cmp)(const void *, const void *),
			int (*cmpat)(const void *,
				     const void *,
				     const void *),
			void (*incr)(void *),
			int *res){
  size_t i, j;
  size_t vt_offset = 0;
  void *dist = NULL, *prev = NULL;
  adj_lst_t a;
  adj_lst_base_init(&a, g);
  build(&a, g);
  dist = malloc_perror(a.num_vts, a.vt_size);
  prev = malloc_perror(a.num_vts, a.vt_size);
  for (i = 0; i < a.num_vts; i++){
    bfs(&a, i, dist, prev, cmpat, incr);
    for (j = 0; j < a.num_vts; j++){
      *res *= (cmp(ptr(prev, j , a.vt_size),
		   ptr(ret_prev, j + vt_offset, a.vt_size)) == 0);
      if (a.read_vt(ptr(prev, j, a.vt_size)) != a.num_vts){
        /* no trap representation if vertex reached */
        *res *= (cmp(ptr(dist, j, a.vt_size),
		     ptr(ret_dist, j + vt_offset, a.vt_size)) == 0);
      }
    }
    vt_offset += a.num_vts;
  }
  adj_lst_free(&a);
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}


/**  
   Test bfs on large graphs.
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

void run_max_edges_graph_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t i, j, k;
  size_t num_vts;
  size_t start;
  void *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = C_PROB_ONE;
  printf("Run a bfs test on graphs with n vertices, where "
	 "2**%lu <= n <= 2**%lu, and n(n - 1) edges\n",
	 TOLU(log_start), TOLU(log_end));
  for (i = log_start; i <= log_end; i++){
    num_vts = pow_two_perror(i);
    printf("\t\tvertices: %lu\n", TOLU(num_vts));
    for (j = 0; j < C_FN_COUNT; j++){
      /* no declared type after realloc; effective type is set by bfs */
      dist = realloc_perror(dist, num_vts, C_VT_SIZES[j]);
      prev = realloc_perror(prev, num_vts, C_VT_SIZES[j]);
      adj_lst_rand_dir(&a,
		       num_vts,
		       C_VT_SIZES[j],
		       C_READ[j],
		       C_WRITE[j],
		       bern,
		       &b);
      start =  RANDOM() % num_vts;
      bfs(&a, start, dist, prev, C_CMPAT[j], C_INCR[j]);
      for (k = 0; k < num_vts; k++){
	if (k == start){
	  res *= (C_READ[j](ptr(dist, k, C_VT_SIZES[j]))  == 0);
	}else{
	  res *= (C_READ[j](ptr(dist, k, C_VT_SIZES[j]))  == 1);
	}
	res *= (C_READ[j](ptr(prev, k, C_VT_SIZES[j]))  == start);
      }
      printf("\t\t\t%s correctness:     ", C_VT_TYPES[j]);
      print_test_result(res);
      res = 1;
      adj_lst_free(&a); /* deallocates blocks with effective vertex type */
    }
  }
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

/**
   Runs a bfs test on directed graphs with no edges.
*/
void run_no_edges_graph_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t i, j, k;
  size_t num_vts;
  size_t start;
  void *dist = NULL, *prev = NULL;
  bern_arg_t b;
  adj_lst_t a;
  b.p = C_PROB_ZERO;
  printf("Run a bfs test on graphs with n vertices, where "
	 "2**%lu <= n <= 2**%lu, and no edges\n",
	 TOLU(log_start), TOLU(log_end));
  for (i = log_start; i <= log_end; i++){
    num_vts = pow_two_perror(i);
    printf("\t\tvertices: %lu\n", TOLU(num_vts));
    for (j = 0; j < C_FN_COUNT; j++){
      /* no declared type after realloc; effective type is set by bfs */
      dist = realloc_perror(dist, num_vts, C_VT_SIZES[j]);
      prev = realloc_perror(prev, num_vts, C_VT_SIZES[j]);
      adj_lst_rand_dir(&a,
		       num_vts,
		       C_VT_SIZES[j],
		       C_READ[j],
		       C_WRITE[j],
		       bern,
		       &b);
      start =  RANDOM() % num_vts;
      bfs(&a, start, dist, prev, C_CMPAT[j], C_INCR[j]);
      for (k = 0; k < num_vts; k++){
	if (k == start){
	  res *= (C_READ[j](ptr(prev, k, C_VT_SIZES[j])) == start &&
		  C_READ[j](ptr(dist, k, C_VT_SIZES[j])) == 0);
        }else{
	  res *= (C_READ[j](ptr(prev, k, C_VT_SIZES[j])) == num_vts);
	}
      }
      printf("\t\t\t%s correctness:     ", C_VT_TYPES[j]);
      print_test_result(res);
      res = 1;
      adj_lst_free(&a); /* deallocates blocks with effective vertex type */
    }
  }
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

/**
   Run a bfs test on random directed graphs.
*/

void run_random_dir_graph_helper(size_t num_vts,
				 size_t vt_size,
				 const char *vt_type,
				 size_t (*read)(const void *),
				 void (*write)(void *, size_t),
				 int (*cmpat)(const void *,
					      const void *,
					      const void *),
				 void (*incr)(void *),
				 int bern(void *),
				 bern_arg_t *b);

void run_random_dir_graph_test(size_t log_start, size_t log_end){
  size_t i, j;
  size_t num_vts;
  bern_arg_t b;
  printf("Run a bfs test on random directed graphs from %lu random "
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
				  C_CMPAT[0],
				  C_INCR[0],
				  bern,
				  &b);
    }
  }
}

void run_random_dir_graph_helper(size_t num_vts,
				 size_t vt_size,
				 const char *vt_type,
				 size_t (*read)(const void *),
				 void (*write)(void *, size_t),
				 int (*cmpat)(const void *,
					      const void *,
					      const void *),
				 void (*incr)(void *),
				 int bern(void *),
				 bern_arg_t *b){
  size_t i;
  size_t *start = NULL;
  void *dist = NULL, *prev = NULL;
  adj_lst_t a;
  clock_t t;
  /* no declared type after realloc; effective type is set by bfs */
  start = malloc_perror(C_ITER, sizeof(size_t));
  dist = realloc_perror(dist, num_vts, vt_size);
  prev = realloc_perror(prev, num_vts, vt_size);
  adj_lst_rand_dir(&a, num_vts, vt_size, read, write, bern, b);
  for (i = 0; i < C_ITER; i++){
    start[i] =  RANDOM() % num_vts;
  }
  t = clock();
  for (i = 0; i < C_ITER; i++){
    bfs(&a, start[i], dist, prev, cmpat, incr);
  }
  t = clock() - t;
  printf("\t\t\t%s ave runtime:     %.6f seconds\n",
	 vt_type, (float)t / C_ITER / CLOCKS_PER_SEC);
  adj_lst_free(&a); /* deallocates blocks with effective vertex type */
  free(start);
  free(dist);
  free(prev);
  start = NULL;
  dist = NULL;
  prev = NULL;
}

/**
   Auxiliary functions.
*/

/**
   Computes a pointer to the ith element in the block of elements.
*/
static void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
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
  if (args[0] > C_USHORT_BIT - 1 ||
      args[1] > C_USHORT_BIT - 1 ||
      args[2] > C_USHORT_BIT - 1 ||
      args[3] > C_USHORT_BIT - 1 ||
      args[4] > C_USHORT_BIT - 1 ||
      args[5] > C_USHORT_BIT - 1 ||
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
