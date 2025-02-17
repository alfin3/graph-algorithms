#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

int cmp_float(const void *a, const void *b){
  return ((*(const float *)a > *(const float *)b) -
          (*(const float *)a < *(const float *)b));
}
void add_float(void *s, const void *a, const void *b){
  *(float *)s = *(const float *)a + *(const float *)b;
}

void add_dir_float_edge(struct adj_lst *a,
                        size_t u,
                        size_t v,
                        const void *wt_l,
                        const void *wt_h,
                        void (*write_vt)(void *, size_t),
                        int (*bern)(void *),
                        void *arg){
  float rand_val =
    *(float *)wt_l +
     (float)DRAND() * (*(float *)wt_h - *(float *)wt_l);
  adj_lst_add_dir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void add_undir_float_edge(struct adj_lst *a,
                          size_t u,
                          size_t v,
                          const void *wt_l,
                          const void *wt_h,
                          void (*write_vt)(void *, size_t),
                          int (*bern)(void *),
                          void *arg){
  float rand_val =
    *(float *)wt_l +
     (float)DRAND() * (*(float *)wt_h - *(float *)wt_l);
  adj_lst_add_undir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void set_zero_float(void *a){
  *(float *)a = 0.0f;
}

void set_one_float(void *a){
  *(float *)a = 1.0f;
}

void set_high_float(void *a, size_t high){
  *(float *)a = high;
}

void set_test_ulimit_float(void *a, size_t num_vts){
  *(float *)a = 1.0f / num_vts;
}

void print_float(const void *a){
  printf("%.8f", *(const float *)a);
}

