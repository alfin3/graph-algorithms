#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

/* Type-related code composition.*/

#include "wt-compt-ulong.c"
#include "wt-compt-float.c"
#include "wt-compt-double.c"

#define FILE_SCOPE_WT_COUNT (3)

const size_t C_FN_WT_COUNT = 3u;

int (* const C_CMP_WT[FILE_SCOPE_WT_COUNT])(const void *, const void *) ={
  graph_cmp_ulong,
  cmp_float,
  cmp_double};
void (* const C_ADD_WT[FILE_SCOPE_WT_COUNT])(void *, const void *, const void *) ={
  graph_add_ulong,
  add_float,
  add_double};
const size_t C_WT_SIZES[FILE_SCOPE_WT_COUNT] ={
  sizeof(unsigned long),
  sizeof(float),
  sizeof(double)};
const char *C_WT_TYPES[FILE_SCOPE_WT_COUNT] ={
  "ulong ",
  "float ",
  "double"};

void (* const C_ADD_DIR_EDGE[FILE_SCOPE_WT_COUNT])(struct adj_lst *,
                                                   size_t,
                                                   size_t,
                                                   const void *,
                                                   const void *,
                                                   void (*)(void *, size_t),
                                                   int (*)(void *),
                                                   void *) ={
  add_dir_ulong_edge,
  add_dir_float_edge,
  add_dir_double_edge};
void (* const C_ADD_UNDIR_EDGE[FILE_SCOPE_WT_COUNT])(struct adj_lst *,
                                                     size_t,
                                                     size_t,
                                                     const void *,
                                                     const void *,
                                                     void (*)(void *, size_t),
                                                     int (*)(void *),
                                                     void *) ={
  add_undir_ulong_edge,
  add_undir_float_edge,
  add_undir_double_edge};
void (* const C_SET_ZERO[FILE_SCOPE_WT_COUNT])(void *) ={
  set_zero_ulong,
  set_zero_float,
  set_zero_double};
void (* const C_SET_ONE[FILE_SCOPE_WT_COUNT])(void *) ={
  set_one_ulong,
  set_one_float,
  set_one_double};
void (* const C_SET_HIGH[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_high_ulong,
  set_high_float,
  set_high_double};
void (* const C_SET_TEST_ULIMIT[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_test_ulimit_ulong,
  set_test_ulimit_float,
  set_test_ulimit_double};
void (* const C_PRINT[FILE_SCOPE_WT_COUNT])(const void *) ={
  print_ulong,
  print_float,
  print_double};
