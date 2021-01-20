CC = gcc
UTILS_MEM_DIR = ../../utilities/utilities-mem/
CFLAGS = -I$(UTILS_MEM_DIR) -Wall -Wextra -O3
OBJ = dll-main.o dll.o $(UTILS_MEM_DIR)utilities-mem.o

dll : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

dll-main.o                      : dll.h $(UTILS_MEM_DIR)utilities-mem.o
dll.o                           : dll.h $(UTILS_MEM_DIR)utilities-mem.o
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f dll $(OBJ) 
