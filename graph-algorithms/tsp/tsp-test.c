/**
   tsp-test.c

   Tests of an exact solution of TSP without vertex revisiting
   across i) default, division and multiplication-based hash tables, ii)
   unsigned char vertices, and ii) edge weight types.

   The following command line arguments can be used to customize tests:
   tsp-test:
   -  [1, size_t width) : a
   -  [1, size_t width) : b s.t. a <= |V| <= b for all hash tables test
   -  [1, size_t width) : c
   -  [1, size_t width) : d s.t. c <= |V| <= d for default hash table test
   -  > 1 : e
   -  > 1 : f s.t. e <= |V| <= f for sparse graph test
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
#include "tsp.h"
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
  "tsp-test \n"
  "[1, size_t width) : a\n"
  "[1, size_t width) : b s.t. a <= |V| <= b for all hash tables test\n"
  "[1, size_t width) : c\n"
  "[1, size_t width) : d s.t. c <= |V| <= d for default hash table test\n"
  "> 1 : e\n"
  "> 1 : f s.t. e <= |V| <= f for sparse graph test\n"
  "[0, 1] : on/off for small graph test\n"
  "[0, 1] : on/off for all hash tables test\n"
  "[0, 1] : on/off for default hash table test\n"
  "[0, 1] : on/off for sparse graph test\n";
const int C_ARGC_ULIMIT = 11;
const size_t C_ARGS_DEF[10] = {10u, 11u, 20u, 21u, 100u, 104u, 1u, 1u, 1u, 1u};

/* hash table load factor upper bounds */
const size_t C_ALPHA_N_DIVCHN = 1u;
const size_t C_LOG_ALPHA_D_DIVCHN = 0u;
const size_t C_ALPHA_N_MULOA = 13107u;
const size_t C_LOG_ALPHA_D_MULOA = 15u;

/* small graph test */
const size_t C_NUM_VTS = 4u;
const size_t C_NUM_ES = 12u;

const unsigned char C_UCHAR_U[12] = {0u, 1u, 2u, 3u, 1u, 2u,
                                     3u, 0u, 0u, 2u, 1u, 3u};
const unsigned char C_UCHAR_V[12] = {1u, 2u, 3u, 0u, 0u, 1u,
                                     2u, 3u, 2u, 0u, 3u, 1u};
const unsigned char C_UCHAR_WTS[12] = {1u, 1u, 1u, 1u, 2u, 2u,
                                       2u, 2u, 2u, 2u, 2u, 2u};

const unsigned short C_USHORT_U[12] = {0u, 1u, 2u, 3u, 1u, 2u,
                                       3u, 0u, 0u, 2u, 1u, 3u};
const unsigned short C_USHORT_V[12] = {1u, 2u, 3u, 0u, 0u, 1u,
                                       2u, 3u, 2u, 0u, 3u, 1u};
const unsigned short C_USHORT_WTS[12] = {1u, 1u, 1u, 1u, 2u, 2u,
                                         2u, 2u, 2u, 2u, 2u, 2u};

const unsigned int C_UINT_U[12] = {0u, 1u, 2u, 3u, 1u, 2u,
                                   3u, 0u, 0u, 2u, 1u, 3u};
const unsigned int C_UINT_V[12] = {1u, 2u, 3u, 0u, 0u, 1u,
                                   2u, 3u, 2u, 0u, 3u, 1u};
const unsigned int C_UINT_WTS[12] = {1u, 1u, 1u, 1u, 2u, 2u,
                                     2u, 2u, 2u, 2u, 2u, 2u};

const unsigned long C_ULONG_U[12] = {0u, 1u, 2u, 3u, 1u, 2u,
                                     3u, 0u, 0u, 2u, 1u, 3u};
const unsigned long C_ULONG_V[12] = {1u, 2u, 3u, 0u, 0u, 1u,
                                     2u, 3u, 2u, 0u, 3u, 1u};
const unsigned long C_ULONG_WTS[12] = {1u, 1u, 1u, 1u, 2u, 2u,
                                       2u, 2u, 2u, 2u, 2u, 2u};

const size_t C_SZ_U[12] = {0u, 1u, 2u, 3u, 1u, 2u, 3u, 0u, 0u, 2u, 1u, 3u};
const size_t C_SZ_V[12] = {1u, 2u, 3u, 0u, 0u, 1u, 2u, 3u, 2u, 0u, 3u, 1u};
const size_t C_SZ_WTS[12] = {1u, 1u, 1u, 1u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u};

const double C_DOUBLE_WTS[12] = {1.0, 1.0, 1.0, 1.0, 2.0, 2.0,
                                 2.0, 2.0, 2.0, 2.0, 2.0, 2.0};

