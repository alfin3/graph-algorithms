/**
   bounded-buffer-cond-vars-2.c

   A program for running a bounded buffer (producer-consumer) example
   by using mutex locks and an additional condition variable, the latter to 
   further reduce while loop polling in time slices.

   usage example on a 4-core machine:
   bounded-buffer-cond-vars-2 -c 1 -t 1 -q 10000 -s 100 -o 1000000
   bounded-buffer-cond-vars-2 -c 3 -t 1 -q 10000 -s 100 -o 1000000

   comparison example demonstrating the effect of cond_fulfilled:
   bounded-buffer-cond-mutex -c 20 -t 1 -q 1 -s 1 -o 10000
   bounded-buffer-cond-vars-1 -c 20 -t 1 -q 1 -s 1 -o 10000
   bounded-buffer-cond-vars-2 -c 20 -t 1 -q 1 -s 1 -o 10000

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  queue and market initialization and freeing functions are changed
      to require a pointer to a preallocated block,
   -  the names of some variables are changed and a few minor bugs
      are fixed; the names of some condition variables are changed to
      negated names to reflect their use,
   -  the outer polling while loops are removed due to the use of
      pthread_cond_wait within dedicated predicate re-testing while loops,
   -  memory allocation and pthread functions are used with wrapped 
      error checking.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "ctimer.h"
#include "utilities-mem.h"
#include "utilities-concur.h"

#define RAND() (drand48())
#define ARGS "c:t:o:q:s:V"

const char *usage =
  "bounded-buffer-cond-vars-2 "
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
  pthread_mutex_t lock; //only the handling producer and consumer
  pthread_cond_t cond_fulfilled;
} order_t;

typedef struct{
  int size;
  int head;
  int tail;
  order_t **orders;
  pthread_mutex_t lock; //producers and consumers
  pthread_cond_t cond_not_full;
  pthread_cond_t cond_not_empty;
} order_q_t;

typedef struct market{
  int num_stocks;
  int *stocks;
  pthread_mutex_t lock; //consumers
} market_t;

void order_q_init(order_q_t *q, int size){
  memset(q, 0, sizeof(order_q_t)); //head = 0 and tail = 0
  q->size = size + 1; //+ 1 due to fifo queue implementation
  q->orders = calloc_perror(q->size, sizeof(order_t *));
  mutex_init_perror(&q->lock);
  cond_init_perror(&q->cond_not_full);
  cond_init_perror(&q->cond_not_empty);
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
  m->stocks = malloc_perror(num_stocks * sizeof(int));
  for (int i = 0; i < num_stocks; i++){
    m->stocks[i] = stock_quantity;
  }
  mutex_init_perror(&m->lock);
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
  for (int i = 0; i < ca->order_count; i++){
    //produce an order
    order = malloc_perror(sizeof(order_t));
    order->stock_id = (int)(RAND() * (ca->num_stocks - 1));
    order->stock_quantity = (int)(RAND() * ca->stock_quantity);
    order->action = (RAND() > 0.5) ? 0 : 1;
    order->fulfilled = false;
    mutex_init_perror(&order->lock);
    cond_init_perror(&order->cond_fulfilled);
    //queue the order
    mutex_lock_perror(&ca->q->lock);
    next = (ca->q->head + 1) % ca->q->size;
    while (next == ca->q->tail){
      //queue is full; wait for cond_not_full and retest
      cond_wait_perror(&ca->q->cond_not_full, &ca->q->lock);
      next = (ca->q->head + 1) % ca->q->size;
    }
    //queue is not full; queue, signal cond_not_empty, unlock mutex
    if (ca->verbose){
      printf("%10.6f client %d: ", ctimer(), ca->id);
      printf("queued stock %d, for %d, %s\n",
	     order->stock_id,
	     order->stock_quantity,
	     (order->action ? "SELL" : "BUY"));
    }
    ca->q->orders[next] = order;
    ca->q->head = next;
    cond_signal_perror(&ca->q->cond_not_empty);
    mutex_unlock_perror(&ca->q->lock);
    //wait for cond_fulfilled before producing another order, if any
    mutex_lock_perror(&order->lock);
    while(!order->fulfilled){
      cond_wait_perror(&order->cond_fulfilled, &order->lock);
    }
    mutex_unlock_perror(&order->lock);
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
  while (true){
    mutex_lock_perror(&ta->q->lock);
    while (ta->q->head == ta->q->tail){
      //empty queue; exit if done, else wait for cond_not_empty and retest
      if (*ta->done){
	cond_signal_perror(&ta->q->cond_not_empty);
	mutex_unlock_perror(&ta->q->lock);
	pthread_exit(NULL);
      }
      cond_wait_perror(&ta->q->cond_not_empty, &ta->q->lock);
    }
    //queue is not empty; dequeue, signal cond_not_full, unlock mutex
    next = (ta->q->tail + 1) % ta->q->size;
    order = ta->q->orders[next];
    ta->q->tail = next;
    cond_signal_perror(&ta->q->cond_not_full);
    mutex_unlock_perror(&ta->q->lock);
    //process a dequeued order
    mutex_lock_perror(&ta->m->lock);
    if (order->action == 0){
      ta->m->stocks[order->stock_id] -= order->stock_quantity;
      if (ta->m->stocks[order->stock_id] < 0){
	ta->m->stocks[order->stock_id] = 0;
      }
    }else{
      ta->m->stocks[order->stock_id] += order->stock_quantity;
    }
    if (ta->verbose){
      printf("%10.0f trader: %d ", ctimer(), ta->id);
      printf("fulfilled stock %d for %d\n",
	     order->stock_id,
	     order->stock_quantity);
    }
    mutex_unlock_perror(&ta->m->lock);
    //signal cond_fulfilled for the next order to be produced, if any
    mutex_lock_perror(&order->lock);
    order->fulfilled = true;
    cond_signal_perror(&order->cond_fulfilled);
    mutex_unlock_perror(&order->lock);
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
  int c;
  bool verbose = false;
  bool done = false;
  double start, end;
  while ((c = getopt(argc, argv, ARGS)) != -1){
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
  ca = malloc_perror(num_client_threads * sizeof(client_arg_t));
  ta = malloc_perror(num_trader_threads * sizeof(trader_arg_t));
  client_ids = malloc_perror(num_client_threads * sizeof(pthread_t));
  trader_ids = malloc_perror(num_trader_threads * sizeof(pthread_t));
  q = malloc_perror(sizeof(order_q_t));
  m = malloc_perror(sizeof(market_t));
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
    thread_create_perror(&client_ids[i], client_thread, &ca[i]);
  }
  for (int i = 0; i < num_trader_threads; i++){
    ta[i].id = i;
    ta[i].q = q;
    ta[i].m = m;
    ta[i].done = &done;
    ta[i].verbose = verbose;
    thread_create_perror(&trader_ids[i], trader_thread, &ta[i]);
  }
  //join client threads after each client's orders are fulfilled
  for (int i = 0; i < num_client_threads; i++){
    thread_join_perror(client_ids[i], NULL);
  }
  //set done to true, then signal waiting trader threads to stop waiting
  mutex_lock_perror(&q->lock);
  done = true;
  cond_signal_perror(&q->cond_not_empty);
  mutex_unlock_perror(&q->lock);
  for (int i = 0; i < num_trader_threads; i++){
    thread_join_perror(trader_ids[i], NULL);
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
