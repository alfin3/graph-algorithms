/**
   dijkstra-test.c

   Tests of Dijkstra's algorithm with a hash table parameter across
   i) default, division-based and multiplication-based hash tables, ii)
   vertex types, and iii) edge weight types.

   The following command line arguments can be used to customize tests:
   dijkstra-test:
   -  [0, ushort width) : n for 2**n vertices in the smallest graph
   -  [0, ushort width) : n for 2**n vertices in the largest graph
   -  [0, 1] : small graph test on/off
   -  [0, 1] : bfs comparison test on/off
   -  [0, 1] : test on random graphs with random weights on/off

   usage examples: 
   ./dijkstra-test
   ./dijkstra-test 10 12
   ./dijkstra-test 13 13 0 0 1

   dijkstra-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99. The tests require that:
   - size_t and clock_t are convertible to double,
   - size_t can represent values upto 65535 for default values, and
     upto USHRT_MAX (>= 65535) otherwise,
   - the widths of the unsigned integral types are less than 2040 and even.

   TODO: add portable size_t printing
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
  "dijkstra-test \n"
  "[0, ushort width) : n for 2**n vertices in smallest graph\n"
  "[0, ushort width) : n for 2**n vertices in largest graph\n"
  "[0, 1] : small graph test on/off\n"
  "[0, 1] : bfs comparison test on/off\n"
  "[0, 1] : random graphs with random weights test on/off\n";
const int C_ARGC_MAX = 6;
const size_t C_ARGS_DEF[5] = {6u, 9u, 1u, 1u, 1u};

/* hash table load factor upper bounds */
const size_t C_ALPHA_N_DIVCHN = 1u;
const size_t C_LOG_ALPHA_D_DIVCHN = 0u;
const size_t C_ALPHA_N_MULOA = 13107u;
const size_t C_LOG_ALPHA_D_MULOA = 15u;

/* small graph tests */
const size_t C_NUM_VTS = 5u;
const size_t C_NUM_ES = 4u;

const unsigned short C_USHORT_U[4] = {0u, 0u, 0u, 1u};
const unsigned short C_USHORT_V[4] = {1u, 2u, 3u, 3u};
const unsigned short C_USHORT_WTS[4] = {4u, 3u, 2u, 1u};

const unsigned int C_UINT_U[4] = {0u, 0u, 0u, 1u};
const unsigned int C_UINT_V[4] = {1u, 2u, 3u, 3u};
const unsigned int C_UINT_WTS[4] = {4u, 3u, 2u, 1u};

const unsigned long C_ULONG_U[4] = {0u, 0u, 0u, 1u};
const unsigned long C_ULONG_V[4] = {1u, 2u, 3u, 3u};
const unsigned long C_ULONG_WTS[4] = {4u, 3u, 2u, 1u};

const size_t C_SZ_U[4] = {0u, 0u, 0u, 1u};
const size_t C_SZ_V[4] = {1u, 2u, 3u, 3u};
const size_t C_SZ_WTS[4] = {4u, 3u, 2u, 1u};

const double C_DOUBLE_WTS[4] = {4.0, 3.0, 2.0, 1.0};

/* small graph initialization ops */
void init_ushort_ushort(graph_t *g);
void init_ushort_uint(graph_t *g);
void init_ushort_ulong(graph_t *g);
void init_ushort_sz(graph_t *g);
void init_ushort_double(graph_t *g);

void init_uint_ushort(graph_t *g);
void init_uint_uint(graph_t *g);
void init_uint_ulong(graph_t *g);
void init_uint_sz(graph_t *g);
void init_uint_double(graph_t *g);

void init_ulong_ushort(graph_t *g);
void init_ulong_uint(graph_t *g);
void init_ulong_ulong(graph_t *g);
void init_ulong_sz(graph_t *g);
void init_ulong_double(graph_t *g);

void init_sz_ushort(graph_t *g);
void init_sz_uint(graph_t *g);
void init_sz_ulong(graph_t *g);
void init_sz_sz(graph_t *g);
void init_sz_double(graph_t *g);

const size_t C_FN_VT_COUNT = 4u;
const size_t C_FN_WT_COUNT = 5u;
const size_t C_FN_INTEGRAL_WT_COUNT = 4u;
void (* const C_INIT_GRAPH[4][5])(graph_t *) ={
  {init_ushort_ushort,
   init_ushort_uint,
   init_ushort_ulong,
   init_ushort_sz,
   init_ushort_double},
  {init_uint_ushort,
   init_uint_uint,
   init_uint_ulong,
   init_uint_sz,
   init_uint_double},
  {init_ulong_ushort,
   init_ulong_uint,
   init_ulong_ulong,
   init_ulong_sz,
   init_ulong_double},
  {init_sz_ushort,
   init_sz_uint,
   init_sz_ulong,
   init_sz_sz,
   init_sz_double}};

/* vertex ops */
size_t (* const C_READ_VT[4])(const void *) ={
  graph_read_ushort,
  graph_read_uint,
  graph_read_ulong,
  graph_read_sz};
void (* const C_WRITE_VT[4])(void *, size_t) ={
  graph_write_ushort,
  graph_write_uint,
  graph_write_ulong,
  graph_write_sz};
void *(* const C_AT_VT[4])(const void *, const void *) ={
  graph_at_ushort,
  graph_at_uint,
  graph_at_ulong,
  graph_at_sz};
int (* const C_CMP_VT[4])(const void *, const void *) ={
  graph_cmpeq_ushort,
  graph_cmpeq_uint,
  graph_cmpeq_ulong,
  graph_cmpeq_sz};
void (* const C_INCR_VT[4])(void *) ={
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

/* weight and print ops */

int cmp_double(const void *a, const void *b);
void add_double(void *s, const void *a, const void *b);

int (* const C_CMP_WT[5])(const void *, const void *) ={
  graph_cmp_ushort,
  graph_cmp_uint,
  graph_cmp_ulong,
  graph_cmp_sz,
  cmp_double};
void (* const C_ADD_WT[5])(void *, const void *, const void *) ={
  graph_add_ushort,
  graph_add_uint,
  graph_add_ulong,
  graph_add_sz,
  add_double};
const size_t C_WT_SIZES[5] = {
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t),
  sizeof(double)};
const char *C_WT_TYPES[5] = {"ushort",
			     "uint  ",
			     "ulong ",
			     "sz    ",
			     "double"};

/* random graph tests */
/* C89 (draft): USHRT_MAX >= 65535, UINT_MAX >= 65535, 
   ULONG_MAX >= 4294967295, RAND_MAX >= 32767 */
const size_t C_RANDOM_BIT = 15u;
const unsigned int C_RANDOM_MASK = 32767u;

const size_t C_USHORT_BIT = UINT_WIDTH_FROM_MAX(USHRT_MAX);
const size_t C_USHORT_BIT_MOD = UINT_WIDTH_FROM_MAX(USHRT_MAX) / 15u;
const size_t C_USHORT_HALF_BIT = UINT_WIDTH_FROM_MAX(USHRT_MAX) / 2u;
const unsigned short C_USHORT_MAX = USHRT_MAX;
const unsigned short C_USHORT_LOW_MASK =
  ((unsigned short)-1 >> (UINT_WIDTH_FROM_MAX(USHRT_MAX) / 2u));

