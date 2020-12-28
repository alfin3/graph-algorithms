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
		       int (*cmp_elt_fn)(const void *, const void *));

#endif
