CC = gcc
DS_DIR = ../../data-structures/
GRAPH_DIR         = $(DS_DIR)graph/
HT_DIV_UINT64_DIR = $(DS_DIR)ht-div-uint64/
HT_MUL_UINT64_DIR = $(DS_DIR)ht-mul-uint64/
DLL_DIR           = $(DS_DIR)dll/
STACK_DIR         = $(DS_DIR)stack/
UTILS_MEM_DIR     = ../../utilities/utilities-mem/
UTILS_MOD_DIR      = ../../utilities/utilities-mod/
CFLAGS = -I$(GRAPH_DIR)                        \
         -I$(HT_DIV_UINT64_DIR)                \
         -I$(HT_MUL_UINT64_DIR)                \
         -I$(DLL_DIR)                          \
         -I$(STACK_DIR)                        \
         -I$(UTILS_MEM_DIR)                    \
         -I$(UTILS_MOD_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = tsp-main.o                          \
      tsp.o                               \
      $(GRAPH_DIR)graph.o                 \
      $(HT_DIV_UINT64_DIR)ht-div-uint64.o \
      $(HT_MUL_UINT64_DIR)ht-mul-uint64.o \
      $(DLL_DIR)dll.o                     \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_MOD_DIR)utilities-mod.o

tsp : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

tsp-main.o                          : tsp.h                               \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h
tsp.o                               : tsp.h                               \
                                      $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(GRAPH_DIR)graph.o                 : $(GRAPH_DIR)graph.h                 \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(HT_DIV_UINT64_DIR)ht-div-uint64.o : $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(HT_MUL_UINT64_DIR)ht-mul-uint64.o : $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o     : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f tsp $(OBJ)
