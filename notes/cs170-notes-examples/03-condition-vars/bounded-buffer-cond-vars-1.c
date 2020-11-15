/**
   bounded-buffer-cond-vars-1.c

   A program for running a bounded buffer (producer-consumer) example
   by using mutex locks and condition variables, the latter to reduce 
   while loop polling in time slices.

   usage example on a 4-core machine:
   bounded-buffer-cond-vars-1 -c 1 -t 1 -q 10000 -s 100 -o 1000000
   bounded-buffer-cond-vars-1 -c 3 -t 1 -q 10000 -s 100 -o 1000000

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  queue and market initialization and freeing functions are changed
      to require a pointer to a preallocated block,
   -  the names of some variables are changed and a few minor bugs
      are fixed; the names of condition variables are changed to negated
      names to reflect their use,
   -  the outer polling while loop is removed due to the use of
      pthread_cond_wait within a dedicated predicate re-testing while loop.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "ctimer.h"

#define RAND() (drand48())
#define ARGS "c:t:o:q:s:V"

char *usage =
  "bounded-buffer-mutex "
  "-c clients "
  "-t traders "
  "-o orders "
  "-q queue-size "
  "-s number-stocks"
  "-V <verbose on>\n";

/**
   Order, order queue, and market structs, as well as initialization and 
   freeing functions.
*/

typedef struct{
  int stock_id;
  int stock_quantity;
  int action; //0 to buy or 1 to sell
  bool fulfilled;
} order_t;

typedef struct{
  int size;
  int head;
  int tail;
  order_t **orders;
  pthread_mutex_t lock; //producers and consumers
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
} order_q_t;

typedef struct market{
  int num_stocks;
  int *stocks;
  pthread_mutex_t lock; //consumers
} market_t;

void order_q_init(order_q_t *q, int size){
  memset(q, 0, sizeof(order_q_t)); //head = 0 and tail = 0
  q->size = size + 1; //+ 1 due to fifo queue implementation
  q->orders = calloc(q->size, sizeof(order_t *));
  assert(q->orders != NULL);
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->not_full, NULL);
  pthread_cond_init(&q->not_empty, NULL);
}

void order_q_free(order_q_t *q){
  while (q->head != q->tail){
    free(q->orders[q->tail]);
    q->orders[q->tail] = NULL;
    q->tail = (q->tail + 1) % q->size;
  }
  free(q->orders);
  q->orders = NULL;
}

void market_init(market_t *m, int num_stocks, int stock_quantity){
  m->num_stocks = num_stocks;
  m->stocks = malloc(num_stocks * sizeof(int));
  assert(m->stocks != NULL);
  for (int i = 0; i < num_stocks; i++){
    m->stocks[i] = stock_quantity;
  }
  pthread_mutex_init(&m->lock, NULL);
}

void market_free(market_t *m){
  free(m->stocks);
  m->stocks = NULL;
}

void market_print(market_t *m){
  for(int i = 0; i < m->num_stocks; i++){
    printf("stock: %d, quantity: %d\n", i , m->stocks[i]);
  }
}

/**
   Client (producer) and trader (consumer) thread arguments and entry 
   functions.
*/

typedef struct{
  int id;
  int order_count;
  int num_stocks;
  int stock_quantity;
  bool verbose;
  order_q_t *q;
} client_arg_t;

typedef struct{
  int id;
  bool verbose;
  bool *done;
  order_q_t *q;
  market_t *m;
} trader_arg_t;

/**
   Produces and queues order_count orders. After queuing an order, waits 
   until the order is fulfilled before queuing the next order. 
*/
void *client_thread(void *arg){
  client_arg_t *ca = arg;
  order_t *order = NULL;
  int next;
  double now;
  for (int i = 0; i < ca->order_count; i++){
    //produce an order
    order = malloc(sizeof(order_t));
    assert(order != NULL);
    order->stock_id = (int)(RAND() * (ca->num_stocks - 1));
    order->stock_quantity = (int)(RAND() * ca->stock_quantity);
    order->action = (RAND() > 0.5) ? 0 : 1;
    order->fulfilled = false;
    //queue the order
    pthread_mutex_lock(&ca->q->lock);
    next = (ca->q->head + 1) % ca->q->size;
    while (next == ca->q->tail){
      //queue is full; wait for not_full signal and retest
      pthread_cond_wait(&(ca->q->not_full), &(ca->q->lock));
      next = (ca->q->head + 1) % ca->q->size;
    }
    //queue is not full; queue the order, signal not_empty, unlock mutex
    if (ca->verbose){
      now = ctimer();
      printf("%10.0f client %d: ", now, ca->id);
      printf("queued stock %d, for %d, %s\n",
	     order->stock_id,
	     order->stock_quantity,
	     (order->action ? "SELL" : "BUY"));
    }
    ca->q->orders[next] = order;
    ca->q->head = next;
    pthread_cond_signal(&(ca->q->not_empty));
    pthread_mutex_unlock(&(ca->q->lock));
    //wait; no race condition wrt order->fulfilled (producer is reading)
    while(!order->fulfilled);
    free(order);
    order = NULL;
  }
  return NULL;
}

