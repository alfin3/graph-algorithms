CC = gcc
CFLAGS = -I. -Wall -Wextra -O3
OBJ = utilities-ds-main.o utilities-ds.o

utilities-ds : $(OBJ)
	$(CC) $(CFLAGS) -o utilities-ds $(OBJ) 

utilities-ds-main.o : utilities-ds.h 
utilities-ds.o : utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-ds $(OBJ)
