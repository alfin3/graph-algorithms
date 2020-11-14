CC = gcc
CFLAGS = -Wall -Wextra

all : bounded-buffer-mutex

bounded-buffer-mutex : bounded-buffer-mutex.c ctimer.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

ctimer.o : ctimer.h

.PHONY : clean

clean :
	rm -f bounded-buffer-mutex ctimer.o
