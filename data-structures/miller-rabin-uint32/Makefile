CC = gcc
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra -Wno-maybe-uninitialized -O3
OBJ = miller-rabin-uint32-main.o miller-rabin-uint32.o $(UTILS_DIR)utilities-rand-mod.o

miller-rabin-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

miller-rabin-uint32-main.o : miller-rabin-uint32.h $(UTILS_DIR)utilities-rand-mod.h
miller-rabin-uint32.o : miller-rabin-uint32.h $(UTILS_DIR)utilities-rand-mod.h 
$(UTILS_DIR)utilities-rand-mod.o : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)

clean-all : 
	rm -f miller-rabin-uint32 $(OBJ)
