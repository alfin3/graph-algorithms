#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"
#include "utilities-lim.h"

/* Type-related code composition.*/

#include "wt-compt-ushort.c"
#include "wt-compt-uint.c"
#include "wt-compt-float.c"
#include "wt-compt-double.c"

#define FILE_SCOPE_WT_COUNT (4)

const size_t C_FN_WT_COUNT = 4u;

int (* const C_CMP_WT[FILE_SCOPE_WT_COUNT])(const void *, const void *) ={
  graph_cmp_ushort,
  graph_cmp_uint,
  cmp_float,
  cmp_double};
void (* const C_ADD_WT[FILE_SCOPE_WT_COUNT])(void *, const void *, const void *) ={
  graph_add_ushort,
  graph_add_uint,
  add_float,
  add_double};
const size_t C_WT_SIZES[FILE_SCOPE_WT_COUNT] ={
  sizeof(unsigned short),
  sizeof(unsigned int),
  sizeof(float),
  sizeof(double)};
const char *C_WT_TYPES[FILE_SCOPE_WT_COUNT] ={
  "ushort",
  "uint  ",
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
  add_dir_ushort_edge,
  add_dir_uint_edge,
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
  add_undir_ushort_edge,
  add_undir_uint_edge,
  add_undir_float_edge,
  add_undir_double_edge};
void (* const C_SET_ZERO[FILE_SCOPE_WT_COUNT])(void *) ={
  set_zero_ushort,
  set_zero_uint,
  set_zero_float,
  set_zero_double};
void (* const C_SET_ONE[FILE_SCOPE_WT_COUNT])(void *) ={
  set_one_ushort,
  set_one_uint,
  set_one_float,
  set_one_double};
void (* const C_SET_HIGH[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_high_ushort,
  set_high_uint,
  set_high_float,
  set_high_double};
void (* const C_SET_TEST_ULIMIT[FILE_SCOPE_WT_COUNT])(void *, size_t) ={
  set_test_ulimit_ushort,
  set_test_ulimit_uint,
  set_test_ulimit_float,
  set_test_ulimit_double};
void (* const C_PRINT[FILE_SCOPE_WT_COUNT])(const void *) ={
  print_ushort,
  print_uint,
  print_float,
  print_double};
