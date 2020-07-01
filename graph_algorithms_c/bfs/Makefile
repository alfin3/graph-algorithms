CC = gcc
DS_DIR = ../../data_structures_c/
GRAPH_DIR = $(DS_DIR)graph/
QUEUE_DIR = $(DS_DIR)queue/
STACK_DIR = $(DS_DIR)stack/
UTILS_DIR = $(DS_DIR)utilities-ds/
CFLAGS = -I$(GRAPH_DIR) -I$(QUEUE_DIR) -I$(STACK_DIR) -I$(UTILS_DIR) \
         -Wall -Wextra -Og

OBJ = bfs-main.o \
      bfs.o \
      $(GRAPH_DIR)graph.o \
      $(QUEUE_DIR)queue.o \
      $(STACK_DIR)stack.o \
      $(UTILS_DIR)utilities-ds.o

bfs : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

bfs-main.o : bfs.h \
             $(GRAPH_DIR)graph.h \
             $(STACK_DIR)stack.h \
             $(UTILS_DIR)utilities-ds.h
bfs.o : bfs.h \
        $(GRAPH_DIR)graph.h \
        $(QUEUE_DIR)queue.h \
        $(STACK_DIR)stack.h
$(GRAPH_DIR)graph.o : $(GRAPH_DIR)graph.h \
                      $(STACK_DIR)stack.h \
                      $(UTILS_DIR)utilities-ds.h
$(QUEUE_DIR)queue.o : $(QUEUE_DIR)queue.h
$(STACK_DIR)stack.o : $(STACK_DIR)stack.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bfs $(OBJ)
