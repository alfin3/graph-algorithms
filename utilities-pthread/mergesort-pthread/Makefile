CC = gcc
UTILS_ALG_DIR = ../../utilities/utilities-alg/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_PTH_DIR = ../utilities-pthread/
CFLAGS = -I$(UTILS_ALG_DIR)                             \
         -I$(UTILS_MEM_DIR)                             \
         -I$(UTILS_PTH_DIR)                             \
         -std=c90 -pthread -Wall -Wextra -Wpedantic -O3

OBJ = mergesort-pthread-main.o            \
      mergesort-pthread.o                 \
      $(UTILS_ALG_DIR)utilities-alg.o     \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_PTH_DIR)utilities-pthread.o

mergesort-pthread : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

mergesort-pthread-main.o            : mergesort-pthread.h                  \
                                      $(UTILS_MEM_DIR)utilities-mem.h
mergesort-pthread.o                 : mergesort-pthread.h                  \
                                      $(UTILS_ALG_DIR)utilities-alg.h      \
                                      $(UTILS_PTH_DIR)utilities-pthread.h  \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_ALG_DIR)utilities-alg.o     : $(UTILS_ALG_DIR)utilities-alg.h      \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTH_DIR)utilities-pthread.o : $(UTILS_PTH_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f mergesort-pthread $(OBJ)
