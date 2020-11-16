CC = gcc
CFLAGS = -Wall -Wextra

all : bounded-buffer-mutex       \
      bounded-buffer-cond-vars-1 \
      bounded-buffer-cond-vars-2

bounded-buffer-mutex : bounded-buffer-mutex.c ctimer.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-1 : bounded-buffer-cond-vars-1.c ctimer.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-2 : bounded-buffer-cond-vars-2.c ctimer.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

ctimer.o : ctimer.h

.PHONY : clean

clean :
	rm -f bounded-buffer-mutex bounded-buffer-cond-vars-1 bounded-buffer-cond-vars-2 ctimer.o
