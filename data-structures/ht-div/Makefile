CC = gcc
DLL_DIR = ../dll/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
CFLAGS = -I$(DLL_DIR)                          \
         -I$(UTILS_MEM_DIR)                    \
         -I$(UTILS_MOD_DIR)                    \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = ht-div-main.o                   \
      ht-div.o                        \
      $(DLL_DIR)dll.o                 \
      $(UTILS_MEM_DIR)utilities-mem.o \
      $(UTILS_MOD_DIR)utilities-mod.o

ht-div : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

ht-div-main.o                       : ht-div.h                        \
                                      $(DLL_DIR)dll.h                 \
                                      $(UTILS_MOD_DIR)utilities-mod.h
ht-div.o                            : ht-div.h                        \
                                      $(DLL_DIR)dll.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                 \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o     : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f ht-div $(OBJ)
