CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR = $(DS_DIR)graph-uint64/
HEAP_DIR  = $(DS_DIR)heap-uint32/
HT_DIR    = $(DS_DIR)ht-div-uint32/
DLL_DIR   = $(DS_DIR)dll/
STACK_DIR = $(DS_DIR)stack-uint64/
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)    \
         -I$(HEAP_DIR)     \
         -I$(HT_DIR)       \
         -I$(DLL_DIR)      \
         -I$(STACK_DIR)    \
         -I$(UTILS_DIR)    \
         -Wall -Wextra -O3

OBJ = prim-uint64-main.o               \
      prim-uint64.o                    \
      $(GRAPH_DIR)graph-uint64.o       \
      $(HEAP_DIR)heap-uint32.o         \
      $(HT_DIR)ht-div-uint32.o         \
      $(DLL_DIR)dll.o                  \
      $(STACK_DIR)stack-uint64.o       \
      $(UTILS_DIR)utilities-rand-mod.o

prim-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

prim-uint64-main.o :               prim-uint64.h                    \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
prim-uint64.o :                    prim-uint64.h                    \
                                   $(GRAPH_DIR)graph-uint64.h       \
                                   $(HEAP_DIR)heap-uint32.h         \
                                   $(STACK_DIR)stack-uint64.h
$(GRAPH_DIR)graph-uint64.o       : $(GRAPH_DIR)graph-uint64.h       \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(HEAP_DIR)heap-uint32.o         : $(HEAP_DIR)heap-uint32.h         \
                                   $(HT_DIR)ht-div-uint32.h         \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(HT_DIR)ht-div-uint32.o         : $(HT_DIR)ht-div-uint32.h         \
                                   $(DLL_DIR)dll.h                  \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                  : $(DLL_DIR)dll.h
$(STACK_DIR)stack-uint64.o       : $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(UTILS_DIR)utilities-rand-mod.o : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f prim-uint64 $(OBJ)
