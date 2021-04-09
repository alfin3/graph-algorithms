CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
CFLAGS = -I$(GRAPH_DIR)                        \
         -I$(STACK_DIR)                        \
         -I$(UTILS_MEM_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = dfs-test.o                      \
      dfs.o                           \
      $(GRAPH_DIR)graph.o             \
      $(STACK_DIR)stack.o             \
      $(UTILS_MEM_DIR)utilities-mem.o 


dfs-test : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dfs-test.o                      : dfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
dfs.o                           : dfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h
$(GRAPH_DIR)graph.o             : $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f dfs-test $(OBJ)
