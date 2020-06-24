CC = gcc
CFLAGS = -I. -Wall -Wextra
OBJ = utilities-ds-main.o utilities-ds.o

utilities-ds : $(OBJ)
	$(CC) $(CFLAGS) -o utilities-ds $(OBJ) 

utilities-ds-main.o : utilities-ds.h 
utilities-ds.o : utilities-ds.h

.PHONY : clean
clean :
	rm $(OBJ)
