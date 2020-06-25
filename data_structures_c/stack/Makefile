CC = gcc
CFLAGS = -I. -Wall -Wextra
OBJ = stack-main.o stack.o

stack : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

stack-main.o : stack.h 
stack.o : stack.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)

clean-all : 
	rm -f stack $(OBJ)
