#
#  Instructions for making graph tests according to an optional user-provided
#  build mode.
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

STACK_DIR     = ../stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
UTILS_LIM_DIR = ../../utilities/utilities-lim/
CFLAGS = -I$(STACK_DIR)                               \
         -I$(UTILS_MEM_DIR)                           \
         -I$(UTILS_MOD_DIR)                           \
         -I$(UTILS_LIM_DIR)                           \
         ${CFLAGS_BUILD_MODE} -Wall -Wextra -flto -O3
OBJ = graph-test.o                    \
      graph.o                         \
      $(STACK_DIR)stack.o             \
      $(UTILS_MEM_DIR)utilities-mem.o \
      $(UTILS_MOD_DIR)utilities-mod.o


graph-test : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

graph-test.o                     : graph.h                        \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h \
                                  $(UTILS_LIM_DIR)utilities-lim.h
graph.o                         : graph.h                         \
                                  $(STACK_DIR)stack.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o : $(UTILS_MOD_DIR)utilities-mod.h \
                                  $(UTILS_LIM_DIR)utilities-lim.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all :
	rm -f graph-test $(OBJ)
