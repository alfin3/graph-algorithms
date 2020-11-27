CC = gcc
UTILS_DIR = ../../../utilities-c/utilities-concur/
CFLAGS = -I$(UTILS_DIR) -Wall -Wextra
OBJ = ctimer.o                        \
      $(UTILS_DIR)utilities-concur.o 

all : bounded-buffer-mutex       \
      bounded-buffer-cond-vars-1 \
      bounded-buffer-cond-vars-2

bounded-buffer-mutex : bounded-buffer-mutex.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-1 : bounded-buffer-cond-vars-1.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bounded-buffer-cond-vars-2 : bounded-buffer-cond-vars-2.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread


ctimer.o                        : ctimer.h
$(UTILS_DIR)utilities-concur.o  : $(UTILS_DIR)utilities-concur.h

.PHONY : clean

clean :
	rm -f bounded-buffer-mutex bounded-buffer-cond-vars-1 bounded-buffer-cond-vars-2 $(OBJ)
