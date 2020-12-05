CC = gcc
QUEUE_DIR = ../../../data-structures-c/queue-uint64/
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
UTILS_DS_DIR = ../../../data-structures-c/utilities-ds/
CFLAGS = -I$(QUEUE_DIR)        \
         -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -I$(UTILS_DS_DIR)     \
         -pthread -Wall -Wextra

EXE = dphil-deadlock-1      \
      dphil-deadlock-2      \
      dphil-deadlock-free-1 \
      dphil-deadlock-free-2 \
      dphil-deadlock-free-3

OBJ_SHARED = dphil-driver.o                        \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

OBJ_NSHARED = dphil-deadlock-1.o            \
              dphil-deadlock-2.o            \
              dphil-deadlock-free-1.o       \
              dphil-deadlock-free-2.o       \
              dphil-deadlock-free-3.o       \
              $(QUEUE_DIR)queue-uint64.o    \
              $(UTILS_DS_DIR)utilities-ds.o
              

all                   : $(EXE)
dphil-deadlock-1      : dphil-deadlock-1.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-2      : dphil-deadlock-2.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-free-1 : dphil-deadlock-free-1.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-free-2 : dphil-deadlock-free-2.o $(OBJ_SHARED)
	$(CC) $(CFLAGS) -o $@ $^
dphil-deadlock-free-3 : dphil-deadlock-free-3.o       \
                        $(QUEUE_DIR)queue-uint64.o    \
                        $(UTILS_DS_DIR)utilities-ds.o \
                        $(OBJ_SHARED)
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
dphil-deadlock-free-3.o               : dphil.h                               \
                                        $(QUEUE_DIR)queue-uint64.h            \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
dphil-driver.o                        : dphil.h                               \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
$(QUEUE_DIR)queue-uint64.o            : $(QUEUE_DIR)queue-uint64.h                        \
                                        $(UTILS_DS_DIR)utilities-ds.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h
$(UTILS_DS_DIR)utilities-ds.o         : $(UTILS_DS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ_SHARED) $(OBJ_NSHARED)
clean-all :
	rm -f $(EXE) $(OBJ_SHARED) $(OBJ_NSHARED)
