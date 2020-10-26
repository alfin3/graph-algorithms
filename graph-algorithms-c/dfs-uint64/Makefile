CC = gcc
DS_DIR = ../../data-structures-c/
GRAPH_DIR = $(DS_DIR)graph-uint64/
STACK_DIR = $(DS_DIR)stack-uint64/
UTILS_DIR = $(DS_DIR)utilities-ds/
CFLAGS = -I$(GRAPH_DIR) -I$(STACK_DIR) -I$(UTILS_DIR) \
         -Wall -Wextra -O3

OBJ = dfs-uint64-main.o          \
      dfs-uint64.o               \
      $(GRAPH_DIR)graph-uint64.o \
      $(STACK_DIR)stack-uint64.o \
      $(UTILS_DIR)utilities-ds.o

dfs-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dfs-uint64-main.o :          dfs-uint64.h               \
                             $(GRAPH_DIR)graph-uint64.h \
                             $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
dfs-uint64.o :               dfs-uint64.h               \
                             $(GRAPH_DIR)graph-uint64.h \
                             $(STACK_DIR)stack-uint64.h
$(GRAPH_DIR)graph-uint64.o : $(GRAPH_DIR)graph-uint64.h \
                             $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
$(STACK_DIR)stack-uint64.o : $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f dfs-uint64 $(OBJ)
