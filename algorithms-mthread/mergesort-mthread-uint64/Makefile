CC = gcc
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra -O3

OBJ = mergesort-mthread-uint64-main.o       \
      mergesort-mthread-uint64.o            \
      $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

mergesort-mthread-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

mergesort-mthread-uint64-main.o        : mergesort-mthread-uint64.h \
                                         $(UTILS_MEM_DIR)utilities-mem.h
mergesort-mthread-uint64.o             : mergesort-mthread-uint64.h \
                                         $(UTILS_MEM_DIR)utilities-mem.h       \
                                         $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f mergesort-mthread-uint64 $(OBJ)
