CC = gcc
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = stack-uint64-main.o stack-uint64.o $(UTILS_DIR)utilities-ds.o

stack-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

stack-uint64-main.o                 : stack-uint64.h $(UTILS_DIR)utilities-ds.o
stack-uint64.o                      : stack-uint64.h $(UTILS_DIR)utilities-ds.o
$(UTILS_DIR)utilities-ds.o          : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f stack-uint64 $(OBJ)
