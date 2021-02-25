CC = gcc
UTILS_MEM_DIR = ../utilities-mem/
UTILS_MOD_DIR = ../utilities-mod/
CFLAGS = -I$(UTILS_MEM_DIR) \
         -I$(UTILS_MOD_DIR) \
         -Wall -Wextra -O3

OBJ = utilities-rand-uint64-main.o    \
      utilities-rand-uint64.o         \
      $(UTILS_MEM_DIR)utilities-mem.o \
      $(UTILS_MOD_DIR)utilities-mod.o

utilities-rand-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

utilities-rand-uint64-main.o        : utilities-rand-uint64.h         \
                                      $(UTILS_MEM_DIR)utilities-mem.h \
                                      $(UTILS_MOD_DIR)utilities-mod.h
utilities-rand-uint64.o             : utilities-rand-uint64.h         \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o     : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-rand-uint64 $(OBJ)