/* small graph initialization ops */
void init_uchar_uchar(struct graph *g);
void init_uchar_ushort(struct graph *g);
void init_uchar_uint(struct graph *g);
void init_uchar_ulong(struct graph *g);
void init_uchar_sz(struct graph *g);
void init_uchar_double(struct graph *g);

void init_ushort_uchar(struct graph *g);
void init_ushort_ushort(struct graph *g);
void init_ushort_uint(struct graph *g);
void init_ushort_ulong(struct graph *g);
void init_ushort_sz(struct graph *g);
void init_ushort_double(struct graph *g);

void init_uint_uchar(struct graph *g);
void init_uint_ushort(struct graph *g);
void init_uint_uint(struct graph *g);
void init_uint_ulong(struct graph *g);
void init_uint_sz(struct graph *g);
void init_uint_double(struct graph *g);

void init_ulong_uchar(struct graph *g);
void init_ulong_ushort(struct graph *g);
void init_ulong_uint(struct graph *g);
void init_ulong_ulong(struct graph *g);
void init_ulong_sz(struct graph *g);
void init_ulong_double(struct graph *g);

void init_sz_uchar(struct graph *g);
void init_sz_ushort(struct graph *g);
void init_sz_uint(struct graph *g);
void init_sz_ulong(struct graph *g);
void init_sz_sz(struct graph *g);
void init_sz_double(struct graph *g);

const size_t C_FN_VT_COUNT = 5u;
const size_t C_FN_WT_COUNT = 6u;
/*const size_t C_FN_INTEGRAL_WT_COUNT = 5u;*/
void (* const C_INIT_GRAPH[5][6])(struct graph *) ={
  {init_uchar_uchar,
   init_uchar_ushort,
   init_uchar_uint,
   init_uchar_ulong,
   init_uchar_sz,
   init_uchar_double},
  {init_ushort_uchar,
   init_ushort_ushort,
   init_ushort_uint,
   init_ushort_ulong,
   init_ushort_sz,
   init_ushort_double},
  {init_uint_uchar,
   init_uint_ushort,
   init_uint_uint,
   init_uint_ulong,
   init_uint_sz,
   init_uint_double},
  {init_ulong_uchar,
   init_ulong_ushort,
   init_ulong_uint,
   init_ulong_ulong,
   init_ulong_sz,
   init_ulong_double},
  {init_sz_uchar,
   init_sz_ushort,
   init_sz_uint,
   init_sz_ulong,
   init_sz_sz,
   init_sz_double}};

/* vertex ops */
size_t (* const C_READ_VT[5])(const void *) ={
  graph_read_uchar,
  graph_read_ushort,
  graph_read_uint,
  graph_read_ulong,
  graph_read_sz};
void (* const C_WRITE_VT[5])(void *, size_t) ={
  graph_write_uchar,
  graph_write_ushort,
  graph_write_uint,
  graph_write_ulong,
  graph_write_sz};
const size_t C_VT_SIZES[5] = {
  sizeof(unsigned char),
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t)};
const char *C_VT_TYPES[5] = {"uchar ",
                             "ushort",
                             "uint  ",
                             "ulong ",
                             "sz    "};

/* weight and print ops */

int cmp_double(const void *a, const void *b);
void add_double(void *s, const void *a, const void *b);

int (* const C_CMP_WT[6])(const void *, const void *) ={
  graph_cmp_uchar,
  graph_cmp_ushort,
  graph_cmp_uint,
  graph_cmp_ulong,
  graph_cmp_sz,
  cmp_double};
void (* const C_ADD_WT[6])(void *, const void *, const void *) ={
  graph_add_uchar,
  graph_add_ushort,
  graph_add_uint,
  graph_add_ulong,
  graph_add_sz,
  add_double};
const size_t C_WT_SIZES[6] = {
  sizeof(unsigned char),
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(unsigned long),
  sizeof(size_t),
  sizeof(double)};
const char *C_WT_TYPES[6] = {"uchar ",
                             "ushort",
                             "uint  ",
                             "ulong ",
                             "sz    ",
                             "double"};

/* random graph tests */
/* C89 (draft): UCHAR_MAX >= 255, USHRT_MAX >= 65535, UINT_MAX >= 65535,
   ULONG_MAX >= 4294967295, RAND_MAX >= 32767 */
const size_t C_RANDOM_BIT = 15u;
const unsigned int C_RANDOM_MASK = 32767u;

const size_t C_UCHAR_BIT = PRECISION_FROM_ULIMIT(UCHAR_MAX);
const size_t C_UCHAR_BIT_MOD = PRECISION_FROM_ULIMIT(UCHAR_MAX) / 15u;
const size_t C_UCHAR_HALF_BIT = PRECISION_FROM_ULIMIT(UCHAR_MAX) / 2u;
const unsigned char C_UCHAR_ULIMIT = UCHAR_MAX;
const unsigned char C_UCHAR_LOW_MASK =
  ((unsigned char)-1 >> (PRECISION_FROM_ULIMIT(UCHAR_MAX) / 2u));

