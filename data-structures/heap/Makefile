CC = gcc
HT_DIV_UINT64_DIR = ../ht-div-uint64/
HT_MUL_UINT64_DIR = ../ht-mul-uint64/
DLL_DIR = ../dll/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
CFLAGS = -I$(HT_DIV_UINT64_DIR) \
         -I$(HT_MUL_UINT64_DIR) \
         -I$(DLL_DIR)           \
         -I$(UTILS_MEM_DIR)     \
         -I$(UTILS_MOD_DIR)      \
         -std=c90 -Wall -Wextra -Wpedantic -O3

OBJ = heap-main.o                         \
      heap.o                              \
      $(HT_DIV_UINT64_DIR)ht-div-uint64.o \
      $(HT_MUL_UINT64_DIR)ht-mul-uint64.o \
      $(DLL_DIR)dll.o                     \
      $(UTILS_MEM_DIR)utilities-mem.o     \
      $(UTILS_MOD_DIR)utilities-mod.o

heap : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

heap-main.o                         : heap.h                              \
                                      $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(UTILS_MEM_DIR)utilities-mem.h
heap.o                              : heap.h                              \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(HT_DIV_UINT64_DIR)ht-div-uint64.o : $(HT_DIV_UINT64_DIR)ht-div-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(HT_MUL_UINT64_DIR)ht-mul-uint64.o : $(HT_MUL_UINT64_DIR)ht-mul-uint64.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h     \
                                      $(UTILS_MOD_DIR)utilities-mod.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h                     \
                                      $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o     : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_RM_DIR)utilities-mod.o      : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f heap $(OBJ)
