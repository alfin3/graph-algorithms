CC = gcc
UTILS_MEM_DIR = ../../../utilities/utilities-mem/
UTILS_CONCUR_DIR = ../../../utilities/utilities-concur/
CFLAGS = -I$(UTILS_MEM_DIR)    \
         -I$(UTILS_CONCUR_DIR) \
         -pthread -Wall -Wextra

OBJ = bounded-buffer-semaphores.o           \
      ctimer.o                              \
      $(UTILS_MEM_DIR)utilities-mem.o       \
      $(UTILS_CONCUR_DIR)utilities-concur.o

bounded-buffer-semaphores : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

bounded-buffer-semaphores.o            : ctimer.h                              \
                                         $(UTILS_MEM_DIR)utilities-mem.h       \
                                         $(UTILS_CONCUR_DIR)utilities-concur.h
ctimer.o                               : ctimer.h
$(UTILS_MEM_DIR)utilities-mem.o        : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_CONCUR_DIR)utilities-concur.o  : $(UTILS_CONCUR_DIR)utilities-concur.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bounded-buffer-semaphores $(OBJ)
