CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph-uint64/
QUEUE_DIR = $(DS_DIR)queue-uint64/
STACK_DIR = $(DS_DIR)stack-uint64/
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR) -I$(QUEUE_DIR) -I$(STACK_DIR) -I$(UTILS_DIR) \
         -Wall -Wextra -O3

OBJ = bfs-uint64-main.o               \
      bfs-uint64.o                    \
      $(GRAPH_DIR)graph-uint64.o      \
      $(QUEUE_DIR)queue-uint64.o      \
      $(STACK_DIR)stack-uint64.o      \
      $(UTILS_DIR)utilities-rand-mod.o

bfs-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

bfs-uint64-main.o :                bfs-uint64.h                     \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
bfs-uint64.o :                     bfs-uint64.h                     \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(QUEUE_DIR)queue-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h
$(GRAPH_DIR)graph-uint64.o       : $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(QUEUE_DIR)queue-uint64.o       : $(QUEUE_DIR)queue-uint64.h
$(STACK_DIR)stack-uint64.o       : $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(UTILS_DIR)utilities-rand-mod.o : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bfs-uint64 $(OBJ)