/**
   Dequeues and consumes orders, as long as there are orders.
*/
void *trader_thread(void *arg){
  trader_arg_t *ta = arg;
  order_t *order = NULL;
  int next;
  double now;
  while (true){
    pthread_mutex_lock(&(ta->q->lock));
    while (ta->q->head == ta->q->tail){
      //empty queue; exit if done, else wait for not_empty signal and retest
      if (*ta->done){
	pthread_cond_signal(&(ta->q->not_empty));
	pthread_mutex_unlock(&(ta->q->lock));
	pthread_exit(NULL);
      }
      pthread_cond_wait(&(ta->q->not_empty), &(ta->q->lock));
    }
    //queue is not empty; dequeue, signal not_full, unlock mutex
    next = (ta->q->tail + 1) % ta->q->size;
    order = ta->q->orders[next];
    ta->q->tail = next;
    pthread_cond_signal(&(ta->q->not_full));
    pthread_mutex_unlock(&(ta->q->lock));
    //process a dequeued order
    pthread_mutex_lock(&(ta->m->lock));
    if (order->action == 0){
      ta->m->stocks[order->stock_id] -= order->stock_quantity;
      if (ta->m->stocks[order->stock_id] < 0){
	ta->m->stocks[order->stock_id] = 0;
      }
    }else{
      ta->m->stocks[order->stock_id] += order->stock_quantity;
    }
    if (ta->verbose){
      now = ctimer();
      printf("%10.0f trader: %d ", now, ta->id);
      printf("fulfilled stock %d for %d\n",
	     order->stock_id,
	     order->stock_quantity);
    }
    pthread_mutex_unlock(&(ta->m->lock));
    //atomic memory write on x86; inform the reading client thread
    order->fulfilled = true;
  }
  return NULL;
}

int main(int argc, char **argv){
  client_arg_t *ca = NULL;
  trader_arg_t *ta = NULL;
  pthread_t *client_ids = NULL;
  pthread_t *trader_ids = NULL;
  order_q_t *q = NULL;
  market_t *m = NULL;
  int num_client_threads = 1, num_trader_threads = 1;
  int orders_per_client = 1;
  int queue_size = 1, num_stocks = 1, stock_quantity = 5000;
  int c, err;
  bool verbose = false;
  bool done = false;
  double start, end;
  while ((c = getopt(argc, argv, ARGS)) != EOF){
    switch (c){
    case 'c':
      num_client_threads = atoi(optarg);
      break;
    case 't':
      num_trader_threads = atoi(optarg);
      break;
    case 'o':
      orders_per_client = atoi(optarg);
      break;
    case 'q':
      queue_size = atoi(optarg);
      break;
    case 's':
      num_stocks = atoi(optarg);
      break;
    case 'V':
      verbose = true;
      break;
    default:
      fprintf(stderr, "unrecognized command %c\n", (char)c);
      fprintf(stderr,"usage: %s\n", usage);
      exit(1);
    }
  }
  ca = malloc(num_client_threads * sizeof(client_arg_t));
  assert(ca != NULL);
  ta = malloc(num_trader_threads * sizeof(trader_arg_t));
  assert(ta != NULL);
  client_ids = malloc(num_client_threads * sizeof(pthread_t));
  assert(client_ids != NULL);
  trader_ids = malloc(num_trader_threads * sizeof(pthread_t));
  assert(trader_ids != NULL);
  q = malloc(sizeof(order_q_t));
  assert(q != NULL);
  m = malloc(sizeof(market_t));
  assert(m != NULL);
  order_q_init(q, queue_size);
  market_init(m, num_stocks, stock_quantity);
  start = ctimer();
  //spawn threads
  for (int i = 0; i < num_client_threads; i++){
    ca[i].id = i;
    ca[i].order_count = orders_per_client;
    ca[i].num_stocks = num_stocks;
    ca[i].stock_quantity = stock_quantity;
    ca[i].q = q;
    ca[i].verbose = verbose;
    err = pthread_create(&client_ids[i], NULL, client_thread, &ca[i]);
    assert(err == 0);
  }
  for (int i = 0; i < num_trader_threads; i++){
    ta[i].id = i;
    ta[i].q = q;
    ta[i].m = m;
    ta[i].done = &done;
    ta[i].verbose = verbose;
    err = pthread_create(&trader_ids[i], NULL, trader_thread, &ta[i]);
    assert(err == 0);
  }
  //join client threads after each client's orders are fulfilled
  for (int i = 0; i < num_client_threads; i++){
    err = pthread_join(client_ids[i], NULL);
    assert(err == 0);
  }
  //set done to true, then signal waiting trader threads to stop waiting
  pthread_mutex_lock(&q->lock);
  done = true;
  pthread_cond_signal(&q->not_empty);
  pthread_mutex_unlock(&q->lock);
  for (int i = 0; i < num_trader_threads; i++){
    err = pthread_join(trader_ids[i], NULL);
    assert(err == 0);
  }
  end = ctimer();
  if (verbose){market_print(m);}
  printf("%f transactions / sec\n",
	 orders_per_client * num_client_threads / (end - start));
  free(ca);
  free(ta);
  free(client_ids);
  free(trader_ids);
  order_q_free(q);
  free(q);
  market_free(m);
  free(m);
  ca = NULL;
  ta = NULL;
  client_ids = NULL;
  trader_ids = NULL;
  q = NULL;
  m = NULL;
  return 0;
}
