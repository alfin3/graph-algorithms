CC = gcc
DLL_DIR = ../dll/
UTILS_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(DLL_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = ht-div-uint64-main.o       \
      ht-div-uint64.o            \
      $(DLL_DIR)dll.o            \
      $(UTILS_DIR)utilities-rand-mod.o

ht-div-uint64 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-div-uint64-main.o             : ht-div-uint64.h                  \
                                   $(DLL_DIR)dll.h                  \
                                   $(UTILS_DIR)utilities-rand-mod.h
ht-div-uint64.o                  : ht-div-uint64.h                  \
                                   $(DLL_DIR)dll.h                  \
                                   $(UTILS_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                  : $(DLL_DIR)dll.h
$(UTILS_DIR)utilities-rand-mod.o : $(UTILS_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-div-uint64 $(OBJ)
