/**
   dijkstra-main.c

   Examples of running Dijkstra's algorithm on a graph with generic 
   non-negative weights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "dijkstra.h"
#include "bfs.h"
#include "heap.h"
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

int cmp_uint64_arrs(const uint64_t *a, const uint64_t *b, uint64_t n);
void norm_uint64_arr(uint64_t *a, uint64_t norm, uint64_t n);
void print_test_result(int res);

/** 
    Graphs with uint64_t weights.
*/

/**
   Initialize graphs with uint64_t weights.
*/
void graph_uint64_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  uint64_t wts[] = {4, 3, 2, 1};
  graph_base_init(g, 5, sizeof(uint64_t));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((uint64_t *)g->wts + i) = wts[i];
  }
}

void graph_uint64_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(uint64_t));
}

/**
   Printing helper functions.
*/
void print_uint64_elts(stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%lu ", *((uint64_t *)s->elts + i));
  }
  printf("\n");
}

void print_double_elts(stack_t *s){
  for (uint64_t i = 0; i < s->num_elts; i++){
    printf("%.2lf ", *((double *)s->elts + i));
  }
  printf("\n");
}
  
void print_adj_lst(adj_lst_t *a,
		   void (*print_wts_fn)(stack_t *)){
  printf("\tvertices: \n");
  for (uint64_t i = 0; i < a->num_vts; i++){
    printf("\t%lu : ", i);
    print_uint64_elts(a->vts[i]);
  }
  if (print_wts_fn != NULL){
    printf("\tweights: \n");
    for (uint64_t i = 0; i < a->num_vts; i++){
      printf("\t%lu : ", i);
      print_wts_fn(a->wts[i]);
    }
  }
  printf("\n");
}

void print_uint64_arr(uint64_t *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (arr[i] == NR){
      printf("NR ");
    }else{
      printf("%lu ", arr[i]);
    }
  }
  printf("\n");
} 

void print_double_arr(double *arr, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    printf("%.2lf ", arr[i]);
  }
  printf("\n");
}

/**
   Run a test on graphs with uint64_t weights.
*/

void add_uint64(void *sum, const void *wt_a, const void *wt_b){
  *(uint64_t *)sum = *(uint64_t *)wt_a + *(uint64_t *)wt_b;
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

typedef struct{
  float alpha;
} context_t;

void ht_div_uint64_init_helper(ht_div_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       int (*cmp_key)(const void *, const void *),
			       void (*free_elt)(void *),
			       void *context){
  context_t *c = context;
  ht_div_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     cmp_key,
		     free_elt);
}

void ht_mul_uint64_init_helper(ht_mul_uint64_t *ht,
			       size_t key_size,
			       size_t elt_size,
			       int (*cmp_key)(const void *, const void *),
			       void (*free_elt)(void *),
			       void *context){
  context_t * c = context;
  ht_mul_uint64_init(ht,
		     key_size,
		     elt_size,
		     c->alpha,
		     cmp_key,
		     NULL, //always NULL for dijkstra
		     free_elt);
}

