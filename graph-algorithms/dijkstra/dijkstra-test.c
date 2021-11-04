/**
   dijkstra-test.c

   Tests of Dijkstra's algorithm with a hash table parameter across
   i) default, division-based and multiplication-based hash tables, ii)
   vertex types, and iii) edge weight types.

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
   C89/C90 and C99 with the requirement that the width of size_t
   is greater or equal to 16, less than 2040, and is even.

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
  "[0, # bits in size_t / 2] : n for 2^n vertices in smallest graph\n"
  "[0, # bits in size_t / 2] : n for 2^n vertices in largest graph\n"
  "[0, 1] : small graph test on/off\n"
  "[0, 1] : bfs comparison test on/off\n"
  "[0, 1] : random graphs with random size_t weights test on/off\n";
const int C_ARGC_MAX = 6;
const size_t C_ARGS_DEF[5] = {0u, 10u, 1u, 1u, 1u};
const size_t C_FULL_BIT = UINT_WIDTH_FROM_MAX((size_t)-1);

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

const size_t C_FN_VT_COUNT = 4;
const size_t C_FN_WT_COUNT = 5;
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
  graph_cmp_ushort,
  graph_cmp_uint,
  graph_cmp_ulong,
  graph_cmp_sz};
const size_t C_VT_SIZES[4] = {
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t)};
const char *C_VT_TYPES[4] = {"ushort", "uint  ", "ulong ", "sz    "};

/* weight and print ops */

void zero_ushort(void *a);
void zero_uint(void *a);
void zero_ulong(void *a);
void zero_sz(void *a);
void zero_double(void *a);

int cmp_ushort(const void *a, const void *b);
int cmp_uint(const void *a, const void *b);
int cmp_ulong(const void *a, const void *b);
int cmp_sz(const void *a, const void *b);
int cmp_double(const void *a, const void *b);

void add_ushort(void *s, const void *a, const void *b);
void add_uint(void *s, const void *a, const void *b);
void add_ulong(void *s, const void *a, const void *b);
void add_sz(void *s, const void *a, const void *b);
void add_double(void *s, const void *a, const void *b);

void print_ushort(const void *a);
void print_uint(const void *a);
void print_ulong(const void *a);
void print_sz(const void *a);
void print_double(const void *a);

void (* const C_ZERO_WT[5])(void *) ={
  zero_ushort,
  zero_uint,
  zero_ulong,
  zero_sz,
  zero_double};
int (* const C_CMP_WT[5])(const void *, const void *) ={
  cmp_ushort,
  cmp_uint,
  cmp_ulong,
  cmp_sz,
  cmp_double};
void (* const C_ADD_WT[5])(void *, const void *, const void *) ={
  add_ushort,
  add_uint,
  add_ulong,
  add_sz,
  add_double};
void (* const C_PRINT[5])(const void *) ={
  print_ushort,
  print_uint,
  print_ulong,
  print_sz,
  print_double};
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
const size_t C_ITER = 10u;
const size_t C_PROBS_COUNT = 7u;
const double C_PROBS[7] = {1.000000, 0.250000, 0.062500,
			   0.015625, 0.003906, 0.000977,
			   0.000000};
const size_t C_WEIGHT_HIGH = ((size_t)-1 >>
			      ((UINT_WIDTH_FROM_MAX((size_t)-1) + 1) / 2));

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

void init_ushort_ushort_no_edges(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned short),
		  sizeof(unsigned short));
}

void init_ulong_ushort_no_edges(graph_t *g){
  graph_base_init(g,
		  C_NUM_VTS,
		  sizeof(unsigned long),
		  sizeof(unsigned short));
}

/**
   Write, compare, and add weights.
*/

void zero_ushort(void *a){
  *(unsigned short *)a = 0;
}
void zero_uint(void *a){
  *(unsigned int *)a = 0;
}
void zero_ulong(void *a){
  *(unsigned long *)a = 0;
}
void zero_sz(void *a){
  *(size_t *)a = 0;
}
void zero_double(void *a){
  *(double *)a = 0.0;
}

int cmp_ushort(const void *a, const void *b){
  if (*(const unsigned short *)a > *(const unsigned short *)b){
    return 1;
  }else if (*(const unsigned short *)a < *(const unsigned short *)b){
    return -1;
  }else{
    return 0;
  }
}
int cmp_uint(const void *a, const void *b){
  if (*(const unsigned int *)a > *(const unsigned int *)b){
    return 1;
  }else if (*(const unsigned int *)a < *(const unsigned int *)b){
    return -1;
  }else{
    return 0;
  }
}
int cmp_ulong(const void *a, const void *b){
  if (*(const unsigned long *)a > *(const unsigned long *)b){
    return 1;
  }else if (*(const unsigned long *)a < *(const unsigned long *)b){
    return -1;
  }else{
    return 0;
  }
}
int cmp_sz(const void *a, const void *b){
  if (*(const size_t *)a > *(const size_t *)b){
    return 1;
  }else if (*(const size_t *)a < *(const size_t *)b){
    return -1;
  }else{
    return 0;
  }
}
int cmp_double(const void *a, const void *b){
  if (*(const double *)a > *(const double *)b){
    return 1;
  }else if (*(const double *)a < *(const double *)b){
    return -1;
  }else{
    return 0;
  }
}

void add_ushort(void *s, const void *a, const void *b){
  *(unsigned short *)s = *(const unsigned short *)a + *(const unsigned short *)b;
}
void add_uint(void *s, const void *a, const void *b){
  *(unsigned int *)s = *(const unsigned int *)a + *(const unsigned int *)b;
}
void add_ulong(void *s, const void *a, const void *b){
  *(unsigned long *)s = *(const unsigned long *)a + *(const unsigned long *)b;
}
void add_sz(void *s, const void *a, const void *b){
  *(size_t *)s = *(const size_t *)a + *(const size_t *)b;
}
void add_double(void *s, const void *a, const void *b){
  *(double *)s = *(const double *)a + *(const double *)b;
}

/**
   Run a test on small graphs with across vertex and weight types.
*/

void small_graph_helper(void (*build)(adj_lst_t *,
				      const graph_t *,
				      size_t (*)(const void *))){
  size_t i, j, k;
  graph_t g;
  adj_lst_t a;
  ht_divchn_t ht_divchn;
  ht_muloa_t ht_muloa;
  daht_t daht_divchn, daht_muloa;
  void *wt_zero = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  void *prev_def = NULL, *prev_divchn = NULL, *prev_muloa = NULL;
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
	C_ZERO_WT[k](wt_zero);
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
  printf("Run a dijkstra test on a directed graph across vertex and weight types, with a\n"
	 "i) default hash table (index array)\n"
	 "ii) ht_divchn_t hash table\n"
	 "iii) ht_muloa_t hash table\n\n");
  small_graph_helper(adj_lst_dir_build);
  printf("Run a dijkstra test on an undirected graph across vertex and weight types, with a\n"
	 "i) default hash table (index array)\n"
	 "ii) ht_divchn_t hash table\n"
	 "iii) ht_muloa_t hash table\n\n");
  small_graph_helper(adj_lst_undir_build);
}

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Printing functions.
*/

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
  printf("%.1f", *(const double *)a);
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
  if (args[0] > C_FULL_BIT / 2 ||
      args[1] > C_FULL_BIT / 2 ||
      args[1] < args[0] ||
      args[2] > 1 ||
      args[3] > 1 ||
      args[4] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  run_small_graph_test();
  free(args);
  args = NULL;
  return 0;
}