const size_t C_USHORT_BIT = PRECISION_FROM_ULIMIT(USHRT_MAX);
const size_t C_USHORT_BIT_MOD = PRECISION_FROM_ULIMIT(USHRT_MAX) / 15u;
const size_t C_USHORT_HALF_BIT = PRECISION_FROM_ULIMIT(USHRT_MAX) / 2u;
const unsigned short C_USHORT_ULIMIT = USHRT_MAX;
const unsigned short C_USHORT_LOW_MASK =
  ((unsigned short)-1 >> (PRECISION_FROM_ULIMIT(USHRT_MAX) / 2u));

const size_t C_UINT_BIT = PRECISION_FROM_ULIMIT(UINT_MAX);
const size_t C_UINT_BIT_MOD = PRECISION_FROM_ULIMIT(UINT_MAX) / 15u;
const size_t C_UINT_HALF_BIT = PRECISION_FROM_ULIMIT(UINT_MAX) / 2u;
const unsigned int C_UINT_ULIMIT = UINT_MAX;
const unsigned int C_UINT_LOW_MASK =
  ((unsigned int)-1 >> (PRECISION_FROM_ULIMIT(UINT_MAX) / 2u));

const size_t C_ULONG_BIT = PRECISION_FROM_ULIMIT(ULONG_MAX);
const size_t C_ULONG_BIT_MOD = PRECISION_FROM_ULIMIT(ULONG_MAX) / 15u;
const size_t C_ULONG_HALF_BIT = PRECISION_FROM_ULIMIT(ULONG_MAX) / 2u;
const unsigned long C_ULONG_ULIMIT = ULONG_MAX;
const unsigned long C_ULONG_LOW_MASK =
  ((unsigned long)-1 >> (PRECISION_FROM_ULIMIT(ULONG_MAX) / 2u));

const size_t C_SZ_BIT = PRECISION_FROM_ULIMIT((size_t)-1);
const size_t C_SZ_BIT_MOD = PRECISION_FROM_ULIMIT((size_t)-1) / 15u;
const size_t C_SZ_HALF_BIT = PRECISION_FROM_ULIMIT((size_t)-1) / 2u;
const size_t C_SZ_ULIMIT = (size_t)-1;
const size_t C_SZ_LOW_MASK =
  ((size_t)-1 >> (PRECISION_FROM_ULIMIT((size_t)-1) / 2u));

const size_t C_ITER = 3u;
const size_t C_PROBS_COUNT = 3u;
const size_t C_SPARSE_PROBS_COUNT = 2u;
const double C_PROBS[3] = {1.0000, 0.2500, 0.0000};
const double C_SPARSE_PROBS[2] = {0.0050, 0.0025};
const double C_PROB_ZERO = 0.0;
const double C_PROB_ONE = 1.0;

/* random number generation and random graph construction */
unsigned char random_uchar();
unsigned short random_ushort();
unsigned int random_uint();
unsigned long random_ulong();
size_t random_sz();

unsigned char mul_high_uchar(unsigned char a, unsigned char b);
unsigned short mul_high_ushort(unsigned short a, unsigned short b);
unsigned int mul_high_uint(unsigned int a, unsigned int b);
unsigned long mul_high_ulong(unsigned long a, unsigned long b);
size_t mul_high_sz(size_t a, size_t b);

void add_dir_uchar_edge(struct adj_lst *a,
                        size_t u,
                        size_t v,
                        const void *wt_l,
                        const void *wt_h,
                        void (*write_vt)(void *, size_t),
                        int (*bern)(void *),
                        void *arg);
void add_dir_ushort_edge(struct adj_lst *a,
                         size_t u,
                         size_t v,
                         const void *wt_l,
                         const void *wt_h,
                         void (*write_vt)(void *, size_t),
                         int (*bern)(void *),
                         void *arg);
void add_dir_uint_edge(struct adj_lst *a,
                       size_t u,
                       size_t v,
                       const void *wt_l,
                       const void *wt_h,
                       void (*write_vt)(void *, size_t),
                       int (*bern)(void *),
                       void *arg);
void add_dir_ulong_edge(struct adj_lst *a,
                        size_t u,
                        size_t v,
                        const void *wt_l,
                        const void *wt_h,
                        void (*write_vt)(void *, size_t),
                        int (*bern)(void *),
                        void *arg);
