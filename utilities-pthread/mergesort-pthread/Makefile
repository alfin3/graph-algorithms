#
#  Instructions for making tests for mergesort with parallel sorting and
#  parallel merging according to an optional user-provided build mode.
#
#  On x86-64 processors in 64-bit environments, the use of a non-default
#  build mode may require "apt-get install gcc-multilib".
#
#  Additional information is available at:
#  https://gcc.gnu.org/onlinedocs/gcc/Submodel-Options.html#Submodel-Options
#  https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#x86-Options
#   
#  usage examples:
#    make
#    make BUILD_MODE=M32
#    make BUILD_MODE=M64
#

BUILD_MODE = DEF
CFLAGS_BUILD_MODE_M64 = -std=c90 -m64 -Wpedantic
CFLAGS_BUILD_MODE_M32 = -std=c90 -m32 -Wpedantic
CFLAGS_BUILD_MODE_DEF = -std=c90 -Wpedantic
CFLAGS_BUILD_MODE = ${CFLAGS_BUILD_MODE_${BUILD_MODE}}
CC = gcc

UTILS_ALG_DIR  = ../../utilities/utilities-alg/
UTILS_MEM_DIR  = ../../utilities/utilities-mem/
UTILS_PTHD_DIR = ../utilities-pthread/
CFLAGS = -I$(UTILS_ALG_DIR)                             \
         -I$(UTILS_MEM_DIR)                             \
         -I$(UTILS_PTHD_DIR)                            \
         -std=c90 -pthread -Wall -Wextra -Wpedantic -O3

OBJ = mergesort-pthread-test.o             \
      mergesort-pthread.o                  \
      $(UTILS_ALG_DIR)utilities-alg.o      \
      $(UTILS_MEM_DIR)utilities-mem.o      \
      $(UTILS_PTHD_DIR)utilities-pthread.o

mergesort-pthread : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

mergesort-pthread-test.o             : mergesort-pthread.h                  \
                                       $(UTILS_MEM_DIR)utilities-mem.h
mergesort-pthread.o                  : mergesort-pthread.h                  \
                                       $(UTILS_ALG_DIR)utilities-alg.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h \
                                       $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_ALG_DIR)utilities-alg.o      : $(UTILS_ALG_DIR)utilities-alg.h      \
                                       $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o      : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTHD_DIR)utilities-pthread.o : $(UTILS_PTHD_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f mergesort-pthread $(OBJ)
