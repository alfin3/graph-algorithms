/**
   bfs-test-perf-uint.c

   Performance test of the BFS algorithm across graphs with only unsigned
   int vertices.

   The following command line arguments can be used to customize tests:
   bfs-test-perf-uint
     [0, uint width - 1] : a
     [0, uint width - 1] : b s.t. 2**a <= V <= 2**b or rand graph test

   usage examples: 
   ./bfs-test-perf-uint
   ./bfs-test-perf-uint 10 14

   bfs-test-perf-uint can be run with any subset of command line arguments
   in the above-defined order. If the (i + 1)th argument is specified then
   the ith argument must be specified for i >= 0. Default values are used
   for the unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the requirement that the width of unsigned int
   is greater or equal to 16 and less than 2040. The width of size_t must
   also be even.
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
  "bfs-test-perf-uint\n"
  "[0, uint width - 1] : a\n"
  "[0, uint width - 1] : b s.t. 2**a <= V <= 2**b for rand graph test\n";
const int C_ARGC_MAX = 3;
const size_t C_ARGS_DEF[2] = {14u, 14u};
const size_t C_UINT_BIT = UINT_WIDTH_FROM_MAX((unsigned int)-1);

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
/**
   Run a bfs test on random directed graphs.
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
  printf("Run a bfs test on random directed graphs from %lu random "
	 "start vertices in each graph\n", TOLU(C_ITER));
  for (i = 0; i < C_PROBS_COUNT; i++){
    b.p = C_PROBS[i];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (j = log_start; j <= log_end; j++){
      num_vts = pow_two_perror(j);
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f\n",
	     TOLU(num_vts), b.p * num_vts * (num_vts - 1));
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
  void *dist = NULL, *prev = NULL;
  graph_t g;
  adj_lst_t a;
  clock_t t;
  /* no declared type after malloc; effective type is set by bfs */
  start = malloc_perror(C_ITER, sizeof(size_t));
  dist = malloc_perror(num_vts, vt_size);
  prev = malloc_perror(num_vts, vt_size);
  graph_base_init(&g, num_vts, vt_size, 0);
  adj_lst_base_init(&a, &g);
  adj_lst_rand_dir(&a, write_vt, bern, b);
  for (i = 0; i < C_ITER; i++){
    start[i] =  RANDOM() % num_vts;
  }
  t = clock();
  for (i = 0; i < C_ITER; i++){
    bfs(&a, start[i], dist, prev, read_vt, write_vt, at_vt, cmp_vt, incr_vt);
  }
  t = clock() - t;
  printf("\t\t\t%s ave runtime:     %.6f seconds\n",
	 type_string, (float)t / C_ITER / CLOCKS_PER_SEC);
  adj_lst_free(&a); /* deallocates blocks with effective vertex type */
  free(start);
  free(dist);
  free(prev);
  start = NULL;
  dist = NULL;
  prev = NULL;
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
  if (args[0] > C_UINT_BIT - 1 ||
      args[1] > C_UINT_BIT - 1 ||
      args[1] < args[0]){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  run_random_dir_graph_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
