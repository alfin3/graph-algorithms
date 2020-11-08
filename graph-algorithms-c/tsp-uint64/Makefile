CC = gcc
DS_DIR = ../../data-structures-c/
ALG_DIR = ../
DIJKSTRA_DIR   = $(ALG_DIR)dijkstra-uint64/
GRAPH_DIR      = $(DS_DIR)graph-uint64/
HEAP_DIR       = $(DS_DIR)heap-uint32/
HT_DIV_DIR     = $(DS_DIR)ht-div-uint32/
HT_MUL_DIR     = $(DS_DIR)ht-mul-uint64/
DLL_DIR        = $(DS_DIR)dll/
STACK_DIR      = $(DS_DIR)stack-uint64/
UTILS_DIR      = $(DS_DIR)utilities-ds/
CFLAGS = -I$(DIJKSTRA_DIR) \
         -I$(GRAPH_DIR)    \
         -I$(HEAP_DIR)     \
         -I$(HT_DIV_DIR)   \
         -I$(HT_MUL_DIR)   \
         -I$(DLL_DIR)      \
         -I$(STACK_DIR)    \
         -I$(UTILS_DIR)    \
         -Wall -Wextra -O3 -g

OBJ = tsp-uint64-main.o                \
      tsp-uint64.o                     \
      $(DIJKSTRA_DIR)dijkstra-uint64.o \
      $(GRAPH_DIR)graph-uint64.o       \
      $(HEAP_DIR)heap-uint32.o         \
      $(HT_DIV_DIR)ht-div-uint32.o     \
      $(HT_MUL_DIR)ht-mul-uint64.o     \
      $(DLL_DIR)dll.o                  \
      $(STACK_DIR)stack-uint64.o       \
      $(UTILS_DIR)utilities-ds.o

tsp-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

tsp-uint64-main.o                : tsp-uint64.h                     \
                                   $(DIJKSTRA_DIR)dijkstra-uint64.h \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-ds.h
tsp-uint64.o                     : tsp-uint64.h                     \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(HT_MUL_DIR)ht-mul-uint64.h     \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-ds.h
$(DIJKSTRA_DIR)dijkstra-uint64.o : $(DIJKSTRA_DIR)dijkstra-uint64.h \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(HEAP_DIR)heap-uint32.h         \
                                   $(STACK_DIR)stack-uint64.h
$(GRAPH_DIR)graph-uint64.o       : $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-ds.h
$(HEAP_DIR)heap-uint32.o         : $(HEAP_DIR)heap-uint32.h         \
                                   $(HT_DIV_DIR)ht-div-uint32.h     \
                                   $(UTILS_DIR)utilities-ds.h
$(HT_DIV_DIR)ht-div-uint32.o     : $(HT_DIV_DIR)ht-div-uint32.h     \
                                   $(DLL_DIR)dll.h                  \
                                   $(UTILS_DIR)utilities-ds.h
$(HT_MUL_DIR)ht-mul-uint64.o     : $(HT_MUL_DIR)ht-mul-uint64.h     \
                                   $(DLL_DIR)dll.h                  \
                                   $(UTILS_DIR)utilities-ds.h
$(DLL_DIR)dll.o                  : $(DLL_DIR)dll.h
$(STACK_DIR)stack-uint64.o       : $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-ds.h
$(UTILS_DIR)utilities-ds.o       : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f tsp-uint64 $(OBJ)
