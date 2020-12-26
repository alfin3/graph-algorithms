CC = gcc
UTILS_MEM_DIR = ../utilities-mem/
UTILS_RM_DIR = ../utilities-rand-mod/
CFLAGS = -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -Wno-unused-result  -O3

OBJ = utilities-alg-main.o                      \
      utilities-alg.o                           \
      $(UTILS_MEM_DIR)utilities-mem.o           \
      $(UTILS_RM_DIR)utilities-rand-mod.o

utilities-alg : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

utilities-alg-main.o                : utilities-alg.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h 
utilities-alg.o                     : utilities-alg.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-alg $(OBJ)
