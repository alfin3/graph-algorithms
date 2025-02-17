/* Second part, following type includes.*/

const size_t C_ITER = 3u;
const size_t C_PROBS_COUNT = 3u;
const double C_PROBS[3] = {1.0000, 0.2500, 0.0000};
const double C_PROB_ONE = 1.0;

const char *C_USAGE =
  "tsp-test \n"
  "[1, size_t width) : a\n"
  "[1, size_t width) : b s.t. a <= |V| <= b for hash table test\n";
const int C_ARGC_ULIMIT = 3;
const size_t C_ARGS_DEF[2] = {10u, 11u};

const size_t C_ALPHA_N_DIVCHN = 1u;
const size_t C_LOG_ALPHA_D_DIVCHN = 0u;

size_t random_sz();
size_t mul_high_sz(size_t a, size_t b);
void print_test_result(int res);
void *ptr(const void *block, size_t i, size_t size);

struct bern_arg{
  double p;
};
int bern(void *arg);
void adj_lst_rand_dir_wts(const struct graph *g,
                          struct adj_lst *a,
                          const void *wt_l,
                          const void *wt_h,
                          const void *wt_one,
                          void (*write_vt)(void *, size_t),
                          int (*bern)(void *),
                          void *arg,
                          void (*add_dir_edge)(struct adj_lst *,
                                               size_t,
                                               size_t,
                                               const void *,
                                               const void *,
                                               void (*)(void *, size_t),
                                               int (*)(void *),
                                               void *));

