CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -Wall -Wextra

EXC = bounded-buffer-mutex       \
      bounded-buffer-cond-vars-1 \
      bounded-buffer-cond-vars-2

OBJ = ctimer.o                              \
      $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

all : $(EXC)

bounded-buffer-mutex : bounded-buffer-mutex.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-1 : bounded-buffer-cond-vars-1.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-2 : bounded-buffer-cond-vars-2.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread


ctimer.o                               : ctimer.h
$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean

clean :
	rm -f $(EXC) $(OBJ)