const size_t C_UINT_BIT = UINT_WIDTH_FROM_MAX(UINT_MAX);
const size_t C_UINT_BIT_MOD = UINT_WIDTH_FROM_MAX(UINT_MAX) / 15u;
const size_t C_UINT_HALF_BIT = UINT_WIDTH_FROM_MAX(UINT_MAX) / 2u;
const unsigned int C_UINT_MAX = UINT_MAX;
const unsigned int C_UINT_LOW_MASK =
  ((unsigned int)-1 >> (UINT_WIDTH_FROM_MAX(UINT_MAX) / 2u));

const size_t C_ULONG_BIT = UINT_WIDTH_FROM_MAX(ULONG_MAX);
const size_t C_ULONG_BIT_MOD = UINT_WIDTH_FROM_MAX(ULONG_MAX) / 15u;
const size_t C_ULONG_HALF_BIT = UINT_WIDTH_FROM_MAX(ULONG_MAX) / 2u;
const unsigned long C_ULONG_MAX = ULONG_MAX;
const unsigned long C_ULONG_LOW_MASK =
  ((unsigned long)-1 >> (UINT_WIDTH_FROM_MAX(ULONG_MAX) / 2u));

const size_t C_SZ_BIT = UINT_WIDTH_FROM_MAX((size_t)-1);
const size_t C_SZ_BIT_MOD = UINT_WIDTH_FROM_MAX((size_t)-1) / 15u;
const size_t C_SZ_HALF_BIT = UINT_WIDTH_FROM_MAX((size_t)-1) / 2u;
const size_t C_SZ_MAX = (size_t)-1;
const size_t C_SZ_LOW_MASK =
  ((size_t)-1 >> (UINT_WIDTH_FROM_MAX((size_t)-1) / 2u));

const size_t C_ITER = 10u;
const size_t C_PROBS_COUNT = 7u;
const double C_PROBS[7] = {1.000000, 0.250000, 0.062500,
			   0.015625, 0.003906, 0.000977,
			   0.000000};

/* random number generation and random graph construction */
unsigned short random_ushort();
unsigned int random_uint();
unsigned long random_ulong();
size_t random_sz();

unsigned short mul_high_ushort(unsigned short a, unsigned short b);
unsigned int mul_high_uint(unsigned int a, unsigned int b);
unsigned long mul_high_ulong(unsigned long a, unsigned long b);
size_t mul_high_sz(size_t a, size_t b);

void add_dir_ushort_edge(adj_lst_t *a,
			 size_t u,
			 size_t v,
			 const void *wt_l,
			 const void *wt_h,
			 void (*write_vt)(void *, size_t),
			 int (*bern)(void *),
			 void *arg);
void add_dir_uint_edge(adj_lst_t *a,
		       size_t u,
		       size_t v,
		       const void *wt_l,
		       const void *wt_h,
		       void (*write_vt)(void *, size_t),
		       int (*bern)(void *),
		       void *arg);
void add_dir_ulong_edge(adj_lst_t *a,
			size_t u,
			size_t v,
			const void *wt_l,
			const void *wt_h,
			void (*write_vt)(void *, size_t),
			int (*bern)(void *),
			void *arg);
void add_dir_sz_edge(adj_lst_t *a,
		     size_t u,
		     size_t v,
		     const void *wt_l,
		     const void *wt_h,
		     void (*write_vt)(void *, size_t),
		     int (*bern)(void *),
		     void *arg);
void add_dir_double_edge(adj_lst_t *a,
			 size_t u,
			 size_t v,
			 const void *wt_l,
			 const void *wt_h,
			 void (*write_vt)(void *, size_t),
			 int (*bern)(void *),
			 void *arg);

void (* const C_ADD_DIR_EDGE[5])(adj_lst_t *,
				 size_t,
				 size_t,
				 const void *,
				 const void *,
				 void (*)(void *, size_t),
				 int (*)(void *),
				 void *) ={
  add_dir_ushort_edge,
  add_dir_uint_edge,
  add_dir_ulong_edge,
  add_dir_sz_edge,
  add_dir_double_edge};

/* value initiliazation and printing */
void set_zero_ushort(void *a);
void set_zero_uint(void *a);
void set_zero_ulong(void *a);
void set_zero_sz(void *a);
void set_zero_double(void *a);

void set_one_ushort(void *a);
void set_one_uint(void *a);
void set_one_ulong(void *a);
void set_one_sz(void *a);
void set_one_double(void *a);

void set_test_max_ushort(void *a, size_t num_vts);
void set_test_max_uint(void *a, size_t num_vts);
void set_test_max_ulong(void *a, size_t num_vts);
void set_test_max_sz(void *a, size_t num_vts);
void set_test_max_double(void *a, size_t num_vts);

void sum_dist_ushort(void *dist_sum,
		     void *wt_sum,
		     size_t *num_dist_wraps,
		     size_t *num_wt_wraps,
		     size_t *num_paths,
		     size_t num_vts,
		     size_t vt_size,
		     const void *prev,
		     const void *dist,
		     size_t (*read_vt)(const void *));
void sum_dist_uint(void *dist_sum,
		   void *wt_sum,
		   size_t *num_dist_wraps,
		   size_t *num_wt_wraps,
		   size_t *num_paths,
		   size_t num_vts,
		   size_t vt_size,
		   const void *prev,
		   const void *dist,
		   size_t (*read_vt)(const void *));
void sum_dist_ulong(void *dist_sum,
		    void *wt_sum,
		    size_t *num_dist_wraps,
		    size_t *num_wt_wraps,
		    size_t *num_paths,
		    size_t num_vts,
		    size_t vt_size,
		    const void *prev,
		    const void *dist,
		    size_t (*read_vt)(const void *));
void sum_dist_sz(void *dist_sum,
		 void *wt_sum,
		 size_t *num_dist_wraps,
		 size_t *num_wt_wraps,
		 size_t *num_paths,
		 size_t num_vts,
		 size_t vt_size,
		 const void *prev,
		 const void *dist,
		 size_t (*read_vt)(const void *));
void sum_dist_double(void *dist_sum,
		     void *wt_sum,
		     size_t *num_dist_wraps,
		     size_t *num_wt_wraps,
		     size_t *num_paths,
		     size_t num_vts,
		     size_t vt_size,
		     const void *prev,
		     const void *dist,
		     size_t (*read_vt)(const void *));

void print_ushort(const void *a);
void print_uint(const void *a);
void print_ulong(const void *a);
void print_sz(const void *a);
void print_double(const void *a);

void (* const C_SET_ZERO[5])(void *) ={
  set_zero_ushort,
  set_zero_uint,
  set_zero_ulong,
  set_zero_sz,
  set_zero_double};
void (* const C_SET_ONE[5])(void *) ={
  set_one_ushort,
  set_one_uint,
  set_one_ulong,
  set_one_sz,
  set_one_double};
void (* const C_SET_TEST_MAX[5])(void *, size_t) ={
  set_test_max_ushort,
  set_test_max_uint,
  set_test_max_ulong,
  set_test_max_sz,
  set_test_max_double};
void (* const C_SUM_DIST[5])(void *,
			     void *,
			     size_t *,
			     size_t *,
			     size_t *,
			     size_t,
			     size_t,
			     const void *,
			     const void *,
			     size_t (*)(const void *)) ={
  sum_dist_ushort,
  sum_dist_uint,
  sum_dist_ulong,
  sum_dist_sz,
  sum_dist_double};
void (* const C_PRINT[5])(const void *) ={
  print_ushort,
  print_uint,
  print_ulong,
  print_sz,
  print_double};

/* additional operations */
void *ptr(const void *block, size_t i, size_t size);
void print_arr(const void *arr,
	       size_t size,
	       size_t n,
	       void (*print_elt)(const void *));
