CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph/
QUEUE_DIR = $(DS_DIR)queue/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
CFLAGS = -I$(GRAPH_DIR)                        \
         -I$(QUEUE_DIR)                        \
         -I$(STACK_DIR)                        \
         -I$(UTILS_MEM_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = bfs-main.o                      \
      bfs.o                           \
      $(GRAPH_DIR)graph.o             \
      $(QUEUE_DIR)queue.o             \
      $(STACK_DIR)stack.o             \
      $(UTILS_MEM_DIR)utilities-mem.o \

bfs : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

bfs-main.o                      : bfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
bfs.o                           : bfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(QUEUE_DIR)queue.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(GRAPH_DIR)graph.o             : $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(QUEUE_DIR)queue.o             : $(QUEUE_DIR)queue.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bfs $(OBJ)
