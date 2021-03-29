CC = gcc
DS_DIR        = ../../data-structures/
ALG_DIR       = ../
GRAPH_DIR     = $(DS_DIR)graph/
HEAP_DIR      = $(DS_DIR)heap/
HT_DIV_DIR    = $(DS_DIR)ht-div/
HT_MUL_DIR    = $(DS_DIR)ht-mul/
DLL_DIR       = $(DS_DIR)dll/
STACK_DIR     = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
CFLAGS = -I$(GRAPH_DIR)                        \
         -I$(HEAP_DIR)                         \
         -I$(HT_DIV_DIR)                       \
         -I$(HT_MUL_DIR)                       \
         -I$(DLL_DIR)                          \
         -I$(STACK_DIR)                        \
         -I$(UTILS_MEM_DIR)                    \
         -I$(UTILS_MOD_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = prim-main.o                     \
      prim.o                          \
      $(GRAPH_DIR)graph.o             \
      $(HEAP_DIR)heap.o               \
      $(HT_DIV_DIR)ht-div.o           \
      $(HT_MUL_DIR)ht-mul.o           \
      $(DLL_DIR)dll.o                 \
      $(STACK_DIR)stack.o             \
      $(UTILS_MEM_DIR)utilities-mem.o \
      $(UTILS_MOD_DIR)utilities-mod.o

prim : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

prim-main.o                     : prim.h                          \
                                  $(HEAP_DIR)heap.h               \
                                  $(HT_DIV_DIR)ht-div.h           \
                                  $(HT_MUL_DIR)ht-mul.h           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h
prim.o                          : prim.h                          \
                                  $(GRAPH_DIR)graph.h             \
                                  $(HEAP_DIR)heap.h               \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(GRAPH_DIR)graph.o             : $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(HEAP_DIR)heap.o               : $(HEAP_DIR)heap.h               \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(HT_DIV_DIR)ht-div.o           : $(HT_DIV_DIR)ht-div.h           \
                                  $(DLL_DIR)dll.h                 \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h
$(HT_MUL_DIR)ht-mul.o           : $(HT_MUL_DIR)ht-mul.h           \
                                  $(DLL_DIR)dll.h                 \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h
$(DLL_DIR)dll.o                 : $(DLL_DIR)dll.h                 \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f prim $(OBJ)
