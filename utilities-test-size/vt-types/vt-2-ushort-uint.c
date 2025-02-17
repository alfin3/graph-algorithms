#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

#define FILE_SCOPE_VT_COUNT (2)

const size_t C_FN_VT_COUNT = 2u;

size_t (* const C_READ_VT[FILE_SCOPE_VT_COUNT])(const void *) ={
  graph_read_ushort,
  graph_read_uint};
void (* const C_WRITE_VT[FILE_SCOPE_VT_COUNT])(void *, size_t) ={
  graph_write_ushort,
  graph_write_uint};
void *(* const C_AT_VT[FILE_SCOPE_VT_COUNT])(const void *, const void *) ={
  graph_at_ushort,
  graph_at_uint};
int (* const C_CMP_VT[FILE_SCOPE_VT_COUNT])(const void *, const void *) ={
  graph_cmpeq_ushort,
  graph_cmpeq_uint};
void (* const C_INCR_VT[FILE_SCOPE_VT_COUNT])(void *) ={
  graph_incr_ushort,
  graph_incr_uint};
const size_t C_VT_SIZES[FILE_SCOPE_VT_COUNT] ={
  sizeof(unsigned short),
  sizeof(unsigned int)};
const char *C_VT_TYPES[FILE_SCOPE_VT_COUNT] ={
  "ushort",
  "uint  "};
