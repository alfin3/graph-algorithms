/**
   tsp.h

   Declarations of accessible functions for running an exact solution of
   TSP without vertex revisiting on graphs with generic integer vertices
   and generic weights, including negative weights, and a hash table
   parameter.

   The algorithm provides O(2^n n^2) assymptotic runtime, where n is the
   number of vertices in a tour, as well as tour existence detection. A bit
   array representation provides time and space efficient set membership and
   union operations over O(2^n) sets.

   The hash table parameter specifies a hash table used for set hashing
   operations, and enables the optimization of the associated space and time
   resources by choice of a hash table and its load factor upper bound.
   If NULL is passed as a hash table parameter value, a default hash table
   is used, which contains an array with a count that is equal to n * 2^n,
   where n is the number of vertices in the graph.

   If E >> V and V < width of size_t, a default hash table may provide speed
   advantages by avoiding the computation of hash values. A non-default hash
   table may provide space advantages. A non-default hash may also enable
   computation with V that would exceed the available memory resources with
   the default hash table.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation
   provides an error message and an exit is executed if an integer
   overflow is attempted or an allocation is not completed due to
   insufficient resources. The behavior outside the specified parameter
   ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99.
*/

#ifndef TSP_H
#define TSP_H

#include <stddef.h>
#include "graph.h"

/**
   TSP hash table parameter struct. The function pointers point to the
   hash table op helpers, pre-defined in each hash table. ht points
   to a preallocated hash table struct.
*/
struct tsp_ht{
  void *ht;
  size_t alpha_n;
  size_t log_alpha_d;
  void (*init)(void *,
               size_t,
               size_t,
               size_t,
               size_t,
               size_t,
               int (*)(const void *, const void *),
               size_t (*)(const void *),
               void (*)(void *),
               void (*)(void *));
  void (*align)(void *, size_t);
  void (*insert)(void *, const void *, const void *);
  void *(*search)(const void *, const void *);
  void (*remove)(void *, const void *, void *);
  void (*free)(void *);
};

/**
   Copies to the block pointed to by dist the shortest tour length from
   start to start across all vertices without revisiting, if a tour exists.
   Returns 0 if a tour exists, otherwise returns 1.
   a           : pointer to an adjacency list with at least one vertex
   start       : start vertex for running the algorithm
   dist        : pointer to a preallocated block of size wt_size (wt_size
                 block) that equals to the size of a weight in the adjacency
                 list; if the block pointed to by dist has no declared type,
                 then tsp sets the effective type of the block to the type
                 of a weight in the adjacency list by writing a value of the
                 type; if tsp returns 1, then dist value is set to the value
                 pointed to by zero_wt
   zero_wt     : pointer to a block of size wt_size with a zero value of
                 the type used to represent a distance
   tht         : - NULL pointer, if a default hash table is used for
                 set hashing operations; a default hash table contains an
                 array with a count that is equal to n * 2^n, where n is the
                 number of vertices in the adjacency list; the maximal n
                 in a default hash table is system-dependent and is less
                 than the width of size_t; if the allocation of a default
                 hash table fails for a given adjacency list, the program
                 terminates with an error message
                 - a pointer to a set of parameters specifying a hash table
                 used for set hashing operations
   read_vt     : reads the integer value of the type used to represent
                 vertices from the vt_size block pointed to by the argument
                 and returns a size_t value; unsigned char provides an often
                 sufficient and cache-efficient representation for vertices
   cmp_wt      : comparison function which returns a negative integer value
                 if the weight value pointed to by the first argument is
                 less than the weight value pointed to by the second, a
                 positive integer value if the weight value pointed to by
                 the first argument is greater than the weight value
                 pointed to by the second, and zero integer value if the two
                 weight values are equal
   add_wt      : addition function which copies the sum of the weight values
                 pointed to by the second and third arguments to the
                 preallocated wt_size block pointed to by the first argument;
                 if the distribution of weights can result in an overflow,
                 the user may include an overflow test in the function or
                 use a provided _perror-suffixed function
*/
int tsp(const struct adj_lst *a,
        size_t start,
        void *dist,
        const void *wt_zero,
        const struct tsp_ht *tht,
        size_t (*read_vt)(const void *),
        int (*cmp_wt)(const void *, const void *),
        void (*add_wt)(void *, const void *, const void *));
#endif
