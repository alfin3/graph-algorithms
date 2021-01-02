CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph-uint64/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)     \
         -I$(STACK_DIR)     \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = dfs-uint64-main.o                   \
      dfs-uint64.o                        \
      $(GRAPH_DIR)graph-uint64.o          \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o


dfs-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dfs-uint64-main.o                   : dfs-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
dfs-uint64.o                        : dfs-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h
$(GRAPH_DIR)graph-uint64.o          : $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f dfs-uint64 $(OBJ)
