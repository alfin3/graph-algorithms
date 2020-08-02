CC = gcc
DLL_DIR = ../dll/
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(DLL_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = ht-div-uint32-main.o ht-div-uint32.o $(DLL_DIR)dll.o $(UTILS_DIR)utilities-ds.o

ht-div-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-div-uint32-main.o : ht-div-uint32.h $(DLL_DIR)dll.h $(UTILS_DIR)utilities-ds.h
ht-div-uint32.o : ht-div-uint32.h $(DLL_DIR)dll.h $(UTILS_DIR)utilities-ds.h
$(DLL_DIR)dll.o : $(DLL_DIR)dll.h
$(UTILS_DIR)utilities-ds.o : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-div-uint32 $(OBJ)
