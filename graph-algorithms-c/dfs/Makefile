CC = gcc
DS_DIR = ../../data-structures-c/
GRAPH_DIR = $(DS_DIR)graph/
STACK_DIR = $(DS_DIR)stack/
UTILS_DIR = $(DS_DIR)utilities-ds/
CFLAGS = -I$(GRAPH_DIR) -I$(STACK_DIR) -I$(UTILS_DIR) \
         -Wall -Wextra -O3

OBJ = dfs-main.o \
      dfs.o \
      $(GRAPH_DIR)graph.o \
      $(STACK_DIR)stack.o \
      $(UTILS_DIR)utilities-ds.o

dfs : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dfs-main.o : dfs.h \
             $(GRAPH_DIR)graph.h \
             $(STACK_DIR)stack.h \
             $(UTILS_DIR)utilities-ds.h
dfs.o : dfs.h \
        $(GRAPH_DIR)graph.h \
        $(STACK_DIR)stack.h
$(GRAPH_DIR)graph.o : $(GRAPH_DIR)graph.h \
                      $(STACK_DIR)stack.h \
                      $(UTILS_DIR)utilities-ds.h
$(STACK_DIR)stack.o : $(STACK_DIR)stack.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f dfs $(OBJ)
