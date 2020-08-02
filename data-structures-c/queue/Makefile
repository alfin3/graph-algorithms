CC = gcc
CFLAGS = -I. -Wall -Wextra
OBJ = queue-main.o queue.o

queue : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

queue-main.o : queue.h 
queue.o : queue.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)

clean-all : 
	rm -f queue $(OBJ)
