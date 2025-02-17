/* Second part, following type includes.*/

const size_t C_ITER = 10u;
const size_t C_PROBS_COUNT = 5u;
const double C_PROBS[5] = {1.00, 0.75, 0.50, 0.25, 0.00};
const double C_PROB_ONE = 1.0;
const double C_PROB_ZERO = 0.0;

const char *C_USAGE =
  "dfs-test\n"
  "[0, ushort width - 1) : a\n"
  "[0, ushort width - 1) : b s.t. 2**a <= V <= 2**b for rand graph test\n";
const int C_ARGC_ULIMIT = 3;
const size_t C_ARGS_DEF[2] = {6u, 9u};

void *ptr(const void *block, size_t i, size_t size);
void print_test_result(int res);

struct bern_arg{
  double p;
};
int bern(void *arg);

/**
   Run a dfs test on random directed graphs.
*/
void run_random_dir_graph_test(size_t log_start, size_t log_end){
  size_t p, i, j, l;
  size_t num_vts;
  size_t vt_size;
  struct bern_arg b;
  size_t *rand_start = NULL;
  void *pre = NULL, *post = NULL;
  struct graph g;
  struct adj_lst a;
  clock_t t;
  rand_start = malloc_perror(C_ITER, sizeof(size_t));
  printf("Run a dfs test on random directed graphs from %lu random "
         "start vertices in each graph\n",  TOLU(C_ITER));
  fflush(stdout);
  for (p = 0; p < C_PROBS_COUNT; p++){
    b.p = C_PROBS[p];
    printf("\tP[an edge is in a graph] = %.2f\n", b.p);
    for (i = log_start; i <= log_end; i++){
      num_vts = pow_two_perror(i);
      printf("\t\tvertices: %lu, E[# of directed edges]: %.1f\n",
             TOLU(num_vts), b.p * num_vts * (num_vts - 1));
      for (j = 0; j < C_FN_VT_COUNT; j++){
        vt_size =  C_VT_SIZES[j];
        /* no declared type after realloc; new eff. type to be acquired */
        pre = realloc_perror(pre, num_vts, vt_size);
        post = realloc_perror(post, num_vts, vt_size);
        graph_base_init(&g, num_vts, vt_size, 0);
        adj_lst_base_init(&a, &g);
        adj_lst_rand_dir(&a, C_WRITE_VT[j], bern, &b);
        for (l = 0; l < C_ITER; l++){
          rand_start[l] =  RANDOM() % num_vts;
        }
        t = clock();
        for (l = 0; l < C_ITER; l++){
          dfs(&a, rand_start[l], pre, post,
              C_READ_VT[j], C_WRITE_VT[j], C_AT_VT[j], C_CMP_VT[j], C_INCR_VT[j]);
        }
        t = clock() - t;
        printf("\t\t\t%s ave runtime:     %.6f seconds\n",
              C_VT_TYPES[j], (double)t / C_ITER / CLOCKS_PER_SEC);
        adj_lst_free(&a);
      }
    }
  }
  free(rand_start);
  free(pre);
  free(post);
  rand_start = NULL;
  pre = NULL;
  post = NULL;
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
  if (b->p >= C_PROB_ONE) return 1;
  if (b->p <= C_PROB_ZERO) return 0;
  if (b->p > DRAND()) return 1;
  return 0;
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
 if (args[0] > C_USHORT_BIT - 2 ||
     args[1] > C_USHORT_BIT - 2 ||
     args[1] < args[0]){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  run_random_dir_graph_test(args[0], args[1]);
  free(args);
  args = NULL;
  return 0;
}