void print_prev(const adj_lst_t *a,
		const void *prev,
		void (*print_vt)(const void *));
void print_dist(const adj_lst_t *a,
		const void *dist,
		const void *prev,
		const void *wt_zero,
		size_t (*read_vt)(const void *),
		void (*print_wt)(const void *));
void print_adj_lst(const adj_lst_t *a,
		   void (*print_vt)(const void *),
		   void (*print_wt)(const void *));
void print_test_result(int res);

/**
   Initialize small graphs across vertex and weight types.
*/

void init_ushort_ushort(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_ushort_uint(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_ushort_ulong(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_ushort_sz(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_ushort_double(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_uint_ushort(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned int),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_uint_uint(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned int),
		  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_uint_ulong(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned int),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_uint_sz(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned int),
		  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_uint_double(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned int),
		  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_ulong_ushort(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_ulong_uint(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_ulong_ulong(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_ulong_sz(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_ulong_double(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_sz_ushort(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(size_t),
		  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_sz_uint(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(size_t),
		  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_sz_ulong(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(size_t),
		  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_sz_sz(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(size_t),
		  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_sz_double(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(size_t),
		  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

/**
   Run a test on small graphs across vertex and weight types.
*/

int cmp_double(const void *a, const void *b){
  return ((*(const double *)a > *(const double *)b) -
	  (*(const double *)a < *(const double *)b));
}

void add_double(void *s, const void *a, const void *b){
  *(double *)s = *(const double *)a + *(const double *)b;
}

void small_graph_helper(void (*build)(adj_lst_t *,
				      const graph_t *,
				      size_t (*)(const void *))){
  size_t i, j, k, l;
  void *wt_zero = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  void *prev_def = NULL, *prev_divchn = NULL, *prev_muloa = NULL;
  graph_t g;
  adj_lst_t a;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  dijkstra_ht_t daht_divchn, daht_muloa;
  daht_divchn.ht = &ht_divchn;
  daht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  daht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  daht_divchn.init = ht_divchn_init_helper;
  daht_divchn.align = ht_divchn_align_helper;
  daht_divchn.insert = ht_divchn_insert_helper;
  daht_divchn.search = ht_divchn_search_helper;
  daht_divchn.remove = ht_divchn_remove_helper;
  daht_divchn.free = ht_divchn_free_helper;
  daht_muloa.ht = &ht_muloa;
  daht_muloa.alpha_n = C_ALPHA_N_MULOA;
  daht_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  daht_muloa.init = ht_muloa_init_helper;
  daht_muloa.align = ht_muloa_align_helper;
  daht_muloa.insert = ht_muloa_insert_helper;
  daht_muloa.search = ht_muloa_search_helper;
  daht_muloa.remove = ht_muloa_remove_helper;
  daht_muloa.free = ht_muloa_free_helper;
  for (i = 0; i < C_NUM_VTS; i++){
    printf("\tstart vertex: %lu\n", TOLU(i));
    for (j = 0; j < C_FN_VT_COUNT; j++){
      printf("\t\tvertex type: %s\n", C_VT_TYPES[j]);
      for (k = 0; k < C_FN_WT_COUNT; k++){
	printf("\t\t\tweight type: %s\n", C_WT_TYPES[k]);
	C_INIT_GRAPH[j][k](&g);
	adj_lst_base_init(&a, &g);
	build(&a, &g, C_READ_VT[j]);
	/* no declared type after realloc; new eff. type to be acquired */
	wt_zero = realloc_perror(wt_zero, 1, a.wt_size);
	prev_def = realloc_perror(prev_def, a.num_vts, a.vt_size);
	prev_divchn = realloc_perror(prev_divchn, a.num_vts, a.vt_size);
	prev_muloa = realloc_perror(prev_muloa, a.num_vts, a.vt_size);
	dist_def = realloc_perror(dist_def, a.num_vts, a.wt_size);
	dist_divchn = realloc_perror(dist_divchn, a.num_vts, a.wt_size);
	dist_muloa = realloc_perror(dist_muloa, a.num_vts, a.wt_size);
	C_SET_ZERO[k](wt_zero);
	for (l = 0; l < a.num_vts; l++){
	  /* avoid trap representations in tests */
	  C_SET_ZERO[k](ptr(dist_def, l, a.wt_size));
	  C_SET_ZERO[k](ptr(dist_divchn, l, a.wt_size));
	  C_SET_ZERO[k](ptr(dist_muloa, l, a.wt_size));
	} 
	dijkstra(&a, i, dist_def, prev_def, wt_zero, NULL,
		 C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		 C_CMP_WT[k], C_ADD_WT[k]);
	dijkstra(&a, i, dist_divchn, prev_divchn, wt_zero, &daht_divchn,
		 C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		 C_CMP_WT[k], C_ADD_WT[k]);
	dijkstra(&a, i, dist_muloa, prev_muloa, wt_zero, &daht_muloa,
		 C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		 C_CMP_WT[k], C_ADD_WT[k]);
	adj_lst_free(&a);
	printf("\t\t\t\tdefault dist: ");
	print_dist(&a, dist_def, prev_def, wt_zero,
		   C_READ_VT[j], C_PRINT[k]);
	printf("\n");
	printf("\t\t\t\tdivchn dist:  ");
	print_dist(&a, dist_divchn, prev_divchn, wt_zero,
		   C_READ_VT[j], C_PRINT[k]);
	printf("\n");
	printf("\t\t\t\tmuloa dist:   ");
	print_dist(&a, dist_muloa, prev_muloa, wt_zero,
		   C_READ_VT[j], C_PRINT[k]);
	printf("\n");
	printf("\t\t\t\tdefault prev: ");
	print_prev(&a, prev_def, C_PRINT[j]);
	printf("\n");
	printf("\t\t\t\tdivchn prev:  ");
	print_prev(&a, prev_divchn, C_PRINT[j]);
	printf("\n");
	printf("\t\t\t\tmuloa prev:   ");
	print_prev(&a, prev_muloa, C_PRINT[j]);
	printf("\n");
      }
    }
  }
  printf("\n");
  free(wt_zero);
  free(dist_def);
  free(dist_divchn);
  free(dist_muloa);
  free(prev_def);
  free(prev_divchn);
  free(prev_muloa);
  wt_zero = NULL;
  dist_def = NULL;
  dist_divchn = NULL;
  dist_muloa = NULL;
  prev_def = NULL;
  prev_divchn = NULL;
  prev_muloa = NULL;
}
  
void run_small_graph_test(){
  printf("Run a dijkstra test on a directed graph across vertex and weight"
	 " types, with a\n"
	 "i) default hash table (index array)\n"
	 "ii) ht_divchn_t hash table\n"
	 "iii) ht_muloa_t hash table\n\n");
  small_graph_helper(adj_lst_dir_build);
  printf("Run a dijkstra test on an undirected graph across vertex and"
	 " weight types, with a\n"
	 "i) default hash table (index array)\n"
	 "ii) ht_divchn_t hash table\n"
	 "iii) ht_muloa_t hash table\n\n");
  small_graph_helper(adj_lst_undir_build);
}

/** 
    Construct adjacency lists of random directed graphs with random 
    weights across vertex and weight types.

    A function with the add_dir_ prefix adds a (u, v) edge to an adjacency
    list of a weighted graph, preinitialized with at least adj_lst_base_init
    and with the number of vertices n greater or equal to 1. An edge (u, v),
    where u < n nad v < n, is added with the Bernoulli distribution according
    to the bern and arg parameter values. wt_l and wt_h point to wt_size
    blocks with values l and h of the type used to represent weights in the
    adjacency list, and l must be non-negative and less or equal to h.
    If (u, v) is added, a random weight in [l, h) is chosen for the edge. 

    adj_lst_rand_dir_wts builds a random adjacency list with one of the 
    above functions as a parameter value. g points to a graph preinitialized
    with graph_base_init with at least one vertex. a points to a
    preallocated block of size sizeof(adj_lst_t).
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

void add_dir_ushort_edge(adj_lst_t *a,
			 size_t u,
			 size_t v,
			 const void *wt_l,
			 const void *wt_h,
			 void (*write_vt)(void *, size_t),
			 int (*bern)(void *),
			 void *arg){
  unsigned short rand_val =
    *(unsigned short *)wt_l +
     mul_high_ushort(random_ushort(),
		     (*(unsigned short *)wt_h - *(unsigned short *)wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_dir_uint_edge(adj_lst_t *a,
		       size_t u,
		       size_t v,
		       const void *wt_l,
		       const void *wt_h,
		       void (*write_vt)(void *, size_t),
		       int (*bern)(void *),
		       void *arg){
  unsigned int rand_val =
    *(unsigned int *)wt_l +
    mul_high_uint(random_uint(),
		  (*(unsigned int *)wt_h - *(unsigned int *)wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_dir_ulong_edge(adj_lst_t *a,
			size_t u,
			size_t v,
			const void *wt_l,
			const void *wt_h,
			void (*write_vt)(void *, size_t),
			int (*bern)(void *),
			void *arg){
  unsigned long rand_val =
    *(unsigned long *)wt_l +
    mul_high_ulong(random_ulong(),
		   (*(unsigned long *)wt_h - *(unsigned long *)wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_dir_sz_edge(adj_lst_t *a,
		     size_t u,
		     size_t v,
		     const void *wt_l,
		     const void *wt_h,
		     void (*write_vt)(void *, size_t),
		     int (*bern)(void *),
		     void *arg){
  size_t rand_val =
    *(size_t *)wt_l +
    mul_high_sz(random_sz(),
		(*(size_t *)wt_h - *(size_t *)wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_dir_double_edge(adj_lst_t *a,
			 size_t u,
			 size_t v,
			 const void *wt_l,
			 const void *wt_h,
			 void (*write_vt)(void *, size_t),
			 int (*bern)(void *),
			 void *arg){
  double rand_val =
    *(double *)wt_l +
     DRAND() * (*(double *)wt_h - *(double *)wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void adj_lst_rand_dir_wts(const graph_t *g,
			  adj_lst_t *a,
			  const void *wt_l,
			  const void *wt_h,
			  void (*write_vt)(void *, size_t),
			  int (*bern)(void *),
			  void *arg,
			  void (*add_dir_edge)(adj_lst_t *,
					       size_t,
					       size_t,
					       const void *,
					       const void *,
					       void (*)(void *, size_t),
					       int (*)(void *),
					       void *)){
  size_t i, j;
  adj_lst_base_init(a, g);
  for (i = 0; i < a->num_vts - 1; i++){
    for (j = i + 1; j < a->num_vts; j++){
      add_dir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
      add_dir_edge(a, j, i, wt_l, wt_h, write_vt, bern, arg);
    }
  }
}

/**
   Run a test of distance equivalence of bfs and dijkstra on random
   directed graphs with the same weight for all edges, across integral
   type edge weights, vertex types, as well as default, division-based
   and multiplication-based hash tables.
*/

void run_bfs_comparison_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t p, i, j, k, l;
  size_t num_vts;
  size_t vt_size;
  size_t wt_size;
  size_t *rand_start = NULL;
  void *wt_l = NULL;
  void *wt_zero = NULL;
  void *dist_elt_bfs = NULL;
  void *dist_bfs = NULL;
  void *prev_bfs = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  void *prev_def = NULL, *prev_divchn = NULL, *prev_muloa = NULL;
  graph_t g;
  adj_lst_t a;
  bern_arg_t b;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  dijkstra_ht_t daht_divchn, daht_muloa;
  clock_t t_bfs, t_def, t_divchn, t_muloa;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  daht_divchn.ht = &ht_divchn;
  daht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  daht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  daht_divchn.init = ht_divchn_init_helper;
  daht_divchn.align = ht_divchn_align_helper;
  daht_divchn.insert = ht_divchn_insert_helper;
  daht_divchn.search = ht_divchn_search_helper;
  daht_divchn.remove = ht_divchn_remove_helper;
  daht_divchn.free = ht_divchn_free_helper;
  daht_muloa.ht = &ht_muloa;
  daht_muloa.alpha_n = C_ALPHA_N_MULOA;
  daht_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  daht_muloa.init = ht_muloa_init_helper;
  daht_muloa.align = ht_muloa_align_helper;
  daht_muloa.insert = ht_muloa_insert_helper;
  daht_muloa.search = ht_muloa_search_helper;
  daht_muloa.remove = ht_muloa_remove_helper;
  daht_muloa.free = ht_muloa_free_helper;
  printf("Run a bfs and dijkstra test on random directed graphs across "
	 "vertex types and integer weight types;\nthe runtime is averaged"
	 " over %lu runs from random start vertices\n", TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = log_start; i <= log_end; i++){
      num_vts = pow_two_perror(i); /* 0 < n */
      printf("\t\t# vertices: %lu\n", TOLU(num_vts));
      for (j = 0; j < C_FN_VT_COUNT; j++){
	for (k = 0; k < C_FN_INTEGRAL_WT_COUNT; k++){
	  vt_size =  C_VT_SIZES[j];
	  wt_size =  C_WT_SIZES[k];
	  /* no declared type after realloc; new eff. type to be acquired */
	  wt_l = realloc_perror(wt_l, 3, wt_size);
	  wt_zero = ptr(wt_l, 1, wt_size);
	  dist_elt_bfs = ptr(wt_l, 2, wt_size);
	  prev_bfs = realloc_perror(prev_bfs, num_vts, vt_size);
	  prev_def = realloc_perror(prev_def, num_vts, vt_size);
	  prev_divchn = realloc_perror(prev_divchn, num_vts, vt_size);
	  prev_muloa = realloc_perror(prev_muloa, num_vts, vt_size);
	  dist_bfs = realloc_perror(dist_bfs, num_vts, vt_size);/* vt_size */
	  dist_def = realloc_perror(dist_def, num_vts, wt_size);
	  dist_divchn = realloc_perror(dist_divchn, num_vts, wt_size);
	  dist_muloa = realloc_perror(dist_muloa, num_vts, wt_size);
	  C_SET_ONE[k](wt_l);
	  C_SET_ZERO[k](wt_zero);
	  for (l = 0; l < num_vts; l++){
	    /* avoid trap representations in tests */
	    C_SET_ZERO[j](ptr(dist_bfs, l, vt_size));
	    C_SET_ZERO[k](ptr(dist_def, l, wt_size));
	    C_SET_ZERO[k](ptr(dist_divchn, l, wt_size));
	    C_SET_ZERO[k](ptr(dist_muloa, l, wt_size));
	  } 
	  graph_base_init(&g, num_vts, vt_size, wt_size);
	  adj_lst_rand_dir_wts(&g, &a, wt_l, wt_l,
			       C_WRITE_VT[j], bern, &b, C_ADD_DIR_EDGE[k]);
	  for (l = 0; l < C_ITER; l++){
	    rand_start[l] = mul_high_sz(random_sz(), num_vts);
	  }
	  t_bfs = clock();
	  for (l = 0; l < C_ITER; l++){
	    bfs(&a, rand_start[l], dist_bfs, prev_bfs,
		C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		C_INCR_VT[j]);
	  }
	  t_bfs = clock() - t_bfs;
	  t_def = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_def, prev_def, wt_zero, NULL,
		     C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		     C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_def = clock() - t_def;
	  t_divchn = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_divchn, prev_divchn, wt_zero,
		     &daht_divchn, C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j],
		     C_CMP_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_divchn = clock() - t_divchn;
	  t_muloa = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_muloa, prev_muloa, wt_zero,
		     &daht_muloa, C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j],
		     C_CMP_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_muloa = clock() - t_muloa;
	  for (l = 0; l < num_vts; l++){
	    if (C_READ_VT[j](ptr(prev_bfs, l, vt_size)) == num_vts){
	      res *=
		(C_READ_VT[j](ptr(prev_def, l, vt_size)) == num_vts &&
		 C_READ_VT[j](ptr(prev_divchn, l, vt_size)) == num_vts &&
		 C_READ_VT[j](ptr(prev_muloa, l, vt_size)) == num_vts);
	    }else{
	      /* prev may differ from bfs, but distances are equal */
	      res *= (C_READ_VT[j](ptr(prev_def, l, vt_size)) < num_vts &&
		      C_READ_VT[j](ptr(prev_def, l, vt_size)) ==
		      C_READ_VT[j](ptr(prev_divchn, l, vt_size)) &&
		      C_READ_VT[j](ptr(prev_divchn, l, vt_size)) ==
		      C_READ_VT[j](ptr(prev_muloa, l, vt_size)));
	      C_WRITE_VT[k](dist_elt_bfs,
			    C_READ_VT[j](ptr(dist_bfs, l, vt_size)));
	      res *=
		(C_CMP_WT[k](ptr(dist_def, l, wt_size),
			     ptr(dist_divchn, l, wt_size)) == 0 &&
		 C_CMP_WT[k](ptr(dist_divchn, l, wt_size),
			     ptr(dist_muloa, l, wt_size)) == 0 &&
		 C_CMP_WT[k](ptr(dist_muloa, l, wt_size),
			     dist_elt_bfs) == 0);
	    }
	  }
	  printf("\t\t\t# edges: %lu\n", TOLU(a.num_es));
	  printf("\t\t\t\t%s %s bfs:                     %.8f seconds\n"
		 "\t\t\t\t%s %s dijkstra default ht:     %.8f seconds\n"
		 "\t\t\t\t%s %s dijkstra ht_divchn:      %.8f seconds\n"
		 "\t\t\t\t%s %s dijkstra ht_muloa:       %.8f seconds\n",
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_bfs / C_ITER / CLOCKS_PER_SEC,
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_def / C_ITER / CLOCKS_PER_SEC,
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_divchn / C_ITER / CLOCKS_PER_SEC,
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_muloa / C_ITER / CLOCKS_PER_SEC);
	  printf("\t\t\t\t%s %s correctness:             ",
		 C_VT_TYPES[j], C_WT_TYPES[k]);
	  print_test_result(res);
	  printf("\n");
	  res = 1;
	  adj_lst_free(&a);
	}
      }
    }
  }
  free(rand_start);
  free(wt_l);
  free(dist_bfs);
  free(prev_bfs);
  free(dist_def);
  free(dist_divchn);
  free(dist_muloa);
  free(prev_def);
  free(prev_divchn);
  free(prev_muloa);
  rand_start = NULL;
  wt_l = NULL;
  wt_zero = NULL;
  dist_elt_bfs = NULL;
  dist_bfs = NULL;
  prev_bfs = NULL;
  dist_def = NULL;
  dist_divchn = NULL;
  dist_muloa = NULL;
  prev_def = NULL;
  prev_divchn = NULL;
  prev_muloa = NULL;
}

/**
   Run a test on random directed graphs with random weights, across edge
   weight types, vertex types, as well as default, division-based
   and multiplication-based hash tables.
*/

void run_rand_test(size_t log_start, size_t log_end){
  int res = 1;
  size_t p, i, j, k, l;
  size_t num_vts;
  size_t vt_size;
  size_t wt_size;
  size_t num_dwraps_def, num_dwraps_divchn, num_dwraps_muloa;
  size_t num_wwraps_def, num_wwraps_divchn, num_wwraps_muloa;
  size_t num_paths_def, num_paths_divchn, num_paths_muloa;
  size_t *rand_start = NULL;
  void *wt_l = NULL, *wt_h = NULL;
  void *wt_zero = NULL;
  void *wsum_def = NULL, *wsum_divchn = NULL, *wsum_muloa = NULL;
  void *dsum_def = NULL, *dsum_divchn = NULL, *dsum_muloa = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  void *prev_def = NULL, *prev_divchn = NULL, *prev_muloa = NULL;
  graph_t g;
  adj_lst_t a;
  bern_arg_t b;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  dijkstra_ht_t daht_divchn, daht_muloa;
  clock_t t_def, t_divchn, t_muloa;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  daht_divchn.ht = &ht_divchn;
  daht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  daht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  daht_divchn.init = ht_divchn_init_helper;
  daht_divchn.align = ht_divchn_align_helper;
  daht_divchn.insert = ht_divchn_insert_helper;
  daht_divchn.search = ht_divchn_search_helper;
  daht_divchn.remove = ht_divchn_remove_helper;
  daht_divchn.free = ht_divchn_free_helper;
  daht_muloa.ht = &ht_muloa;
  daht_muloa.alpha_n = C_ALPHA_N_MULOA;
  daht_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  daht_muloa.init = ht_muloa_init_helper;
  daht_muloa.align = ht_muloa_align_helper;
  daht_muloa.insert = ht_muloa_insert_helper;
  daht_muloa.search = ht_muloa_search_helper;
  daht_muloa.remove = ht_muloa_remove_helper;
  daht_muloa.free = ht_muloa_free_helper;
  printf("Run a dijkstra test on random directed graphs with random weights"
	 " across vertex and weight types;\nthe runtime is averaged"
	 " over %lu runs from random start vertices\n", TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (k = 0; k < C_FN_WT_COUNT; k++){
      wt_size =  C_WT_SIZES[k];
      wt_l = realloc_perror(wt_l, 2, wt_size);
      wt_h = ptr(wt_l, 1, wt_size);
      C_SET_ZERO[k](wt_l);
      C_SET_TEST_MAX[k](wt_h, pow_two_perror(log_end));
      printf("\t%s range: [", C_WT_TYPES[k]);
      C_PRINT[k](wt_l);
      printf(", ");
      C_PRINT[k](wt_h);
      printf(")\n");
    }
    for (i = log_start; i <= log_end; i++){
      num_vts = pow_two_perror(i); /* 0 < n */
      printf("\t\t# vertices: %lu\n", TOLU(num_vts));
      for (j = 0; j < C_FN_VT_COUNT; j++){
	for (k = 0; k < C_FN_WT_COUNT; k++){
	  vt_size =  C_VT_SIZES[j];
	  wt_size =  C_WT_SIZES[k];
	  /* no declared type after realloc; new eff. type to be acquired */
	  wt_l = realloc_perror(wt_l, 3, wt_size);
	  wt_h = ptr(wt_l, 1, wt_size);
	  wt_zero = ptr(wt_l, 2, wt_size);
	  wsum_def =realloc_perror(wsum_def, 1, wt_size);
	  wsum_divchn = realloc_perror(wsum_divchn, 1, wt_size);
	  wsum_muloa = realloc_perror(wsum_muloa, 1, wt_size);
	  dsum_def = realloc_perror(dsum_def, 1, wt_size);
	  dsum_divchn = realloc_perror(dsum_divchn, 1, wt_size);
	  dsum_muloa = realloc_perror(dsum_muloa, 1, wt_size);
	  prev_def = realloc_perror(prev_def, num_vts, vt_size);
	  prev_divchn = realloc_perror(prev_divchn, num_vts, vt_size);
	  prev_muloa = realloc_perror(prev_muloa, num_vts, vt_size);
	  dist_def = realloc_perror(dist_def, num_vts, wt_size);
	  dist_divchn = realloc_perror(dist_divchn, num_vts, wt_size);
	  dist_muloa = realloc_perror(dist_muloa, num_vts, wt_size);
	  C_SET_ZERO[k](wt_l);
	  C_SET_TEST_MAX[k](wt_h, pow_two_perror(log_end));
	  C_SET_ZERO[k](wt_zero);
	  for (l = 0; l < num_vts; l++){
	    /* avoid trap representations in tests */
	    C_SET_ZERO[k](ptr(dist_def, l, wt_size));
	    C_SET_ZERO[k](ptr(dist_divchn, l, wt_size));
	    C_SET_ZERO[k](ptr(dist_muloa, l, wt_size));
	  } 
	  graph_base_init(&g, num_vts, vt_size, wt_size);
	  adj_lst_rand_dir_wts(&g, &a, wt_l, wt_h,
			       C_WRITE_VT[j], bern, &b, C_ADD_DIR_EDGE[k]);
	  for (l = 0; l < C_ITER; l++){
	    rand_start[l] = mul_high_sz(random_sz(), num_vts);
	  }
	  t_def = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_def, prev_def, wt_zero, NULL,
		     C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j],
		     C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_def = clock() - t_def;
	  C_SUM_DIST[k](dsum_def,
			wsum_def,
			&num_dwraps_def,
			&num_wwraps_def,
			&num_paths_def,
			num_vts,
			vt_size,
			dist_def,
			prev_def,
			C_READ_VT[j]);
	  t_divchn = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_divchn, prev_divchn, wt_zero,
		     &daht_divchn, C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j],
		     C_CMP_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_divchn = clock() - t_divchn;
	  C_SUM_DIST[k](dsum_divchn,
			wsum_divchn,
			&num_dwraps_divchn,
			&num_wwraps_divchn,
			&num_paths_divchn,
			num_vts,
			vt_size,
			dist_def,
			prev_def,
			C_READ_VT[j]);
	  t_muloa = clock();
	  for (l = 0; l < C_ITER; l++){
	    dijkstra(&a, rand_start[l], dist_muloa, prev_muloa, wt_zero,
		     &daht_muloa, C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j],
		     C_CMP_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
	  }
	  t_muloa = clock() - t_muloa;
	  C_SUM_DIST[k](dsum_muloa,
			wsum_muloa,
			&num_dwraps_muloa,
			&num_wwraps_muloa,
			&num_paths_muloa,
			num_vts,
			vt_size,
			dist_def,
			prev_def,
			C_READ_VT[j]);
	  if (k < C_FN_INTEGRAL_WT_COUNT){
	    res *= (C_CMP_WT[k](dsum_def, dsum_divchn) == 0 &&
		    C_CMP_WT[k](dsum_divchn, dsum_muloa) == 0 &&
		    C_CMP_WT[k](wsum_def, wsum_divchn) == 0 &&
		    C_CMP_WT[k](wsum_divchn, wsum_muloa) == 0);
	  }
	  res *= (num_dwraps_def == num_dwraps_divchn &&
		  num_dwraps_divchn == num_dwraps_muloa &&
		  num_wwraps_def == num_wwraps_divchn && 
		  num_wwraps_divchn == num_wwraps_muloa &&
		  num_paths_def == num_paths_divchn &&
		  num_paths_divchn == num_paths_muloa);
	  printf("\t\t\t# edges: %lu\n", TOLU(a.num_es));
	  printf("\t\t\t\t%s %s dijkstra default ht:     %.8f seconds\n"
		 "\t\t\t\t%s %s dijkstra ht_divchn:      %.8f seconds\n"
		 "\t\t\t\t%s %s dijkstra ht_muloa:       %.8f seconds\n",
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_def / C_ITER / CLOCKS_PER_SEC,
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_divchn / C_ITER / CLOCKS_PER_SEC,
		 C_VT_TYPES[j], C_WT_TYPES[k],
		 (double)t_muloa / C_ITER / CLOCKS_PER_SEC);
	  printf("\t\t\t\t%s %s correctness:             ",
		 C_VT_TYPES[j], C_WT_TYPES[k]);
	  print_test_result(res);
	  printf("\t\t\t\t%s %s last # paths:            %lu\n",
		 C_VT_TYPES[j], C_WT_TYPES[k], TOLU(num_paths_def - 1));
	  printf("\t\t\t\t%s %s last [# wraps, d sum]:   [%lu, ",
		 C_VT_TYPES[j], C_WT_TYPES[k], TOLU(num_dwraps_def));
	  C_PRINT[k](dsum_def);
	  printf("]\n");
	  printf("\t\t\t\t%s %s last [# wraps, wt sum]:  [%lu, ",
		 C_VT_TYPES[j], C_WT_TYPES[k], TOLU(num_wwraps_def));
	  C_PRINT[k](wsum_def);
	  printf("]\n\n");
	  res = 1;
	  adj_lst_free(&a);
	}
      }
    }
  }
  free(rand_start);
  free(wt_l);
  free(wsum_def);
  free(wsum_divchn);
  free(wsum_muloa);
  free(dsum_def);
  free(dsum_divchn);
  free(dsum_muloa);
  free(dist_def);
  free(dist_divchn);
  free(dist_muloa);
  free(prev_def);
  free(prev_divchn);
  free(prev_muloa);
  rand_start = NULL;
  wt_l = NULL;
  wt_h = NULL;
  wt_zero = NULL;
  wsum_def = NULL;
  wsum_divchn = NULL;
  wsum_muloa = NULL;
  dsum_def = NULL;
  dsum_divchn = NULL;
  dsum_muloa = NULL;
  dist_def = NULL;
  dist_divchn = NULL;
  dist_muloa = NULL;
  prev_def = NULL;
  prev_divchn = NULL;
  prev_muloa = NULL;
}

/**
   Portable random number generation. For better uniformity (according
   to rand) RAND_MAX should be 32767, a power to two minus one, or many
   times larger than 32768 on a given system if it is not a power of two
   minus one. Given a value n of one of the below unsigned integral types,
   the overflow bits after multipying n with a random number of the same
   type represent a random number of the type within the range [0, n).

   According to C89 (draft):

   "When a signed integer is converted to an unsigned integer with equal or
   greater size, if the value of the signed integer is nonnegative, its
   value is unchanged."

   "For each of the signed integer types, there is a corresponding (but
   different) unsigned integer type (designated with the keyword unsigned)
   that uses the same amount of storage (including sign information) and has
   the same alignment requirements" 

   "When an integer is demoted to an unsigned integer with smaller size,
   the result is the nonnegative remainder on division by the number one
   greater than the largest unsigned number that can be represented in the
   type with smaller size. "

   It is guaranteed that: sizeof(short) <= sizeof(int) <= sizeof(long)
*/

unsigned short random_ushort(){
  size_t i;
  unsigned short ret = 0;
  for (i = 0; i <= C_USHORT_BIT_MOD; i++){
    ret |= ((unsigned short)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
	    (i * C_RANDOM_BIT));
  }
  return ret;
}

unsigned int random_uint(){
  size_t i;
  unsigned int ret = 0;
  for (i = 0; i <= C_UINT_BIT_MOD; i++){
    ret |= ((unsigned int)RANDOM() & C_RANDOM_MASK) << (i * C_RANDOM_BIT);
  }
  return ret;
}

unsigned long random_ulong(){
  size_t i;
  unsigned long ret = 0;
  for (i = 0; i <= C_ULONG_BIT_MOD; i++){
    ret |= ((unsigned long)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
	    (i * C_RANDOM_BIT));
  }
  return ret;
}

size_t random_sz(){
  size_t i;
  size_t ret = 0;
  for (i = 0; i <= C_SZ_BIT_MOD; i++){
    ret |= ((size_t)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
	    (i * C_RANDOM_BIT));
  }
  return ret;
}

unsigned short mul_high_ushort(unsigned short a, unsigned short b){
  unsigned short al, bl, ah, bh, al_bh, ah_bl;
  unsigned short overlap;
  al = a & C_USHORT_LOW_MASK;
  bl = b & C_USHORT_LOW_MASK;
  ah = a >> C_USHORT_HALF_BIT;
  bh = b >> C_USHORT_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_USHORT_LOW_MASK) +
	     (al_bh & C_USHORT_LOW_MASK) +
	     (al * bl >> C_USHORT_HALF_BIT));
  return ((overlap >> C_USHORT_HALF_BIT) +
	  ah * bh +
	  (ah_bl >> C_USHORT_HALF_BIT) +
	  (al_bh >> C_USHORT_HALF_BIT));
}

unsigned int mul_high_uint(unsigned int a, unsigned int b){
  unsigned int al, bl, ah, bh, al_bh, ah_bl;
  unsigned int overlap;
  al = a & C_UINT_LOW_MASK;
  bl = b & C_UINT_LOW_MASK;
  ah = a >> C_UINT_HALF_BIT;
  bh = b >> C_UINT_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_UINT_LOW_MASK) +
	     (al_bh & C_UINT_LOW_MASK) +
	     (al * bl >> C_UINT_HALF_BIT));
  return ((overlap >> C_UINT_HALF_BIT) +
	  ah * bh +
	  (ah_bl >> C_UINT_HALF_BIT) +
	  (al_bh >> C_UINT_HALF_BIT));
}

unsigned long mul_high_ulong(unsigned long a, unsigned long b){
  unsigned long al, bl, ah, bh, al_bh, ah_bl;
  unsigned long overlap;
  al = a & C_ULONG_LOW_MASK;
  bl = b & C_ULONG_LOW_MASK;
  ah = a >> C_ULONG_HALF_BIT;
  bh = b >> C_ULONG_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_ULONG_LOW_MASK) +
	     (al_bh & C_ULONG_LOW_MASK) +
	     (al * bl >> C_ULONG_HALF_BIT));
  return ((overlap >> C_ULONG_HALF_BIT) +
	  ah * bh +
	  (ah_bl >> C_ULONG_HALF_BIT) +
	  (al_bh >> C_ULONG_HALF_BIT));
}

size_t mul_high_sz(size_t a, size_t b){
  size_t al, bl, ah, bh, al_bh, ah_bl;
  size_t overlap;
  al = a & C_SZ_LOW_MASK;
  bl = b & C_SZ_LOW_MASK;
  ah = a >> C_SZ_HALF_BIT;
  bh = b >> C_SZ_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_SZ_LOW_MASK) +
	     (al_bh & C_SZ_LOW_MASK) +
	     (al * bl >> C_SZ_HALF_BIT));
  return ((overlap >> C_SZ_HALF_BIT) +
	  ah * bh +
	  (ah_bl >> C_SZ_HALF_BIT) +
	  (al_bh >> C_SZ_HALF_BIT));
}

/**
   Value initiliazation, arithmetic, and printing. 

   The functions with the set_test_max prefix, set the maximum value of
   random weights (i.e. unreached upper bound) and reflect that at most
   n - 1 edges participate in a path because otherwise there is a cycle.

   The functions with the sum_dist prefix compute i) the sum of the unsigned
   integer path distances, and ii) the sum of the unsigned integer edge
   weights in all paths, by counting the wrap-arounds in each case. The
   computation is overflow safe, because i) each path distance is at most
   the maximum value of the unsigned integer type used to represent weights,
   and there are at most n - 1 paths with a non-zero distance, and ii) there
   are at most n - 1 edges in the set of edges across all paths, because
   when v is popped (u, v) is added to the set with u being a popped vertex.
   The wrap-around count for ii) should be zero in the tests because any
   set of n - 1 edges can be added without overflow. The total sum in
   each case is: # wrap-arounds * max value of weight type + 
   # wraps-arounds + wrapped sum of weight type. 

   For double weights, the maximum value in 1.0 / n, where n can be
   converted to double as required in tests, and the number of wrap-arounds
   is 0.
*/

void set_zero_ushort(void *a){
  *(unsigned short *)a = 0;
}
void set_zero_uint(void *a){
  *(unsigned int *)a = 0;
}
void set_zero_ulong(void *a){
  *(unsigned long *)a = 0;
}
void set_zero_sz(void *a){
  *(size_t *)a = 0;
}
void set_zero_double(void *a){
  *(double *)a = 0.0;
}

void set_one_ushort(void *a){
  *(unsigned short *)a = 1;
}
void set_one_uint(void *a){
  *(unsigned int *)a = 1;
}
void set_one_ulong(void *a){
  *(unsigned long *)a = 1;
}
void set_one_sz(void *a){
  *(size_t *)a = 1;
}
void set_one_double(void *a){
  *(double *)a = 1.0;
}

void set_test_max_ushort(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned short *)a = C_USHORT_MAX;
  }else{
    /* usual arithmetic conversions */
    *(unsigned short *)a = C_USHORT_MAX / num_vts;
  }
}

