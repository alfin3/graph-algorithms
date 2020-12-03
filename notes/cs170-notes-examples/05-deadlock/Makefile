CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

EXE = dphil-deadlock-1

SHARED_OBJ = dphil-driver.o                        \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

NSHARED_OBJ = dphil-deadlock-1.o

all              : $(EXE)
dphil-deadlock-1 : dphil-deadlock-1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

dphil-deadlock-1.o                     : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-driver.o                        : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
