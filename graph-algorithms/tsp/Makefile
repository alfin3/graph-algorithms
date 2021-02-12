CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR      = $(DS_DIR)graph/
HT_MUL_DIR     = $(DS_DIR)ht-mul-uint64/
DLL_DIR        = $(DS_DIR)dll/
STACK_DIR      = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(GRAPH_DIR)     \
         -I$(HT_MUL_DIR)    \
         -I$(DLL_DIR)       \
         -I$(STACK_DIR)     \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = tsp-main.o                          \
      tsp.o                               \
      $(GRAPH_DIR)graph.o                 \
      $(HT_MUL_DIR)ht-mul-uint64.o        \
      $(DLL_DIR)dll.o                     \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

tsp : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

tsp-main.o                          : tsp.h                               \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
tsp.o                               : tsp.h                               \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(HT_MUL_DIR)ht-mul-uint64.h        \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(GRAPH_DIR)graph.o                 : $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(HT_MUL_DIR)ht-mul-uint64.o : $(HT_MUL_DIR)ht-mul-uint64.h \
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
	rm -f tsp $(OBJ)
