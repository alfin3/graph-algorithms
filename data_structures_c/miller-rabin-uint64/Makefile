CC = gcc
CFLAGS = -I. -Wall -Wextra
OBJ = miller-rabin-uint64-main.o miller-rabin-uint64.o utilities-ds.o

miller-rabin-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint64-main.o : miller-rabin-uint64.h utilities-ds.h
miller-rabin-uint64.o : utilities-ds.h 
utilities-ds.o : utilities-ds.h

.PHONY : clean
clean :
	rm miller-rabin-uint64 $(OBJ)