void set_test_max_uint(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned int *)a = C_UINT_MAX;
  }else{
    /* usual arithmetic conversions */
    *(unsigned int *)a = C_UINT_MAX / num_vts;
  }
}

void set_test_max_ulong(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned long *)a = C_ULONG_MAX;
  }else{
    /* usual arithmetic conversions */
    *(unsigned long *)a = C_ULONG_MAX / num_vts;
  }
}

void set_test_max_sz(void *a, size_t num_vts){
  if (num_vts == 0){
    *(size_t *)a = C_SZ_MAX;
  }else{
    *(size_t *)a = C_SZ_MAX / num_vts;
  }
}

void set_test_max_double(void *a, size_t num_vts){
  *(double *)a = 1.0 / num_vts;
}

void sum_dist_ushort(void *dist_sum,
		     void *wt_sum,
		     size_t *num_dist_wraps, /* <= num_vts */
		     size_t *num_wt_wraps, /* <= num_vts */
		     size_t *num_paths, /* <= num_vts */
		     size_t num_vts,
		     size_t vt_size,
		     const void *dist,
		     const void *prev,
		     size_t (*read_vt)(const void *)){
  size_t i, p;
  unsigned short *dsum = dist_sum;
  unsigned short *wsum = wt_sum;
  const unsigned short *d = dist;
  *dsum = 0;
  *wsum = 0;
  *num_dist_wraps = 0;
  *num_wt_wraps = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    p = read_vt(ptr(prev, i, vt_size));
    if (p != num_vts){
      (*num_dist_wraps) += (C_USHORT_MAX - *dsum < d[i]);
      /* includes d[i] < d[p] */
      (*num_wt_wraps) += (C_USHORT_MAX - *wsum < d[i] - d[p]);
      *dsum += d[i];
      *wsum += d[i] - d[p];
      (*num_paths)++;
    }
  }
}

