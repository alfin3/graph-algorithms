CC = gcc
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra
OBJ = miller-rabin-uint32-main.o miller-rabin-uint32.o $(UTILS_DIR)utilities-ds.o

miller-rabin-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint32-main.o : miller-rabin-uint32.h $(UTILS_DIR)utilities-ds.h
miller-rabin-uint32.o : miller-rabin-uint32.h $(UTILS_DIR)utilities-ds.h 
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean
clean :
	rm $(OBJ)
