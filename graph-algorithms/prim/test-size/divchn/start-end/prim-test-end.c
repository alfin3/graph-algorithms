/* Second part, following type includes.*/

const size_t C_ITER = 10u;
const size_t C_PROBS_COUNT = 7u;
const double C_PROBS[7] = {1.000000, 0.250000, 0.062500,
                           0.015625, 0.003906, 0.000977,
                           0.000000};

const char *C_USAGE =
  "prim-test \n"
  "[0, ushort width) : n for 2**n vertices in smallest graph\n"
  "[0, ushort width) : n for 2**n vertices in largest graph\n";
const int C_ARGC_ULIMIT = 3;
const size_t C_ARGS_DEF[2] = {6u, 9u};

const size_t C_ALPHA_N_DIVCHN = 1u;
const size_t C_LOG_ALPHA_D_DIVCHN = 0u;

size_t random_sz();
size_t mul_high_sz(size_t a, size_t b);
void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

struct bern_arg{
  double p;
};
int bern(void *arg);
void adj_lst_rand_undir_wts(const struct graph *g,
                            struct adj_lst *a,
                            const void *wt_l,
                            const void *wt_h,
                            void (*write_vt)(void *, size_t),
                            int (*bern)(void *),
                            void *arg,
                            void (*add_undir_edge)(struct adj_lst *,
                                                   size_t,
                                                   size_t,
                                                   const void *,
                                                   const void *,
                                                   void (*)(void *, size_t),
                                                   int (*)(void *),
                                                   void *));

/**
   Run a test on random undirected graphs with random weights, across edge
   weight types, vertex types, as well as hash tables.
*/

void run_rand_test(size_t log_start, size_t log_end){
  size_t p, i, j, k, l;
  size_t num_vts;
  size_t vt_size;
  size_t wt_size;
  size_t *rand_start = NULL;
  void *wt_l = NULL, *wt_h = NULL;
  void *wt_zero = NULL;
  void *dist_divchn = NULL;
  void *prev_divchn = NULL;
  struct graph g;
  struct adj_lst a;
  struct bern_arg b;
  struct ht_divchn ht_divchn;
  struct prim_ht pmht_divchn;
  clock_t t_divchn;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  pmht_divchn.ht = &ht_divchn;
  pmht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  pmht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  pmht_divchn.init = ht_divchn_init_helper;
  pmht_divchn.align = ht_divchn_align_helper;
  pmht_divchn.insert = ht_divchn_insert_helper;
  pmht_divchn.search = ht_divchn_search_helper;
  pmht_divchn.remove = ht_divchn_remove_helper;
  pmht_divchn.free = ht_divchn_free_helper;
  printf("Run a prim test on random undirected graphs with random weights"
         " across vertex and weight types;\nthe runtime is averaged"
         " over %lu runs from random start vertices\n", TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (k = 0; k < C_FN_WT_COUNT; k++){
      wt_size = C_WT_SIZES[k];
      wt_l = realloc_perror(wt_l, 2, wt_size);
      wt_h = ptr(wt_l, 1, wt_size);
      C_SET_ZERO[k](wt_l);
      C_SET_TEST_ULIMIT[k](wt_h, pow_two_perror(log_end));
      printf("\t%s range: [", C_WT_TYPES[k]);
      C_PRINT[k](wt_l);
      printf(", ");
      C_PRINT[k](wt_h);
      printf(")\n");
    }
    for (i = log_start; i <= log_end; i++){
      num_vts = pow_two_perror(i); /* 0 < n */
      printf("\t\t# vertices: %lu\n", TOLU(num_vts));
      for (j = 0; j < C_FN_VT_COUNT; j++){
        for (k = 0; k < C_FN_WT_COUNT; k++){
          vt_size =  C_VT_SIZES[j];
          wt_size =  C_WT_SIZES[k];
          /* no declared type after realloc; new eff. type to be acquired */
          wt_l = realloc_perror(wt_l, 3, wt_size);
          wt_h = ptr(wt_l, 1, wt_size);
          wt_zero = ptr(wt_l, 2, wt_size);
          prev_divchn = realloc_perror(prev_divchn, num_vts, vt_size);
          dist_divchn = realloc_perror(dist_divchn, num_vts, wt_size);
          C_SET_ZERO[k](wt_l);
          C_SET_TEST_ULIMIT[k](wt_h, pow_two_perror(log_end));
          C_SET_ZERO[k](wt_zero);
          for (l = 0; l < num_vts; l++){
            /* avoid trap representations in tests */
            C_SET_ZERO[k](ptr(dist_divchn, l, wt_size));
          }
          graph_base_init(&g, num_vts, vt_size, wt_size);
          adj_lst_rand_undir_wts(&g, &a, wt_l, wt_h, C_WRITE_VT[j],
                                 bern, &b, C_ADD_UNDIR_EDGE[k]);
          for (l = 0; l < C_ITER; l++){
            rand_start[l] = mul_high_sz(random_sz(), num_vts);
          }
          t_divchn = clock();
          for (l = 0; l < C_ITER; l++){
            prim(&a, rand_start[l], dist_divchn, prev_divchn, wt_zero,
                 &pmht_divchn, C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j],
                 C_CMP_VT[j], C_CMP_WT[k]);
          }
          t_divchn = clock() - t_divchn;
          printf("\t\t\t# edges: %lu\n", TOLU(a.num_es));
          printf("\t\t\t\t%s %s prim ht_divchn:          %.8f seconds\n",
                 C_VT_TYPES[j], C_WT_TYPES[k],
                 (double)t_divchn / C_ITER / CLOCKS_PER_SEC);
          printf("\n\n");
          adj_lst_free(&a);
        }
      }
    }
  }
  free(rand_start);
  free(wt_l);
  free(dist_divchn);
  free(prev_divchn);
  rand_start = NULL;
  wt_l = NULL;
  wt_h = NULL;
  wt_zero = NULL;
  dist_divchn = NULL;
  prev_divchn = NULL;
}

