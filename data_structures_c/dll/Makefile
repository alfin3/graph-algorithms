CC = gcc
CFLAGS = -I. -Wall -Wextra -g
OBJ = dll-main.o dll.o

dll : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dll-main.o : dll.h 
dll.o : dll.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)

clean-all : 
	rm -f dll $(OBJ)
