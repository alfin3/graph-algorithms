CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

EXE = race-conditions-mutex \
      race-conditions-1     \
      race-conditions-2

SHARED_OBJ = $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

NSHARED_OBJ = race-conditions-mutex.o \
              race-conditions-1.o     \
              race-conditions-2.o

all                   : $(EXE)
race-conditions-mutex : race-conditions-mutex.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
race-conditions-1     : race-conditions-1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
race-conditions-2     : race-conditions-2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

race-conditions-mutex.o               : $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
race-conditions-1.o                   : $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
race-conditions-2.o                   : $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
