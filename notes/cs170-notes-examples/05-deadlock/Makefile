CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

EXE = dphil-deadlock-1      \
      dphil-deadlock-2      \
      dphil-deadlock-free-1 \
      dphil-deadlock-free-2

OBJ_SHARED = dphil-driver.o                        \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

OBJ_NSHARED = dphil-deadlock-1.o      \
              dphil-deadlock-2.o      \
              dphil-deadlock-free-1.o \
              dphil-deadlock-free-2.o

all                   : $(EXE)
dphil-deadlock-1      : dphil-deadlock-1.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-2      : dphil-deadlock-2.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-free-1 : dphil-deadlock-free-1.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-free-2 : dphil-deadlock-free-2.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^

dphil-deadlock-1.o                    : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-deadlock-2.o                    : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-deadlock-free-1.o               : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-deadlock-free-2.o               : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-driver.o                        : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(OBJ_SHARED) $(OBJ_NSHARED)
clean-all :
	rm -f $(EXE) $(OBJ_SHARED) $(OBJ_NSHARED)
