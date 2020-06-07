CC = gcc
CFLAGS = -I.
OBJ = dfs_main.o dfs.o graph.o stack.o

dfs : $(OBJ)
	$(CC) -o dfs $(OBJ)

dfs_main.o : dfs.h graph.h stack.h
dfs.o : dfs.h graph.h stack.h
graph.o : graph.h stack.h
stack.o : stack.h

.PHONY : clean
clean :
	rm dfs $(OBJ)
