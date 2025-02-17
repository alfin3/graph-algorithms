#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

/* Type-related code composition.*/

#include "wt-compt-ulong.c"

#define FILE_SCOPE_WT_COUNT (1)

const size_t C_FN_WT_COUNT = 1u;

int (* const C_CMP_WT[FILE_SCOPE_WT_COUNT])(const void *, const void *) ={
  graph_cmp_ulong};
void (* const C_ADD_WT[FILE_SCOPE_WT_COUNT])(void *, const void *, const void *) ={
  graph_add_ulong};
const size_t C_WT_SIZES[FILE_SCOPE_WT_COUNT] ={
  sizeof(unsigned long)};
const char *C_WT_TYPES[FILE_SCOPE_WT_COUNT] ={
  "ulong "};

void (* const C_ADD_DIR_EDGE[FILE_SCOPE_WT_COUNT])(struct adj_lst *,
                                                   size_t,
                                                   size_t,
                                                   const void *,
                                                   const void *,
                                                   void (*)(void *, size_t),
                                                   int (*)(void *),
                                                   void *) ={
  add_dir_ulong_edge};
void (* const C_ADD_UNDIR_EDGE[FILE_SCOPE_WT_COUNT])(struct adj_lst *,
                                                     size_t,
                                                     size_t,
                                                     const void *,
                                                     const void *,
                                                     void (*)(void *, size_t),
                                                     int (*)(void *),
                                                     void *) ={
  add_undir_ulong_edge};
void (* const C_SET_ZERO[FILE_SCOPE_WT_COUNT])(void *) ={
  set_zero_ulong};
void (* const C_SET_ONE[FILE_SCOPE_WT_COUNT])(void *) ={
  set_one_ulong};
void (* const C_SET_HIGH[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_high_ulong};
void (* const C_SET_TEST_ULIMIT[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_test_ulimit_ulong};
void (* const C_PRINT[FILE_SCOPE_WT_COUNT])(const void *) ={
  print_ulong};