void sum_dist_uint(void *dist_sum,
		   void *wt_sum,
		   size_t *num_dist_wraps, /* <= num_vts */
		   size_t *num_wt_wraps, /* <= num_vts */
		   size_t *num_paths, /* <= num_vts */
		   size_t num_vts,
		   size_t vt_size,
		   const void *dist,
		   const void *prev,
		   size_t (*read_vt)(const void *)){
  size_t i, p;
  unsigned int *dsum = dist_sum;
  unsigned int *wsum = wt_sum;
  const unsigned int *d = dist;
  *dsum = 0;
  *wsum = 0;
  *num_dist_wraps = 0;
  *num_wt_wraps = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    p = read_vt(ptr(prev, i, vt_size));
    if (p != num_vts){
      (*num_dist_wraps) += (C_UINT_MAX - *dsum < d[i]);
      /* includes d[i] < d[p] */
      (*num_wt_wraps) += (C_UINT_MAX - *wsum < d[i] - d[p]);
      *dsum += d[i];
      *wsum += d[i] - d[p];
      (*num_paths)++;
    }
  }
}

void sum_dist_ulong(void *dist_sum,
		    void *wt_sum,
		    size_t *num_dist_wraps, /* <= num_vts */
		    size_t *num_wt_wraps, /* <= num_vts */
		    size_t *num_paths, /* <= num_vts */
		    size_t num_vts,
		    size_t vt_size,
		    const void *dist,
		    const void *prev,
		    size_t (*read_vt)(const void *)){
  size_t i, p;
  unsigned long *dsum = dist_sum;
  unsigned long *wsum = wt_sum;
  const unsigned long *d = dist;
  *dsum = 0;
  *wsum = 0;
  *num_dist_wraps = 0;
  *num_wt_wraps = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    p = read_vt(ptr(prev, i, vt_size));
    if (p != num_vts){
      (*num_dist_wraps) += (C_ULONG_MAX - *dsum < d[i]);
      /* includes d[i] < d[p] */
      (*num_wt_wraps) += (C_ULONG_MAX - *wsum < d[i] - d[p]);
      *dsum += d[i];
      *wsum += d[i] - d[p];
      (*num_paths)++;
    }
  }
}

