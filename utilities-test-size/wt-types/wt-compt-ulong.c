#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

const size_t C_ULONG_BIT = PRECISION_FROM_ULIMIT(ULONG_MAX);
const size_t C_ULONG_BIT_MOD = PRECISION_FROM_ULIMIT(ULONG_MAX) / 15u;
const size_t C_ULONG_HALF_BIT = PRECISION_FROM_ULIMIT(ULONG_MAX) / 2u;
const unsigned long C_ULONG_ULIMIT = ULONG_MAX;
const unsigned long C_ULONG_LOW_MASK =
  ((unsigned long)-1 >> (PRECISION_FROM_ULIMIT(ULONG_MAX) / 2u));

unsigned long random_ulong(){
  size_t i;
  unsigned long ret = 0;
  for (i = 0; i <= C_ULONG_BIT_MOD; i++){
    ret |= ((unsigned long)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
            (i * C_RANDOM_BIT));
  }
  return ret;
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

void add_undir_ulong_edge(struct adj_lst *a,
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
  adj_lst_add_undir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void set_zero_ulong(void *a){
  *(unsigned long *)a = 0u;
}

void set_one_ulong(void *a){
  *(unsigned long *)a = 1u;
}

void set_high_ulong(void *a, size_t num_vts){
  *(unsigned long *)a = C_ULONG_ULIMIT / num_vts;
}

void set_test_ulimit_ulong(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned long *)a = C_ULONG_ULIMIT;
  }else{
    /* usual arithmetic conversions */
    *(unsigned long *)a = C_ULONG_ULIMIT / num_vts;
  }
}

void print_ulong(const void *a){
  printf("%lu", *(const unsigned long *)a);
}
