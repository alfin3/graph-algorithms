/**
   mergesort-mthread-uint64.h

   
*/

#ifndef MERGESORT_MTHREAD_UINT64_H  
#define  MERGESORT_MTHREAD_UINT64_H

#include <stdint.h>

void mergesort_mthread_uint64(void *elts,
			      uint64_t count,
			      uint64_t sbase_count,
			      int elt_size,
			      int (*cmp_elt_fn)(const void *, const void *));

#endif
