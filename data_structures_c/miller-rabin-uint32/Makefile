CC = gcc
UTIL_DIR = ../utilities-ds/
CFLAGS = -I$(UTIL_DIR) -Wall -Wextra
OBJ = miller-rabin-uint32-main.o miller-rabin-uint32.o $(UTIL_DIR)utilities-ds.o

miller-rabin-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint32-main.o : miller-rabin-uint32.h $(UTIL_DIR)utilities-ds.h
miller-rabin-uint32.o : $(UTIL_DIR)utilities-ds.h 
$(UTIL_DIR)utilities-ds.o : $(UTIL_DIR)utilities-ds.h

.PHONY : clean
clean :
	rm $(OBJ)
