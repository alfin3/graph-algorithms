CC = gcc
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = queue-uint64-main.o queue-uint64.o $(UTILS_DIR)utilities-rand-mod.o

queue-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

queue-uint64-main.o                 : queue-uint64.h $(UTILS_DIR)utilities-rand-mod.h
queue-uint64.o                      : queue-uint64.h $(UTILS_DIR)utilities-rand-mod.h
$(UTILS_DIR)utilities-rand-mod.o    : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f queue-uint64 $(OBJ)
