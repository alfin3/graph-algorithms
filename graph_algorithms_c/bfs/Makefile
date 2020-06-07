CC = gcc
CFLAGS = -I.
OBJ = bfs_main.o bfs.o graph.o queue.o stack.o

bfs : $(OBJ)
	$(CC) -o bfs $(OBJ)

bfs_main.o : bfs.h graph.h stack.h
bfs.o : bfs.h graph.h queue.h stack.h
graph.o : graph.h stack.h
queue.o : queue.h
stack.o : stack.h

.PHONY : clean
clean :
	rm bfs $(OBJ)
