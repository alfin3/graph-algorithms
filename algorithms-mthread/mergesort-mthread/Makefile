CC = gcc
UTILS_ALG_DIR = ../../utilities/utilities-alg/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_ALG_DIR)    \
         -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra -O3

OBJ = mergesort-mthread-main.o              \
      mergesort-mthread.o                   \
      $(UTILS_ALG_DIR)utilities-alg.o       \
      $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

mergesort-mthread : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

mergesort-mthread-main.o               : mergesort-mthread.h                   \
                                         $(UTILS_MEM_DIR)utilities-mem.h
mergesort-mthread.o                    : mergesort-mthread.h                   \
                                         $(UTILS_ALG_DIR)utilities-alg.h       \
                                         $(UTILS_CONCUR_DIR)utilities-concur.h \
                                         $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_ALG_DIR)utilities-mem.o        : $(UTILS_ALG_DIR)utilities-alg.h       \
                                         $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f mergesort-mthread $(OBJ)
