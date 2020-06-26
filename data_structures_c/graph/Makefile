CC = gcc
STACK_DIR = ../stack/
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(STACK_DIR) -I$(UTILS_DIR) -Wall -Wextra
OBJ = graph-main.o graph.o $(STACK_DIR)stack.o $(UTILS_DIR)utilities-ds.o

graph : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

graph-main.o : graph.h $(STACK_DIR)stack.h $(UTILS_DIR)utilities-ds.h
graph.o : graph.h $(STACK_DIR)stack.h 
$(STACK_DIR)stack.o : $(STACK_DIR)stack.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)

clean-all : 
	rm -f graph $(OBJ)
