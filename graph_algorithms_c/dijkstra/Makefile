CC = gcc
CFLAGS = -I.
OBJ = dijkstra_main.o dijkstra.o graph.o heap.o stack.o

dijkstra : $(OBJ)
	$(CC) -o dijkstra $(OBJ)

dijkstra_main.o : dijkstra.h graph.h heap.h stack.h
dijkstra.o : dijkstra.h graph.h heap.h stack.h
graph.o : graph.h stack.h
heap.o : heap.h
stack.o : stack.h

.PHONY : clean
clean :
	rm dijkstra $(OBJ)
