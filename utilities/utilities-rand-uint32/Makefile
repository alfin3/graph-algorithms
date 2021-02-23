CC = gcc
UTILS_MOD_DIR = ../utilities-mod/
CFLAGS = -I$(UTILS_MOD_DIR) \
         -Wall -Wextra -O3

OBJ = utilities-rand-uint32-main.o    \
      utilities-rand-uint32.o         \
      $(UTILS_MOD_DIR)utilities-mod.o

utilities-rand-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

utilities-rand-uint32-main.o        : utilities-rand-uint32.h         \
                                      $(UTILS_MOD_DIR)utilities-mod.h
utilities-rand-uint32.o             : utilities-rand-uint32.h         \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(UTILS_MOD_DIR)utilities-mod.o     : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-rand-uint32 $(OBJ)
