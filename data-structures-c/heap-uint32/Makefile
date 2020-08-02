CC = gcc
HT_DIV_UINT32_DIR = ../ht-div-uint32/
DLL_DIR = ../dll/
UTILS_DIR = ../utilities-ds/
CFLAGS = -I$(HT_DIV_UINT32_DIR) -I$(DLL_DIR) -I$(UTILS_DIR) -Wall -Wextra -O3
OBJ = heap-uint32-main.o                  \
      heap-uint32.o                       \
      $(HT_DIV_UINT32_DIR)ht-div-uint32.o \
      $(DLL_DIR)dll.o                     \
      $(UTILS_DIR)utilities-ds.o          \

heap-uint32 : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

heap-uint32-main.o                  : heap-uint32.h                       \
                                      $(UTILS_DIR)utilities-ds.h                     
heap-uint32.o                       : heap-uint32.h                       \
                                      $(HT_DIV_UINT32_DIR)ht-div-uint32.h \
                                      $(UTILS_DIR)utilities-ds.h
$(HT_DIV_UINT32_DIR)ht-div-uint32.o : $(HT_DIV_UINT32_DIR)ht-div-uint32.h \
                                      $(DLL_DIR)dll.h                     \
                                      $(UTILS_DIR)utilities-ds.h
$(DLL_DIR)dll.o                     : $(DLL_DIR)dll.h
$(UTILS_DIR)utilities-ds.o          : $(UTILS_DIR)utilities-ds.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f heap-uint32 $(OBJ)
