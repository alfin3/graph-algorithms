CC = gcc
HT_DIV_UINT32_DIR = ../ht-div-uint32/
DLL_DIR = ../dll/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_RM_DIR = ../../utilities/utilities-rand-mod/
CFLAGS = -I$(HT_DIV_UINT32_DIR) \
         -I$(DLL_DIR)           \
         -I$(UTILS_MEM_DIR)     \
         -I$(UTILS_RM_DIR)      \
         -Wall -Wextra -O3

OBJ = heap-uint32-main.o                  \
      heap-uint32.o                       \
      $(HT_DIV_UINT32_DIR)ht-div-uint32.o \
      $(DLL_DIR)dll.o                     \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_RM_DIR)utilities-rand-mod.o

heap-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

heap-uint32-main.o                  : heap-uint32.h                       \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h                     
heap-uint32.o                       : heap-uint32.h                       \
                                      $(HT_DIV_UINT32_DIR)ht-div-uint32.h \
                                      $(UTILS_RM_DIR)utilities-rand-mod.h
$(HT_DIV_UINT32_DIR)ht-div-uint32.o : $(HT_DIV_UINT32_DIR)ht-div-uint32.h \
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
	rm -f heap-uint32 $(OBJ)
