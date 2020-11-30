CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra -O2

EXE = avg-single-thread \
      avg-mult-thread

OBJ = $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

all                                   : $(EXE)
avg-single-thread                     : avg-single-thread.c $(OBJ)
avg-mult-thread                       : avg-mult-thread.c $(OBJ)

$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean

clean :
	rm -f $(EXE) $(OBJ)
