CC = gcc
DLL_DIR = ../dll/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(DLL_DIR)       \
         -I$(UTILS_MEM_DIR) \
         -I$(UTILS_RM_DIR)  \
         -Wall -Wextra -O3

OBJ = ht-div-uint32-main.o                \
      ht-div-uint32.o                     \
      $(DLL_DIR)dll.o                     \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

ht-div-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-div-uint32-main.o                : ht-div-uint32.h                     \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
ht-div-uint32.o                     : ht-div-uint32.h                     \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.o
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-rand-mod.o : $(UTILS_RM_DIR)utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-div-uint32 $(OBJ)