void sum_dist_sz(void *dist_sum,
		 void *wt_sum,
		 size_t *num_dist_wraps, /* <= num_vts */
		 size_t *num_wt_wraps, /* <= num_vts */
		 size_t *num_paths, /* <= num_vts */
		 size_t num_vts,
		 size_t vt_size,
		 const void *dist,
		 const void *prev,
		 size_t (*read_vt)(const void *)){
  size_t i, p;
  size_t *dsum = dist_sum;
  size_t *wsum = wt_sum;
  const size_t *d = dist;
  *dsum = 0;
  *wsum = 0;
  *num_dist_wraps = 0;
  *num_wt_wraps = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    p = read_vt(ptr(prev, i, vt_size));
    if (p != num_vts){
      (*num_dist_wraps) += (C_SZ_MAX - *dsum < d[i]);
      /* includes d[i] < d[p] */
      (*num_wt_wraps) += (C_SZ_MAX - *wsum < d[i] - d[p]);
      *dsum += d[i];
      *wsum += d[i] - d[p];
      (*num_paths)++;
    }
  }
}

void sum_dist_double(void *dist_sum,
		     void *wt_sum,
		     size_t *num_dist_wraps, /* <= num_vts */
		     size_t *num_wt_wraps, /* <= num_vts */
		     size_t *num_paths, /* <= num_vts */
		     size_t num_vts,
		     size_t vt_size,
		     const void *dist,
		     const void *prev,
		     size_t (*read_vt)(const void *)){
  size_t i, p;
  double *dsum = dist_sum;
  double *wsum = wt_sum;
  const double *d = dist;
  *dsum = 0;
  *wsum = 0;
  *num_dist_wraps = 0;
  *num_wt_wraps = 0;
  *num_paths = 0;
  for (i = 0; i < num_vts; i++){
    p = read_vt(ptr(prev, i, vt_size));
    if (p != num_vts){
      *dsum += d[i];
      *wsum += d[i] - d[p];
      (*num_paths)++;
    }
  }
}

