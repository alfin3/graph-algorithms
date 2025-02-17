/* First part, preceding type includes.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "prim.h"
#include "heap.h"
#include "ht-divchn.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand())
#define DRAND() ((double)rand() / RAND_MAX)
#define TOLU(i) ((unsigned long int)(i))

const size_t C_RANDOM_BIT = 15u;
const unsigned int C_RANDOM_MASK = 32767u;

const size_t C_USHORT_BIT = PRECISION_FROM_ULIMIT(USHRT_MAX);
const size_t C_USHORT_BIT_MOD = PRECISION_FROM_ULIMIT(USHRT_MAX) / 15u;
const size_t C_USHORT_HALF_BIT = PRECISION_FROM_ULIMIT(USHRT_MAX) / 2u;
const unsigned short C_USHORT_ULIMIT = USHRT_MAX;
const unsigned short C_USHORT_LOW_MASK =
  ((unsigned short)-1 >> (PRECISION_FROM_ULIMIT(USHRT_MAX) / 2u));

const size_t C_SZ_BIT = PRECISION_FROM_ULIMIT((size_t)-1);
const size_t C_SZ_BIT_MOD = PRECISION_FROM_ULIMIT((size_t)-1) / 15u;
const size_t C_SZ_HALF_BIT = PRECISION_FROM_ULIMIT((size_t)-1) / 2u;
const size_t C_SZ_ULIMIT = (size_t)-1;
const size_t C_SZ_LOW_MASK =
  ((size_t)-1 >> (PRECISION_FROM_ULIMIT((size_t)-1) / 2u));
