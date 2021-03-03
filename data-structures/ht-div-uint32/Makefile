CC = gcc
DLL_DIR = ../dll/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
CFLAGS = -I$(DLL_DIR)       \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_MOD_DIR)  \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = ht-div-uint32-main.o                \
      ht-div-uint32.o                     \
      $(DLL_DIR)dll.o                     \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_MOD_DIR)utilities-mod.o

ht-div-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-div-uint32-main.o                : ht-div-uint32.h                     \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
ht-div-uint32.o                     : ht-div-uint32.h                     \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h 
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o     : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-div-uint32 $(OBJ)