void print_ushort(const void *a){
  printf("%hu", *(const unsigned short *)a);
}
void print_uint(const void *a){
  printf("%u", *(const unsigned int *)a);
}
void print_ulong(const void *a){
  printf("%lu", *(const unsigned long *)a);
}
void print_sz(const void *a){
  printf("%lu", TOLU(*(const size_t *)a));
}
void print_double(const void *a){
  printf("%.8f", *(const double *)a);
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Prints an array.
*/
void print_arr(const void *arr,
	       size_t size,
	       size_t n,
	       void (*print_elt)(const void *)){
  size_t i;
  for (i = 0; i < n; i++){
    print_elt(ptr(arr, i, size));
    printf(" ");
  }
}

/**
   Prints a prev array.
*/
void print_prev(const adj_lst_t *a,
		const void *prev,
		void (*print_vt)(const void *)){
  print_arr(prev, a->vt_size, a->num_vts, print_vt);
}

/**
   Prints a dist array.
*/
void print_dist(const adj_lst_t *a,
		const void *dist,
		const void *prev,
		const void *wt_zero,
		size_t (*read_vt)(const void *),
		void (*print_wt)(const void *)){
  size_t i;
  for (i = 0; i < a->num_vts; i++){
    if (read_vt(ptr(prev, i, a->vt_size)) != a->num_vts){
      print_wt(ptr(dist, i, a->wt_size));
      printf(" ");
    }else{
      print_wt(wt_zero);
      printf(" ");
    }
  }
}

/**
   Prints an adjacency list. If the graph is unweighted, then print_wt is
   NULL.
*/
void print_adj_lst(const adj_lst_t *a,
		   void (*print_vt)(const void *),
		   void (*print_wt)(const void *)){
  size_t i;
  const void *p = NULL, *p_start = NULL, *p_end = NULL;
  printf("\tvertices: \n");
  for (i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", TOLU(i));
    p_start = a->vt_wts[i]->elts;
    p_end = (char *)p_start + a->vt_wts[i]->num_elts * a->pair_size;
    for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
      print_vt(p);
      printf(" ");
    }
    printf("\n");
  }
  if (a->wt_size > 0 && print_wt != NULL){
    printf("\tweights: \n");
    for (i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", TOLU(i));
      p_start = a->vt_wts[i]->elts;
      p_end = (char *)p_start + a->vt_wts[i]->num_elts * a->pair_size;
      for (p = p_start; p != p_end; p = (char *)p + a->pair_size){
	print_wt((char *)p + a->wt_offset);
	printf(" ");
      }
      printf("\n");
    }
  }
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
  if (args[0] > C_USHORT_BIT - 1 ||
      args[1] > C_USHORT_BIT - 1 ||
      args[1] < args[0] ||
      args[2] > 1 ||
      args[3] > 1 ||
      args[4] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  /*pow_two_perror later tests that # vertices is representable as size_t */
  if (args[2]) run_small_graph_test();
  if (args[3]) run_bfs_comparison_test(args[0], args[1]);
  if (args[4]) run_rand_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
