CC = gcc
CFLAGS = -I. -Wall -Wextra
OBJ = miller-rabin-uint32-main.o miller-rabin-uint32.o utilities-ds.o

miller-rabin-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint32-main.o : miller-rabin-uint32.h utilities-ds.h
miller-rabin-uint32.o : utilities-ds.h 
utilities-ds.o : utilities-ds.h

.PHONY : clean
clean :
	rm miller-rabin-uint32 $(OBJ)
