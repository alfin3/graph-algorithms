/**
   dfs-test-perf-ushort.c

   Performance test of the DFS algorithm across graphs with only unsigned
   short vertices.

   The following command line arguments can be used to customize tests:
   dfs-test-perf-ushort
     [0, ushort width - 1) : a
     [0, ushort width - 1) : b s.t. 2**a <= V <= 2**b or rand graph test

   usage examples: 
   ./dfs-test-perf-ushort
   ./dfs-test-perf-ushort 10 14

   dfs-test-perf-ushort can be run with any subset of command line arguments
   in the above-defined order. If the (i + 1)th argument is specified then
   the ith argument must be specified for i >= 0. Default values are used
   for the unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99. The tests require that:
   - size_t and clock_t are convertible to double,
   - size_t can represent values upto 65535 for default values, and upto
     USHRT_MAX (>= 65535) otherwise,
   - the widths of the unsigned intergral types are less than 2040 and even.
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
  "dfs-test-perf-ushort\n"
  "[0, ushort width - 1) : a\n"
  "[0, ushort width - 1) : b s.t. 2**a <= V <= 2**b for rand graph test\n";
const int C_ARGC_MAX = 3;
const size_t C_ARGS_DEF[2] = {14u, 14u};
const size_t C_USHORT_BIT = UINT_WIDTH_FROM_MAX((unsigned short)-1);

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
int (* const C_CMPEQ[4])(const void *, const void *) ={
  graph_cmpeq_ushort,
  graph_cmpeq_uint,
  graph_cmpeq_ulong,
  graph_cmpeq_sz};
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

/**
   Run a dfs test on random directed graphs.
*/

struct bern_arg{
  double p;
};

int bern(void *arg){
  struct bern_arg *b = arg;
  if (b->p >= C_PROB_ONE) return 1;
  if (b->p <= C_PROB_ZERO) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
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
				 struct bern_arg *b);

void run_random_dir_graph_test(size_t log_start, size_t log_end){
  size_t i, j;
  size_t num_vts;
  struct bern_arg b;
  printf("Run a dfs test on random directed graphs from %lu random "
	 "start vertices in each graph\n", TOLU(C_ITER));
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
				  C_CMPEQ[0],
				  C_INCR[0],
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
				 struct bern_arg *b){
  size_t i;
  size_t *start = NULL;
  void *pre = NULL, *post = NULL;
  struct graph g;
  struct adj_lst a;
  clock_t t;
  /* no declared type after malloc; effective type is set by dfs */
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
	 type_string, (double)t / C_ITER / CLOCKS_PER_SEC);
  adj_lst_free(&a); /* deallocates blocks with effective vertex type */
  free(start);
  free(pre);
  free(post);
  start = NULL;
  pre = NULL;
  post = NULL;
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
      args[1] < args[0]){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  run_random_dir_graph_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
