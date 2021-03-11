CC = gcc
UTILS_MEM_DIR = ../utilities-mem/
CFLAGS = -I$(UTILS_MEM_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = utilities-mod-main.o            \
      utilities-mod.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o

utilities-mod : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

utilities-mod-main.o                : utilities-mod.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
utilities-mod.o                     : utilities-mod.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-mod $(OBJ)
