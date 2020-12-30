CC = gcc
UTILS_MEM_DIR = ../../utilities/utilities-mem/
CFLAGS = -I$(UTILS_MEM_DIR) -Wall -Wextra -O3
OBJ = stack-main.o stack.o $(UTILS_MEM_DIR)utilities-mem.o

stack : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

stack-main.o                    : stack.h $(UTILS_MEM_DIR)utilities-mem.o
stack.o                         : stack.h $(UTILS_MEM_DIR)utilities-mem.o
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f stack $(OBJ)