void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int bern(void *arg){
  struct bern_arg *b = arg;
  if (b->p >= 1.0) return 1;
  if (b->p <= 0.0) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
}

void adj_lst_rand_undir_wts(const struct graph *g,
                            struct adj_lst *a,
                            const void *wt_l,
                            const void *wt_h,
                            void (*write_vt)(void *, size_t),
                            int (*bern)(void *),
                            void *arg,
                            void (*add_undir_edge)(struct adj_lst *,
                                                   size_t,
                                                   size_t,
                                                   const void *,
                                                   const void *,
                                                   void (*)(void *, size_t),
                                                   int (*)(void *),
                                                   void *)){
  size_t i, j;
  adj_lst_base_init(a, g);
  for (i = 0; i < a->num_vts - 1; i++){
    for (j = i + 1; j < a->num_vts; j++){
      add_undir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
    }
  }
}

size_t random_sz(){
  size_t i;
  size_t ret = 0;
  for (i = 0; i <= C_SZ_BIT_MOD; i++){
    ret |= ((size_t)((unsigned int)RANDOM() & C_RANDOM_MASK) <<
            (i * C_RANDOM_BIT));
  }
  return ret;
}

size_t mul_high_sz(size_t a, size_t b){
  size_t al, bl, ah, bh, al_bh, ah_bl;
  size_t overlap;
  al = a & C_SZ_LOW_MASK;
  bl = b & C_SZ_LOW_MASK;
  ah = a >> C_SZ_HALF_BIT;
  bh = b >> C_SZ_HALF_BIT;
  al_bh = al * bh;
  ah_bl = ah * bl;
  overlap = ((ah_bl & C_SZ_LOW_MASK) +
             (al_bh & C_SZ_LOW_MASK) +
             (al * bl >> C_SZ_HALF_BIT));
  return ((overlap >> C_SZ_HALF_BIT) +
          ah * bh +
          (ah_bl >> C_SZ_HALF_BIT) +
          (al_bh >> C_SZ_HALF_BIT));
}

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_ULIMIT){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_ULIMIT - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_ULIMIT - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_USHORT_BIT - 1 ||
      args[1] > C_USHORT_BIT - 1 ||
      args[1] < args[0]){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  /*pow_two_perror later tests that # vertices is representable as size_t */
  run_rand_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
