CC = gcc
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra
OBJ = miller-rabin-uint64-main.o miller-rabin-uint64.o $(UTILS_DIR)utilities-ds.o

miller-rabin-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint64-main.o : miller-rabin-uint64.h $(UTILS_DIR)utilities-ds.h
miller-rabin-uint64.o : miller-rabin-uint64.h $(UTILS_DIR)utilities-ds.h 
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean
clean :
	rm $(OBJ)
