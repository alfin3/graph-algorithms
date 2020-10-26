CC = gcc
STACK_DIR = ../stack-uint64/
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(STACK_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = graph-uint64-main.o        \
      graph-uint64.o             \
      $(STACK_DIR)stack-uint64.o \
      $(UTILS_DIR)utilities-ds.o

graph-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

graph-uint64-main.o        : graph-uint64.h             \
                             $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
graph-uint64.o             : graph-uint64.h             \
                             $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
$(STACK_DIR)stack-uint64.o : $(STACK_DIR)stack-uint64.h \
                             $(UTILS_DIR)utilities-ds.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f graph-uint64 $(OBJ)
