#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

int cmp_double(const void *a, const void *b){
  return ((*(const double *)a > *(const double *)b) -
          (*(const double *)a < *(const double *)b));
}
void add_double(void *s, const void *a, const void *b){
  *(double *)s = *(const double *)a + *(const double *)b;
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

void add_undir_double_edge(struct adj_lst *a,
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
  adj_lst_add_undir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void set_zero_double(void *a){
  *(double *)a = 0.0;
}

void set_one_double(void *a){
  *(double *)a = 1.0;
}

void set_high_double(void *a, size_t high){
  *(double *)a = high;
}

void set_test_ulimit_double(void *a, size_t num_vts){
  *(double *)a = 1.0 / num_vts;
}

void print_double(const void *a){
  printf("%.8f", *(const double *)a);
}

