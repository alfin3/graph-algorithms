CC = gcc
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = queue-uint64-main.o queue-uint64.o $(UTILS_DIR)utilities-ds.o

queue-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

queue-uint64-main.o                 : queue-uint64.h $(UTILS_DIR)utilities-ds.o
queue-uint64.o                      : queue-uint64.h $(UTILS_DIR)utilities-ds.o
$(UTILS_DIR)utilities-ds.o          : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f queue-uint64 $(OBJ)
