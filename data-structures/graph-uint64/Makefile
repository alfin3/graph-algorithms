CC = gcc
STACK_DIR = ../stack-uint64/
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(STACK_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = graph-uint64-main.o              \
      graph-uint64.o                   \
      $(STACK_DIR)stack-uint64.o       \
      $(UTILS_DIR)utilities-rand-mod.o

graph-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

graph-uint64-main.o              : graph-uint64.h                   \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
graph-uint64.o                   : graph-uint64.h                   \
                                   $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(STACK_DIR)stack-uint64.o       : $(STACK_DIR)stack-uint64.h       \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(UTILS_DIR)utilities-rand-mod.o : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f graph-uint64 $(OBJ)