void run_default_uint64_dijkstra(adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (uint64_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_uint64, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_uint64_dijkstra(adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 1.0;
  context_t context;
  heap_ht_t ht;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  ht.size = sizeof(ht_div_uint64_t);
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &ht, add_uint64, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_uint64_dijkstra(adj_lst_t *a){
  uint64_t *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 0.4;
  context_t context;
  heap_ht_t ht;
  dist = malloc_perror(a->num_vts * sizeof(uint64_t));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  ht.size = sizeof(ht_mul_uint64_t);
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  for (uint64_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &ht, add_uint64, cmp_uint64);
    printf("distances and previous vertices with %lu as start \n", i);
    print_uint64_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}
  
void run_uint64_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_uint64_wts_init(&g);
  printf("Running a test on a directed uint64_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_dijkstra(&a);
  run_div_uint64_dijkstra(&a);
  run_mul_uint64_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected uint64_t graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_dijkstra(&a);
  run_div_uint64_dijkstra(&a);
  run_mul_uint64_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_uint64_wts_no_edges_init(&g);
  printf("Running a test on a directed uint64_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_dijkstra(&a);
  run_div_uint64_dijkstra(&a);
  run_mul_uint64_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected uint64_t graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_uint64_elts);
  run_default_uint64_dijkstra(&a);
  run_div_uint64_dijkstra(&a);
  run_mul_uint64_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
    Graphs with double weights.
*/

/**
   Initialize graphs with double weights.
*/
void graph_double_wts_init(graph_t *g){
  uint64_t u[] = {0, 0, 0, 1};
  uint64_t v[] = {1, 2, 3, 3};
  double wts[] = {4.0, 3.0, 2.0, 1.0};
  graph_base_init(g, 5, sizeof(double));
  g->num_es = 4;
  g->u = malloc_perror(g->num_es * sizeof(uint64_t));
  g->v = malloc_perror(g->num_es * sizeof(uint64_t));
  g->wts = malloc_perror(g->num_es * g->wt_size);
  for (uint64_t i = 0; i < g->num_es; i++){
    g->u[i] = u[i];
    g->v[i] = v[i];
    *((double *)g->wts + i) = wts[i];
  }
}

void graph_double_wts_no_edges_init(graph_t *g){
  graph_base_init(g, 5, sizeof(double));
}

/**
   Run a test on graphs with double weights.
*/
void add_double(void *sum, const void *wt_a, const void *wt_b){
  *(double *)sum = *(double *)wt_a + *(double *)wt_b;
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

void run_default_double_dijkstra(adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  for (size_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, NULL, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_div_double_dijkstra(adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 1.0;
  context_t context;
  heap_ht_t ht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  ht.size = sizeof(ht_div_uint64_t);
  ht.context = &context;
  ht.init = (heap_ht_init)ht_div_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_div_uint64_insert;
  ht.search = (heap_ht_search)ht_div_uint64_search;
  ht.remove = (heap_ht_remove)ht_div_uint64_remove;
  ht.free = (heap_ht_free)ht_div_uint64_free;
  for (size_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &ht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_mul_double_dijkstra(adj_lst_t *a){
  double *dist = NULL;
  uint64_t *prev = NULL;
  float alpha = 0.4;
  context_t context;
  heap_ht_t ht;
  dist = malloc_perror(a->num_vts * sizeof(double));
  prev = malloc_perror(a->num_vts * sizeof(uint64_t));
  context.alpha = alpha;
  ht.size = sizeof(ht_mul_uint64_t);
  ht.context = &context;
  ht.init = (heap_ht_init)ht_mul_uint64_init_helper;
  ht.insert = (heap_ht_insert)ht_mul_uint64_insert;
  ht.search = (heap_ht_search)ht_mul_uint64_search;
  ht.remove = (heap_ht_remove)ht_mul_uint64_remove;
  ht.free = (heap_ht_free)ht_mul_uint64_free;
  for (size_t i = 0; i < a->num_vts; i++){
    dijkstra(a, i, dist, prev, &ht, add_double, cmp_double);
    printf("distances and previous vertices with %lu as start \n", i);
    print_double_arr(dist, a->num_vts);
    print_uint64_arr(prev, a->num_vts);
  }
  printf("\n");
  free(dist);
  free(prev);
  dist = NULL;
  prev = NULL;
}

void run_double_graph_test(){
  graph_t g;
  adj_lst_t a;
  graph_double_wts_init(&g);
  printf("Running a test on a directed double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on an undirected double graph with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
  graph_double_wts_no_edges_init(&g);
  printf("Running a test on a directed double graph with no edges, with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_dir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  printf("Running a test on a undirected double graph with no edges, "
	 "with a \n"
	 "i) default hash table (index array) \n"
	 "ii) ht_div_uint64_t hash table \n"
	 "iii) ht_mul_uint64_t hash table \n\n");
  adj_lst_init(&a, &g);
  adj_lst_undir_build(&a, &g);
  print_adj_lst(&a, print_double_elts);
  run_default_double_dijkstra(&a);
  run_div_double_dijkstra(&a);
  run_mul_double_dijkstra(&a);
  adj_lst_free(&a);
  graph_free(&g);
}

/**
   Returns O if two uint64_t arrays are equal.
*/
int cmp_uint64_arrs(const uint64_t *a, const uint64_t *b, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    if (a[i] > b[i]){return 1;}
    if (a[i] < b[i]){return -1;}
  }
  return 0;
}

/**
   Normalizes a uint64_t array.
*/
void norm_uint64_arr(uint64_t *a, uint64_t norm, uint64_t n){
  for (uint64_t i = 0; i < n; i++){
    a[i] = a[i] / norm;
  }
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  run_uint64_graph_test();
  run_double_graph_test();
  //run_bfs_dijkstra_graph_test();
  //run_rand_uint64_wts_graph_test();
  return 0;
}
