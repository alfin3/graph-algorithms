CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra -O2

EXE = avg-single-thread \
      avg-mult-thread

SHARED_OBJ = $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

NSHARED_OBJ = avg-single-thread.o \
              avg-mult-thread.o

all               : $(EXE)
avg-single-thread : avg-single-thread.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
avg-mult-thread   : avg-mult-thread.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

avg-single-thread.o                   : $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
avg-mult-thread.o                     : $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
