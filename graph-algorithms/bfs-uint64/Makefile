CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph-uint64/
QUEUE_DIR = $(DS_DIR)queue/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)     \
         -I$(QUEUE_DIR)     \
         -I$(STACK_DIR)     \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = bfs-uint64-main.o                   \
      bfs-uint64.o                        \
      $(GRAPH_DIR)graph-uint64.o          \
      $(QUEUE_DIR)queue.o                 \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

bfs-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

bfs-uint64-main.o                   : bfs-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
bfs-uint64.o                        : bfs-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(QUEUE_DIR)queue.h                 \
                                      $(STACK_DIR)stack.h
$(GRAPH_DIR)graph-uint64.o          : $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(QUEUE_DIR)queue.o                 : $(QUEUE_DIR)queue.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bfs-uint64 $(OBJ)
