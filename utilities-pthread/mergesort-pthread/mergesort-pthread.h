/**
   mergesort-pthread.h

   Declarations of accessible functions and macro definitions for optimizing
   and running a generic merge sort algorithm with decoupled parallel sorting
   and parallel merging during recursion.

   The design decouples merge and sort parallelisms in mergesort. The
   algorithm provides \Theta(n/log^{2}n) theoretical parallelism within the
   dynamic multithreading model.

   The implementation provides i) a set of parameters for setting the
   constant base case upper bounds for switching from parallel sorting to
   serial sorting and from parallel merging to serial merging during
   recursion, and ii) a macro for setting the constant upper bound for the
   number of recursive calls placed on the stack of a thread across sorting
   and merging operations, thereby enabling the optimization of the
   parallelism and concurrency-associated overhead across input ranges
   and hardware settings. The provided parametrization of multithreading
   is based on the common parametrization of serial mergesort, where the
   recursion depth is limited by switching to a base case non-recursive
   sort algorithm.

   The implementation only uses integer and pointer operations. Given
   parameter values within the specified ranges, the implementation provides
   an error message and an exit is executed if an integer overflow is
   attempted or an allocation is not completed due to insufficient resources.
   The behavior outside the specified parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99. The requirement is that pthreads API is available.

   On a machine with 24 logical cores, with a 13th Gen Intel(R) Core(TM)
   i7-13700HX Processor with Hyper-Threading on 8 of 16 physical cores,
   the initial optimization of the base case upper bound parameters
   resulted in speedups of approximately 11.22X and 9.67X in comparison
   to serial qsort (stdlib.h) on arrays of 2**28 random integer and double
   elements respectively.
*/


#ifndef MERGESORT_PTHREAD_H
#define MERGESORT_PTHREAD_H

#include <stddef.h>

/**
   Sorts a given array pointed to by elts in ascending order according to
   cmp. The array contains count elements of elt_size bytes. The first
   thread entry is placed on the thread stack of the caller.
   elts        : pointer to the array to sort
   count       : > 0, < 2**{size_t width - 1} count of elements in the array
   elt_size    : size of each element in the array in bytes
   sbase_count : > 0 base case upper bound for parallel sorting; if the count
                 of an unsorted subarray is less or equal to sbase_count,
                 then the subarray is sorted with serial sort routine (qsort)
   mbase_count : > 1 base case upper bound for parallel merging; if the sum
                 of the counts of two sorted subarrays is less or equal to
                 mbase_count, then the two subarrays are merged with a serial
                 merge routine
   cmp         : comparison function which returns a negative integer value
                 if the element pointed to by the first argument is less than
                 the element pointed to by the second, a positive integer
                 value if the element pointed to by the first argument is
                 greater than the element pointed to by the second, and zero
                 integer value if the two elements are equal
*/
void mergesort_pthread(void *elts,
                       size_t count,
                       size_t elt_size,
                       size_t sbase_count,
                       size_t mbase_count,
                       int (*cmp)(const void *, const void *));

/**
   A constant upper bound for the number of recursive calls of thread entry
   functions placed on the stack of a thread. Reduces the total number of
   threads and provides an additional speedup if greater than 0. If equal to
   0, then each recursive call results in the creation of a new thread. The
   macro is used as size_t.
*/
#define MERGESORT_PTHREAD_MAX_ONTHREAD_REC (20)

#endif
