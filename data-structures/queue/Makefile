CC = gcc
UTILS_MEM_DIR = ../../utilities/utilities-mem/
CFLAGS = -I$(UTILS_MEM_DIR) -Wall -Wextra -O3
OBJ = queue-main.o queue.o $(UTILS_MEM_DIR)utilities-mem.o

queue : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

queue-main.o                    : queue.h $(UTILS_MEM_DIR)utilities-mem.o
queue.o                         : queue.h $(UTILS_MEM_DIR)utilities-mem.o
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f queue $(OBJ)
