CC = gcc
DS_DIR = ../../data-structures/
ALG_DIR = ../
BFS_DIR   = $(ALG_DIR)bfs-uint64/
GRAPH_DIR = $(DS_DIR)graph-uint64/
HEAP_DIR  = $(DS_DIR)heap-uint32/
HT_DIR    = $(DS_DIR)ht-div-uint32/
DLL_DIR   = $(DS_DIR)dll/
QUEUE_DIR = $(DS_DIR)queue/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(BFS_DIR)       \
         -I$(GRAPH_DIR)     \
         -I$(HEAP_DIR)      \
         -I$(HT_DIR)        \
         -I$(DLL_DIR)       \
         -I$(QUEUE_DIR)     \
         -I$(STACK_DIR)     \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = dijkstra-uint64-main.o              \
      dijkstra-uint64.o                   \
      $(BFS_DIR)bfs-uint64.o              \
      $(GRAPH_DIR)graph-uint64.o          \
      $(HEAP_DIR)heap-uint32.o            \
      $(HT_DIR)ht-div-uint32.o            \
      $(DLL_DIR)dll.o                     \
      $(QUEUE_DIR)queue.o                 \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

dijkstra-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dijkstra-uint64-main.o              : dijkstra-uint64.h                   \
                                      $(BFS_DIR)bfs-uint64.h              \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
dijkstra-uint64.o                   : dijkstra-uint64.h                   \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(HEAP_DIR)heap-uint32.h            \
                                      $(STACK_DIR)stack.h
$(BFS_DIR)bfs-uint64.o              : $(BFS_DIR)bfs-uint64.h              \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(QUEUE_DIR)queue.h                 \
                                      $(STACK_DIR)stack.h
$(GRAPH_DIR)graph-uint64.o          : $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(HEAP_DIR)heap-uint32.o            : $(HEAP_DIR)heap-uint32.h            \
                                      $(HT_DIR)ht-div-uint32.h            \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(HT_DIR)ht-div-uint32.o            : $(HT_DIR)ht-div-uint32.h            \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h
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
	rm -f dijkstra-uint64 $(OBJ)
