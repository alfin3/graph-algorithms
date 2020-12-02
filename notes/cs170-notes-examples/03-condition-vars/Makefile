CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

EXE = bounded-buffer-mutex       \
      bounded-buffer-cond-vars-1 \
      bounded-buffer-cond-vars-2

SHARED_OBJ = ctimer.o                              \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_CONCUR_DIR)utilities-concur.o

NSHARED_OBJ = bounded-buffer-mutex.o        \
              bounded-buffer-cond-vars-1.o  \
              bounded-buffer-cond-vars-2.o

all                        : $(EXE)
bounded-buffer-mutex       : bounded-buffer-mutex.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
bounded-buffer-cond-vars-1 : bounded-buffer-cond-vars-1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
bounded-buffer-cond-vars-2 : bounded-buffer-cond-vars-2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

bounded-buffer-mutex.o                : ctimer.h                              \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
bounded-buffer-cond-vars-1.o          : ctimer.h                              \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
bounded-buffer-cond-vars-2.o          : ctimer.h                              \
                                        $(UTILS_MEM_DIR)utilities-mem.h       \
                                        $(UTILS_CONCUR_DIR)utilities-concur.h
ctimer.o                              : ctimer.h
$(UTILS_MEM_DIR)utilities-mem.o       : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
