CC = gcc
DLL_DIR = ../dll/
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(DLL_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = ht-mul-uint64-main.o       \
      ht-mul-uint64.o            \
      $(DLL_DIR)dll.o            \
      $(UTILS_DIR)utilities-ds.o

ht-mul-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-mul-uint64-main.o       : ht-mul-uint64.h            \
                             $(DLL_DIR)dll.h            \
                             $(UTILS_DIR)utilities-ds.h
ht-mul-uint64.o            : ht-mul-uint64.h            \
                             $(DLL_DIR)dll.h            \
                             $(UTILS_DIR)utilities-ds.h
$(DLL_DIR)dll.o            : $(DLL_DIR)dll.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-mul-uint64 $(OBJ)
