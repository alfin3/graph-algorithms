CC = gcc
STACK_DIR = ../stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(STACK_DIR) -I$(UTILS_MEM_DIR) -I$(UTILS_RM_DIR) -Wall -Wextra -O3
OBJ = graph-uint64-main.o                 \
      graph-uint64.o                      \
      $(STACK_DIR)stack.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

graph-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

graph-uint64-main.o                 : graph-uint64.h                      \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
graph-uint64.o                      : graph-uint64.h                      \
                                      $(STACK_DIR)stack.h                 \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(STACK_DIR)stack.o                 : $(STACK_DIR)stack.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f graph-uint64 $(OBJ)
