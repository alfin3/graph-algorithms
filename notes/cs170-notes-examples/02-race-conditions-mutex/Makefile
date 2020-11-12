CC = gcc
CFLAGS = -Wall -Wextra

all : race-conditions-1 race-conditions-2 race-conditions-mutex

race-conditions-1 : race-conditions-1.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

race-conditions-2 : race-conditions-2.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

race-conditions-mutex : race-conditions-mutex.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

.PHONY : clean

clean :
	rm -f race-conditions-1 race-conditions-2 race-conditions-mutex
