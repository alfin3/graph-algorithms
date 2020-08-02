CC = gcc
CFLAGS = -I.
OBJ = prim_main.o prim.o graph.o heap.o stack.o

prim : $(OBJ)
	$(CC) -o prim $(OBJ)

prim_main.o : prim.h graph.h stack.h
prim.o : prim.h graph.h heap.h stack.h
graph.o : graph.h stack.h
heap.o : heap.h
stack.o : stack.h

.PHONY : clean
clean :
	rm prim $(OBJ)