/**
   Tests tsp on random directed graphs with random non-tour weights and a
   known tour, across edge weight types, vertex types, as well as hash tables.
*/
void run_rand_graph_test(size_t num_start, size_t num_end){
  int res = 1;
  int ret_divchn = -1;
  size_t p, i, j, k, l;
  size_t num_vts;
  size_t vt_size;
  size_t wt_size;
  size_t *rand_start = NULL;
  void *wt_l = NULL, *wt_h = NULL;
  void *wt_zero = NULL, *wt_one = NULL;
  void *dist_divchn = NULL;
  struct graph g;
  struct adj_lst a;
  struct bern_arg b;
  struct ht_divchn ht_divchn;
  struct tsp_ht tht_divchn;
  clock_t t_divchn;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  tht_divchn.ht = &ht_divchn;
  tht_divchn.alpha_n = C_ALPHA_N_DIVCHN;
  tht_divchn.log_alpha_d = C_LOG_ALPHA_D_DIVCHN;
  tht_divchn.init = ht_divchn_init_helper;
  tht_divchn.align = ht_divchn_align_helper;
  tht_divchn.insert = ht_divchn_insert_helper;
  tht_divchn.search = ht_divchn_search_helper;
  tht_divchn.remove = ht_divchn_remove_helper;
  tht_divchn.free = ht_divchn_free_helper;
  printf("Run a tsp test on random directed graphs with existing tours "
         "across vertex and weight types;\nthe runtime is averaged"
         " over %lu runs from random start vertices\n", TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.4f\n", C_PROBS[p]);
    for (i = num_start; i <= num_end; i++){
      num_vts = i;
      printf("\t\t# vertices: %lu\n", TOLU(num_vts));
      for (j = 0; j < C_FN_VT_COUNT; j++){
        for (k = 0; k < C_FN_WT_COUNT; k++){
          vt_size =  C_VT_SIZES[j];
          wt_size =  C_WT_SIZES[k];
          /* no declared type after realloc; new eff. type to be acquired */
          wt_l = realloc_perror(wt_l, 5, wt_size);
          wt_h = ptr(wt_l, 1, wt_size);
          wt_zero = ptr(wt_l, 2, wt_size);
          wt_one = ptr(wt_l, 3, wt_size);
          dist_divchn = ptr(wt_l, 4, wt_size);
          C_SET_ONE[k](wt_l);
          C_SET_HIGH[k](wt_h, num_vts);
          if (C_CMP_WT[k](wt_l, wt_h) > 0){
            memcpy(wt_h, wt_l, wt_size);
          }
          C_SET_ZERO[k](wt_zero);
          C_SET_ONE[k](wt_one);
          /* avoid trap representations in tests */
          C_SET_ZERO[k](dist_divchn);
          graph_base_init(&g, num_vts, vt_size, wt_size);
          adj_lst_rand_dir_wts(&g, &a, wt_l, wt_h, wt_one,
                               C_WRITE_VT[j], bern, &b, C_ADD_DIR_EDGE[k]);
          for (l = 0; l < C_ITER; l++){
            rand_start[l] = mul_high_sz(random_sz(), num_vts);
          }
          t_divchn = clock();
          for (l = 0; l < C_ITER; l++){
            ret_divchn = tsp(&a, rand_start[l],
                             dist_divchn, wt_zero, &tht_divchn,
                             C_READ_VT[j], C_CMP_WT[k], C_ADD_WT[k]);
          }
          t_divchn = clock() - t_divchn;
          res *= (ret_divchn == 0);
          printf("\t\t\t# edges: %lu\n", TOLU(a.num_es));
          printf("\t\t\t\t%s %s tsp ht_divchn:      %.8f seconds\n",
                 C_VT_TYPES[j], C_WT_TYPES[k],
                 (double)t_divchn / C_ITER / CLOCKS_PER_SEC);
          printf("\t\t\t\t%s %s divchn dist:        ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          C_PRINT[k](dist_divchn);
          printf("\n");
          printf("\t\t\t\t%s %s correctness:        ",
                 C_VT_TYPES[j], C_WT_TYPES[k]);
          print_test_result(res);
          printf("\n");
          fflush(stdout);
          ret_divchn = -1;
          res = 1;
          adj_lst_free(&a);
        }
      }
    }
  }
  free(rand_start);
  free(wt_l);
  rand_start = NULL;
  wt_l = NULL;
  wt_h = NULL;
  wt_zero = NULL;
  wt_one = NULL;
  dist_divchn = NULL;
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

int bern(void *arg){
  struct bern_arg *b = arg;
  if (b->p >= 1.0) return 1;
  if (b->p <= 0.0) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
}

void adj_lst_rand_dir_wts(const struct graph *g,
                          struct adj_lst *a,
                          const void *wt_l,
                          const void *wt_h,
                          const void *wt_one,
                          void (*write_vt)(void *, size_t),
                          int (*bern)(void *),
                          void *arg,
                          void (*add_dir_edge)(struct adj_lst *,
                                               size_t,
                                               size_t,
                                               const void *,
                                               const void *,
                                               void (*)(void *, size_t),
                                               int (*)(void *),
                                               void *)){
  size_t i, j;
  struct bern_arg ba;
  adj_lst_base_init(a, g);
  ba.p = C_PROB_ONE;
  for (i = 0; i < a->num_vts - 1; i++){
    for (j = i + 1; j < a->num_vts; j++){
      if (a->num_vts == 2){
        add_dir_edge(a, i, j, wt_one, wt_one, write_vt, bern, &ba);
        add_dir_edge(a, j, i, wt_one, wt_one, write_vt, bern, &ba);
      }else if (j - i == 1){
        add_dir_edge(a, i, j, wt_one, wt_one, write_vt, bern, &ba);
        add_dir_edge(a, j, i, wt_l, wt_h, write_vt, bern, arg);
      }else if (i == 0 && j == a->num_vts - 1){
        add_dir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
        add_dir_edge(a, j, i, wt_one, wt_one, write_vt, bern, &ba);
      }else{
        add_dir_edge(a, i, j, wt_l, wt_h, write_vt, bern, arg);
        add_dir_edge(a, j, i, wt_l, wt_h, write_vt, bern, arg);
      }
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
  if (args[0] < 1 ||
      args[0] > C_SZ_BIT - 1 ||
      args[1] < 1 ||
      args[1] > C_SZ_BIT - 1 ||
      args[0] > args[1]){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  run_rand_graph_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
