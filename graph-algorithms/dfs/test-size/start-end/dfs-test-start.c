/* First part, preceding type includes.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "dfs.h"
#include "graph.h"
#include "stack.h"
#include "utilities-mem.h"
#include "utilities-mod.h"
#include "utilities-lim.h"

#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand())
#define DRAND() ((double)rand() / RAND_MAX)
#define TOLU(i) ((unsigned long int)(i))

const size_t C_USHORT_BIT = PRECISION_FROM_ULIMIT(USHRT_MAX);

