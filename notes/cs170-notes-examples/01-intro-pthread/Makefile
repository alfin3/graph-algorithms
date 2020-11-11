CC = gcc
CFLAGS = -Wall -Wextra -O2

all : avg-single-thread avg-mult-thread

avg-single-thread : avg-single-thread.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

avg-mult-thread : avg-mult-thread.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

.PHONY : clean

clean :
	rm -f avg-single-thread avg-mult-thread
