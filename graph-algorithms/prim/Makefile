CC = gcc
DS_DIR = ../../data-structures/
ALG_DIR = ../
GRAPH_DIR = $(DS_DIR)graph/
HEAP_DIR  = $(DS_DIR)heap/
HT_DIV_UINT64_DIR = $(DS_DIR)ht-div-uint64/
HT_MUL_UINT64_DIR = $(DS_DIR)ht-mul-uint64/
DLL_DIR   = $(DS_DIR)dll/
STACK_DIR = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)         \
         -I$(HEAP_DIR)          \
         -I$(HT_DIV_UINT64_DIR) \
         -I$(HT_MUL_UINT64_DIR) \
         -I$(DLL_DIR)           \
         -I$(STACK_DIR)         \
         -I$(UTILS_MEM_DIR)     \
         -I$(UTILS_RM_DIR)      \
         -Wall -Wextra -O3

OBJ = prim-main.o                         \
      prim.o                              \
      $(GRAPH_DIR)graph.o                 \
      $(HEAP_DIR)heap.o                   \
      $(HT_DIV_UINT64_DIR)ht-div-uint64.o \
      $(HT_MUL_UINT64_DIR)ht-mul-uint64.o \
      $(DLL_DIR)dll.o                     \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

prim : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

prim-main.o                         : prim.h                              \
                                      $(HEAP_DIR)heap.h                   \
                                      $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
prim.o                              : prim.h                              \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(HEAP_DIR)heap.h                   \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(GRAPH_DIR)graph.o                 : $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(HEAP_DIR)heap.o                   : $(HEAP_DIR)heap.h                   \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(HT_DIV_UINT64_DIR)ht-div-uint64.o : $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(HT_MUL_UINT64_DIR)ht-mul-uint64.o : $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f prim $(OBJ)
