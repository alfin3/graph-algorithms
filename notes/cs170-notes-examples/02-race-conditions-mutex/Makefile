CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

EXE = race-conditions-mutex \
      race-conditions-1     \
      race-conditions-2

OBJ = $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

all                                    : $(EXE)
race-conditions-mutex                  : race-conditions-mutex.c $(OBJ)
race-conditions-1                      : race-conditions-1.c $(OBJ)
race-conditions-2                      : race-conditions-2.c $(OBJ)

$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean

clean :
	rm -f $(EXE) $(OBJ)