void add_dir_sz_edge(struct adj_lst *a,
                     size_t u,
                     size_t v,
                     const void *wt_l,
                     const void *wt_h,
                     void (*write_vt)(void *, size_t),
                     int (*bern)(void *),
                     void *arg);
void add_dir_double_edge(struct adj_lst *a,
                         size_t u,
                         size_t v,
                         const void *wt_l,
                         const void *wt_h,
                         void (*write_vt)(void *, size_t),
                         int (*bern)(void *),
                         void *arg);

void (* const C_ADD_DIR_EDGE[6])(struct adj_lst *,
                                 size_t,
                                 size_t,
                                 const void *,
                                 const void *,
                                 void (*)(void *, size_t),
                                 int (*)(void *),
                                 void *) ={
  add_dir_uchar_edge,
  add_dir_ushort_edge,
  add_dir_uint_edge,
  add_dir_ulong_edge,
  add_dir_sz_edge,
  add_dir_double_edge};

/* value initiliazation operations */
void set_zero_uchar(void *a);
void set_zero_ushort(void *a);
void set_zero_uint(void *a);
void set_zero_ulong(void *a);
void set_zero_sz(void *a);
void set_zero_double(void *a);

void (* const C_SET_ZERO[6])(void *) ={
  set_zero_uchar,
  set_zero_ushort,
  set_zero_uint,
  set_zero_ulong,
  set_zero_sz,
  set_zero_double};

void set_one_uchar(void *a);
void set_one_ushort(void *a);
void set_one_uint(void *a);
void set_one_ulong(void *a);
void set_one_sz(void *a);
void set_one_double(void *a);

void (* const C_SET_ONE[6])(void *) ={
  set_one_uchar,
  set_one_ushort,
  set_one_uint,
  set_one_ulong,
  set_one_sz,
  set_one_double};

void set_high_uchar(void *a, size_t num_vts);
void set_high_ushort(void *a, size_t num_vts);
void set_high_uint(void *a, size_t num_vts);
void set_high_ulong(void *a, size_t num_vts);
void set_high_sz(void *a, size_t num_vts);
void set_high_double(void *a, size_t high);

void (* const C_SET_HIGH[6])(void *, size_t) ={
  set_high_uchar,
  set_high_ushort,
  set_high_uint,
  set_high_ulong,
  set_high_sz,
  set_high_double};

/* printing operations */
void print_uchar(const void *a);
void print_ushort(const void *a);
void print_uint(const void *a);
void print_ulong(const void *a);
void print_sz(const void *a);
void print_double(const void *a);

void (* const C_PRINT[6])(const void *) ={
  print_uchar,
  print_ushort,
  print_uint,
  print_ulong,
  print_sz,
  print_double};

void print_adj_lst(const struct adj_lst *a,
                   void (*print_vt)(const void *),
                   void (*print_wt)(const void *));
void print_test_result(int res);

/* additional operations */
void *ptr(const void *block, size_t i, size_t size);

/**
   Initialize small graphs across vertex and weight types.
*/

void init_uchar_uchar(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(unsigned char));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (unsigned char *)C_UCHAR_WTS;
}

void init_uchar_ushort(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_uchar_uint(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_uchar_ulong(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_uchar_sz(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_uchar_double(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned char),
                  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned char *)C_UCHAR_U;
  g->v = (unsigned char *)C_UCHAR_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_ushort_uchar(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(unsigned char));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned char *)C_UCHAR_WTS;
}

void init_ushort_ushort(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_ushort_uint(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_ushort_ulong(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_ushort_sz(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_ushort_double(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned short),
                  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned short *)C_USHORT_U;
  g->v = (unsigned short *)C_USHORT_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_uint_uchar(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(unsigned char));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned char *)C_UCHAR_WTS;
}

void init_uint_ushort(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_uint_uint(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_uint_ulong(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_uint_sz(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_uint_double(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned int),
                  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned int *)C_UINT_U;
  g->v = (unsigned int *)C_UINT_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_ulong_uchar(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(unsigned char));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned char *)C_UCHAR_WTS;
}

void init_ulong_ushort(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_ulong_uint(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_ulong_ulong(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_ulong_sz(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_ulong_double(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(unsigned long),
                  sizeof(double));
  g->num_es = C_NUM_ES;
  g->u = (unsigned long *)C_ULONG_U;
  g->v = (unsigned long *)C_ULONG_V;
  g->wts = (double *)C_DOUBLE_WTS;
}

void init_sz_uchar(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(size_t),
                  sizeof(unsigned char));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned char *)C_UCHAR_WTS;
}

void init_sz_ushort(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(size_t),
                  sizeof(unsigned short));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned short *)C_USHORT_WTS;
}

void init_sz_uint(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(size_t),
                  sizeof(unsigned int));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned int *)C_UINT_WTS;
}

