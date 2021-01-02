CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR      = $(DS_DIR)graph-uint64/
HEAP_DIR       = $(DS_DIR)heap-uint32/
HT_DIV_DIR     = $(DS_DIR)ht-div-uint32/
HT_MUL_DIR     = $(DS_DIR)ht-mul-uint64/
DLL_DIR        = $(DS_DIR)dll/
STACK_DIR      = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)     \
         -I$(HEAP_DIR)      \
         -I$(HT_DIV_DIR)    \
         -I$(HT_MUL_DIR)    \
         -I$(DLL_DIR)       \
         -I$(STACK_DIR)     \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = tsp-uint64-main.o                   \
      tsp-uint64.o                        \
      $(GRAPH_DIR)graph-uint64.o          \
      $(HEAP_DIR)heap-uint32.o            \
      $(HT_DIV_DIR)ht-div-uint32.o        \
      $(HT_MUL_DIR)ht-mul-uint64.o        \
      $(DLL_DIR)dll.o                     \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

tsp-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

tsp-uint64-main.o                   : tsp-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
tsp-uint64.o                        : tsp-uint64.h                        \
                                      $(GRAPH_DIR)graph-uint64.h          \
                                      $(HT_MUL_DIR)ht-mul-uint64.h        \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(GRAPH_DIR)graph-uint64.o          : $(GRAPH_DIR)graph-uint64.h          \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(HT_MUL_DIR)ht-mul-uint64.o        : $(HT_MUL_DIR)ht-mul-uint64.h        \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f tsp-uint64 $(OBJ)
