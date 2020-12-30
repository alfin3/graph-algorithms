/**
   mergesort-mthread.h

   
*/

#ifndef MERGESORT_MTHREAD_H  
#define MERGESORT_MTHREAD_H

#include <stdlib.h>

void mergesort_mthread(void *elts,
		       size_t count,
		       size_t elt_size,
		       size_t sbase_count,
		       size_t mbase_count,
		       int (*cmp)(const void *, const void *));

/**
   Reduces the total number of threads by placing O(logn) recursive calls
   of functions used in the implemention of mergesort_mthread on a thread
   stack, with the tightness of the bound set by the macro, providing an
   additional speedup if greater than 0. If equal to 0, then each
   recursive call results in the creation of a new thread. The macro
   is used as size_t.
*/
#define MERGESORT_MTHREAD_MAX_ONTHREAD_REC (20)

#endif