void init_sz_ulong(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(size_t),
                  sizeof(unsigned long));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (unsigned long *)C_ULONG_WTS;
}

void init_sz_sz(struct graph *g){
  graph_base_init(g,
                  C_NUM_VTS,
                  sizeof(size_t),
                  sizeof(size_t));
  g->num_es = C_NUM_ES;
  g->u = (size_t *)C_SZ_U;
  g->v = (size_t *)C_SZ_V;
  g->wts = (size_t *)C_SZ_WTS;
}

void init_sz_double(struct graph *g){
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

void run_small_graph_test(){
  int ret_def = -1, ret_divchn = -1, ret_muloa = -1;
  size_t i, j, k;
  void *wt_zero = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  struct graph g;
  struct adj_lst a;
  struct ht_divchn ht_divchn;
  struct ht_muloa ht_muloa;
  struct tsp_ht tht_divchn, tht_muloa;
  tht_divchn.ht = &ht_divchn;
  tht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  tht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  tht_divchn.init = ht_divchn_init_helper;
  tht_divchn.align = ht_divchn_align_helper;
  tht_divchn.insert = ht_divchn_insert_helper;
  tht_divchn.search = ht_divchn_search_helper;
  tht_divchn.remove = ht_divchn_remove_helper;
  tht_divchn.free = ht_divchn_free_helper;
  tht_muloa.ht = &ht_muloa;
  tht_muloa.alpha_n = C_ALPHA_N_MULOA;
  tht_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  tht_muloa.init = ht_muloa_init_helper;
  tht_muloa.align = ht_muloa_align_helper;
  tht_muloa.insert = ht_muloa_insert_helper;
  tht_muloa.search = ht_muloa_search_helper;
  tht_muloa.remove = ht_muloa_remove_helper;
  tht_muloa.free = ht_muloa_free_helper;
  printf("Run a tsp test on a directed graph across vertex and weight"
         " types, with a\n"
         "i) default hash table (set index array)\n"
         "ii) ht_divchn hash table\n"
         "iii) ht_muloa hash table\n\n");
  for (i = 0; i < C_NUM_VTS; i++){
    printf("\tstart vertex: %lu\n", TOLU(i));
    for (j = 0; j < C_FN_VT_COUNT; j++){
      printf("\t\tvertex type: %s\n", C_VT_TYPES[j]);
      for (k = 0; k < C_FN_WT_COUNT; k++){
        printf("\t\t\tweight type: %s\n", C_WT_TYPES[k]);
        C_INIT_GRAPH[j][k](&g);
        adj_lst_base_init(&a, &g);
        adj_lst_dir_build(&a, &g, C_READ_VT[j]);
        /* no declared type after realloc; new eff. type to be acquired */
        wt_zero = realloc_perror(wt_zero, 1, a.wt_size);
        dist_def = realloc_perror(dist_def, 1, a.wt_size);
        dist_divchn = realloc_perror(dist_divchn, 1, a.wt_size);
        dist_muloa = realloc_perror(dist_muloa, 1, a.wt_size);
        C_SET_ZERO[k](wt_zero);
        /* avoid trap representations in tests */
        C_SET_ZERO[k](dist_def);
        C_SET_ZERO[k](dist_divchn);
        C_SET_ZERO[k](dist_muloa);
        ret_def = tsp(&a, i, dist_def, wt_zero, NULL,
                      C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
        ret_divchn = tsp(&a, i, dist_divchn, wt_zero, &tht_divchn,
                         C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
        ret_muloa = tsp(&a, i, dist_muloa, wt_zero, &tht_muloa,
                        C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
        adj_lst_free(&a);
        printf("\t\t\t\tdefault dist: ");
        C_PRINT[k](dist_def);
        printf(", tour exists: %s", ret_def ? "N" : "Y");
        printf("\n");
        printf("\t\t\t\tdivchn dist:  ");
        C_PRINT[k](dist_divchn);
        printf(", tour exists: %s", ret_divchn ? "N" : "Y");
        printf("\n");
        printf("\t\t\t\tmuloa dist:   ");
        C_PRINT[k](dist_muloa);
        printf(", tour exists: %s", ret_muloa ? "N" : "Y");
        printf("\n");
      }
    }
  }
  printf("\n");
  free(wt_zero);
  free(dist_def);
  free(dist_divchn);
  free(dist_muloa);
  wt_zero = NULL;
  dist_def = NULL;
  dist_divchn = NULL;
  dist_muloa = NULL;
}

/**
    Construct adjacency lists of random directed graphs with random
    weights across vertex and weight types, with an existing tour.

    A function with the add_dir_ prefix adds a (u, v) edge to an adjacency
    list of a weighted graph, preinitialized with at least adj_lst_base_init
    and with the number of vertices n greater or equal to 1. An edge (u, v),
    where u < n nad v < n, is added with the Bernoulli distribution according
    to the bern and arg parameter values. wt_l and wt_h point to wt_size
    blocks with values l and h of the type used to represent weights in the
    adjacency list, and l must be non-negative and less or equal to h.
    If (u, v) is added, a random weight in [l, h) is chosen for the edge.

    adj_lst_rand_dir_wts builds a random adjacency list with an existing tour,
    with one of the above functions as a parameter value. g points to a graph
    preinitialized with graph_base_init with at least one vertex. a points to
    a preallocated block of size sizeof(struct adj_lst).
*/

struct bern_arg{
  double p;
};

int bern(void *arg){
  struct bern_arg *b = arg;
  if (b->p <= C_PROB_ZERO) return 0;
  return (b->p >= C_PROB_ONE || b->p > DRAND());
}

void add_dir_uchar_edge(struct adj_lst *a,
                        size_t u,
                        size_t v,
                        const void *wt_l,
                        const void *wt_h,
                        void (*write_vt)(void *, size_t),
                        int (*bern)(void *),
                        void *arg){
  unsigned char rand_val =
    *(unsigned char *)wt_l +
    mul_high_uchar(random_uchar(),
                   (*(unsigned char *)wt_h - *(unsigned char *)wt_l));
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_dir_ushort_edge(struct adj_lst *a,
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

void add_dir_uint_edge(struct adj_lst *a,
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

void add_dir_ulong_edge(struct adj_lst *a,
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

void add_dir_sz_edge(struct adj_lst *a,
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

void add_dir_double_edge(struct adj_lst *a,
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

void adj_lst_rand_dir_wts(const struct graph *g,
                          struct adj_lst *a,
                          const void *wt_l,
                          const void *wt_h,
                          const void *wt_one,
                          void (*write_vt)(void *, size_t),
                          int (*bern)(void *),
                          void *arg,
                          void (*add_dir_edge)(struct adj_lst *,
                                               size_t,
                                               size_t,
                                               const void *,
                                               const void *,
                                               void (*)(void *, size_t),
                                               int (*)(void *),
                                               void *)){
  size_t i, j;
  struct bern_arg ba;
  adj_lst_base_init(a, g);
  ba.p = C_PROB_ONE;
  for (i = 0; i < a->num_vts - 1; i++){
    for (j = i + 1; j < a->num_vts; j++){
      if (a->num_vts == 2){
        add_dir_edge(a, i, j, wt_one, wt_one, write_vt, bern, &ba);
        add_dir_edge(a, j, i, wt_one, wt_one, write_vt, bern, &ba);
      }else if (j - i == 1){
        add_dir_edge(a, i, j, wt_one, wt_one, write_vt, bern, &ba);
        add_dir_edge(a, j, i, wt_l, wt_h, write_vt, bern, arg);
      }else if (i == 0 && j == a->num_vts - 1){
        add_dir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
        add_dir_edge(a, j, i, wt_one, wt_one, write_vt, bern, &ba);
      }else{
        add_dir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
        add_dir_edge(a, j, i, wt_l, wt_h, write_vt, bern, arg);
      }
    }
  }
}

/**
   Tests tsp on random directed graphs with random non-tour weights and a
   known tour, across edge weight types, vertex types, as well as default,
   division-based and multiplication-based hash tables.
*/
void run_rand_graph_test(size_t num_start, size_t num_end){
  int res = 1;
  int ret_def = -1, ret_divchn = -1, ret_muloa = -1;
  size_t p, i, j, k, l;
  size_t num_vts;
  size_t vt_size;
  size_t wt_size;
  size_t *rand_start = NULL;
  void *wt_l = NULL, *wt_h = NULL;
  void *wt_zero = NULL, *wt_one = NULL;
  void *dist_def = NULL, *dist_divchn = NULL, *dist_muloa = NULL;
  struct graph g;
  struct adj_lst a;
  struct bern_arg b;
  struct ht_divchn ht_divchn;
  struct ht_muloa ht_muloa;
  struct tsp_ht tht_divchn, tht_muloa;
  clock_t t_def, t_divchn, t_muloa;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  tht_divchn.ht = &ht_divchn;
  tht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  tht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  tht_divchn.init = ht_divchn_init_helper;
  tht_divchn.align = ht_divchn_align_helper;
  tht_divchn.insert = ht_divchn_insert_helper;
  tht_divchn.search = ht_divchn_search_helper;
  tht_divchn.remove = ht_divchn_remove_helper;
  tht_divchn.free = ht_divchn_free_helper;
  tht_muloa.ht = &ht_muloa;
  tht_muloa.alpha_n = C_ALPHA_N_MULOA;
  tht_muloa.log_alpha_d = C_LOG_ALPHA_D_MULOA;
  tht_muloa.init = ht_muloa_init_helper;
  tht_muloa.align = ht_muloa_align_helper;
  tht_muloa.insert = ht_muloa_insert_helper;
  tht_muloa.search = ht_muloa_search_helper;
  tht_muloa.remove = ht_muloa_remove_helper;
  tht_muloa.free = ht_muloa_free_helper;
  printf("Run a tsp test on random directed graphs with existing tours "
         "across vertex and weight types;\nthe runtime is averaged"
         " over %lu runs from random start vertices\n", TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = num_start; i <= num_end; i++){
      num_vts = i;
      printf("\t\t# vertices: %lu\n", TOLU(num_vts));
      for (j = 0; j < C_FN_VT_COUNT; j++){
        for (k = 0; k < C_FN_WT_COUNT; k++){
          vt_size =  C_VT_SIZES[j];
          wt_size =  C_WT_SIZES[k];
          /* no declared type after realloc; new eff. type to be acquired */
          wt_l = realloc_perror(wt_l, 7, wt_size);
          wt_h = ptr(wt_l, 1, wt_size);
          wt_zero = ptr(wt_l, 2, wt_size);
          wt_one = ptr(wt_l, 3, wt_size);
          dist_def = ptr(wt_l, 4, wt_size);
          dist_divchn = ptr(wt_l, 5, wt_size);
          dist_muloa = ptr(wt_l, 6, wt_size);
          C_SET_ONE[k](wt_l);
          C_SET_HIGH[k](wt_h, num_vts);
          if (C_CMP_WT[k](wt_l, wt_h) > 0){
            memcpy(wt_h, wt_l, wt_size);
          }
          C_SET_ZERO[k](wt_zero);
          C_SET_ONE[k](wt_one);
          /* avoid trap representations in tests */
          C_SET_ZERO[k](dist_def);
          C_SET_ZERO[k](dist_divchn);
          C_SET_ZERO[k](dist_muloa);
          graph_base_init(&g, num_vts, vt_size, wt_size);
          adj_lst_rand_dir_wts(&g, &a, wt_l, wt_h, wt_one,
                               C_WRITE_VT[j], bern, &b, C_ADD_DIR_EDGE[k]);
          for (l = 0; l < C_ITER; l++){
            rand_start[l] = mul_high_sz(random_sz(), num_vts);
          }
          t_def = clock();
          for (l = 0; l < C_ITER; l++){
            ret_def = tsp(&a, rand_start[l],
                          dist_def, wt_zero, NULL,
                          C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
          }
          t_def = clock() - t_def;
          t_divchn = clock();
          for (l = 0; l < C_ITER; l++){
            ret_divchn = tsp(&a, rand_start[l],
                             dist_divchn, wt_zero, &tht_divchn,
                             C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
          }
          t_divchn = clock() - t_divchn;
          t_muloa = clock();
          for (l = 0; l < C_ITER; l++){
            ret_muloa = tsp(&a, rand_start[l],
                            dist_muloa, wt_zero, &tht_muloa,
                            C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
          }
          t_muloa = clock() - t_muloa;
          res *= (ret_def == 0 && ret_divchn == 0 && ret_muloa == 0);
          printf("\t\t\t# edges: %lu\n", TOLU(a.num_es));
          printf("\t\t\t\t%s %s tsp default ht:     %.8f seconds\n"
                 "\t\t\t\t%s %s tsp ht_divchn:      %.8f seconds\n"
                 "\t\t\t\t%s %s tsp ht_muloa:       %.8f seconds\n",
                 C_VT_TYPES[j], C_WT_TYPES[k],
                 (double)t_def / C_ITER / CLOCKS_PER_SEC,
                 C_VT_TYPES[j], C_WT_TYPES[k],
                 (double)t_divchn / C_ITER / CLOCKS_PER_SEC,
                 C_VT_TYPES[j], C_WT_TYPES[k],
                 (double)t_muloa / C_ITER / CLOCKS_PER_SEC);
          printf("\t\t\t\t%s %s default dist:       ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          C_PRINT[k](dist_def);
          printf("\n");
          printf("\t\t\t\t%s %s divchn dist:        ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          C_PRINT[k](dist_divchn);
          printf("\n");
          printf("\t\t\t\t%s %s muloa dist:         ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          C_PRINT[k](dist_muloa);
          printf("\n");
          printf("\t\t\t\t%s %s correctness:        ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          print_test_result(res);
          printf("\n");
          fflush(stdout);
          ret_def = -1;
          ret_divchn = -1;
          ret_muloa = -1;
          res = 1;
          adj_lst_free(&a);
        }
      }
    }
  }
  free(rand_start);
  free(wt_l);
  rand_start = NULL;
  wt_l = NULL;
  wt_h = NULL;
  wt_zero = NULL;
  wt_one = NULL;
  dist_def = NULL;
  dist_divchn = NULL;
  dist_muloa = NULL;
}

/**
   Portable random number generation. For better uniformity (according
   to rand) RAND_MAX should be 32767, a power of two minus one, or many
   times larger than 32768 on a given system if it is not a power of two
   minus one.

   Given a value n of one of the below unsigned integral types,
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

   It is guaranteed that:
   1 == sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
*/

unsigned char random_uchar(){
  size_t i;
  unsigned char ret = 0;
  for (i = 0; i <= C_UCHAR_BIT_MOD; i++){
    ret |= ((unsigned char)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
            (i * C_RANDOM_BIT));
  }
  return ret;
}

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

unsigned char mul_high_uchar(unsigned char a, unsigned char b){
  unsigned char al, bl, ah, bh, al_bh, ah_bl;
  unsigned char overlap;
  al = a & C_UCHAR_LOW_MASK;
  bl = b & C_UCHAR_LOW_MASK;
  ah = a >> C_UCHAR_HALF_BIT;
  bh = b >> C_UCHAR_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_UCHAR_LOW_MASK) +
             (al_bh & C_UCHAR_LOW_MASK) +
             (al * bl >> C_UCHAR_HALF_BIT));
  return ((overlap >> C_UCHAR_HALF_BIT) +
          ah * bh +
          (ah_bl >> C_UCHAR_HALF_BIT) +
          (al_bh >> C_UCHAR_HALF_BIT));
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
   Value initiliazation and printing.
*/

void set_zero_uchar(void *a){
  *(unsigned char *)a = 0u;
}
void set_zero_ushort(void *a){
  *(unsigned short *)a = 0u;
}
void set_zero_uint(void *a){
  *(unsigned int *)a = 0u;
}
void set_zero_ulong(void *a){
  *(unsigned long *)a = 0u;
}
void set_zero_sz(void *a){
  *(size_t *)a = 0u;
}
void set_zero_double(void *a){
  *(double *)a = 0.0;
}

void set_one_uchar(void *a){
  *(unsigned char *)a = 1u;
}
void set_one_ushort(void *a){
  *(unsigned short *)a = 1u;
}
void set_one_uint(void *a){
  *(unsigned int *)a = 1u;
}
void set_one_ulong(void *a){
  *(unsigned long *)a = 1u;
}
void set_one_sz(void *a){
  *(size_t *)a = 1u;
}
void set_one_double(void *a){
  *(double *)a = 1.0;
}

/* set the weight upper bound; num_vts > 0 */

void set_high_uchar(void *a, size_t num_vts){
  *(unsigned char *)a = C_UCHAR_ULIMIT / num_vts;
}
void set_high_ushort(void *a, size_t num_vts){
  *(unsigned short *)a = C_USHORT_ULIMIT / num_vts;
}
void set_high_uint(void *a, size_t num_vts){
  *(unsigned int *)a = C_UINT_ULIMIT / num_vts;
}
void set_high_ulong(void *a, size_t num_vts){
  *(unsigned long *)a = C_ULONG_ULIMIT / num_vts;
}
void set_high_sz(void *a, size_t num_vts){
  *(size_t *)a = C_SZ_ULIMIT / num_vts;
}
void set_high_double(void *a, size_t high){
  *(double *)a = high;
}

void print_uchar(const void *a){
  printf("%u", *(const unsigned char *)a);
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
   Prints an adjacency list. If the graph is unweighted, then print_wt is
   NULL.
*/
void print_adj_lst(const struct adj_lst *a,
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

/**
   Computes a pointer to the ith element in the block of elements.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

int main(int argc, char *argv[]){
   int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_ULIMIT){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_ULIMIT - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_ULIMIT - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] < 1 ||
      args[0] > C_SZ_BIT - 1 ||
      args[1] < 1 ||
      args[1] > C_SZ_BIT - 1 ||
      args[2] < 1 ||
      args[2] > C_SZ_BIT - 1 ||
      args[3] < 1 ||
      args[3] > C_SZ_BIT - 1 ||
      args[4] < 1 ||
      args[5] < 1 ||
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
  if (args[6]) run_small_graph_test();
  if (args[7]) run_rand_graph_test(args[0], args[1]);
  /*if (args[8]) run_def_rand_uint_test(args[2], args[3]);
  if (args[9]) run_sparse_rand_uint_test(args[4], args[5]);*/
  free(args);
  args = NULL;
  return 0;
}
