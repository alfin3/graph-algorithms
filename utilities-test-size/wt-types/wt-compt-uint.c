#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

const size_t C_UINT_BIT = PRECISION_FROM_ULIMIT(UINT_MAX);
const size_t C_UINT_BIT_MOD = PRECISION_FROM_ULIMIT(UINT_MAX) / 15u;
const size_t C_UINT_HALF_BIT = PRECISION_FROM_ULIMIT(UINT_MAX) / 2u;
const unsigned int C_UINT_ULIMIT = UINT_MAX;
const unsigned int C_UINT_LOW_MASK =
  ((unsigned int)-1 >> (PRECISION_FROM_ULIMIT(UINT_MAX) / 2u));

unsigned int random_uint(){
  size_t i;
  unsigned int ret = 0;
  for (i = 0; i <= C_UINT_BIT_MOD; i++){
    ret |= ((unsigned int)RANDOM() & C_RANDOM_MASK) << (i * C_RANDOM_BIT);
  }
  return ret;
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

void add_undir_uint_edge(struct adj_lst *a,
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
  adj_lst_add_undir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void set_zero_uint(void *a){
  *(unsigned int *)a = 0u;
}

void set_one_uint(void *a){
  *(unsigned int *)a = 1u;
}

void set_high_uint(void *a, size_t num_vts){
  *(unsigned int *)a = C_UINT_ULIMIT / num_vts;
}

void set_test_ulimit_uint(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned int *)a = C_UINT_ULIMIT;
  }else{
    /* usual arithmetic conversions */
    *(unsigned int *)a = C_UINT_ULIMIT / num_vts;
  }
}

void print_uint(const void *a){
  printf("%u", *(const unsigned int *)a);
}
