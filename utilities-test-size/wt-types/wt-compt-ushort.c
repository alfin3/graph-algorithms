#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

unsigned short random_ushort(){
  size_t i;
  unsigned short ret = 0;
  for (i = 0; i <= C_USHORT_BIT_MOD; i++){
    ret |= ((unsigned short)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
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

void add_undir_ushort_edge(struct adj_lst *a,
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
  adj_lst_add_undir_edge(a, u, v, &rand_val, write_vt, bern, arg);
}

void set_zero_ushort(void *a){
  *(unsigned short *)a = 0u;
}

void set_one_ushort(void *a){
  *(unsigned short *)a = 1u;
}

void set_high_ushort(void *a, size_t num_vts){
  *(unsigned short *)a = C_USHORT_ULIMIT / num_vts;
}

void set_test_ulimit_ushort(void *a, size_t num_vts){
  if (num_vts == 0){
    *(unsigned short *)a = C_USHORT_ULIMIT;
  }else{
    /* usual arithmetic conversions */
    *(unsigned short *)a = C_USHORT_ULIMIT / num_vts;
  }
}

void print_ushort(const void *a){
  printf("%hu", *(const unsigned short *)a);
}
