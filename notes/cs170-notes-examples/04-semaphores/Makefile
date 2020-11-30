CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

OBJ = ctimer.o                              \
      $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

bounded-buffer-semaphores              : bounded-buffer-semaphores.c $(OBJ)

ctimer.o                               : ctimer.h
$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean

clean :
	rm -f bounded-buffer-semaphores $(OBJ)
