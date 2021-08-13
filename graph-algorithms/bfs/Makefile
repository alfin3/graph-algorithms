#
#  Instructions for making BFS tests according to an optional user-
#  provided build mode.
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

DS_DIR        = ../../data-structures/
GRAPH_DIR     = $(DS_DIR)graph/
QUEUE_DIR     = $(DS_DIR)queue/
STACK_DIR     = $(DS_DIR)stack/
UTILS_MEM_DIR = ../../utilities/utilities-mem/
UTILS_MOD_DIR = ../../utilities/utilities-mod/
CFLAGS = -I$(GRAPH_DIR)                               \
         -I$(QUEUE_DIR)                               \
         -I$(STACK_DIR)                               \
         -I$(UTILS_MEM_DIR)                           \
         -I$(UTILS_MOD_DIR)                           \
         ${CFLAGS_BUILD_MODE} -Wall -Wextra -flto -O3

OBJ = bfs-test.o                      \
      bfs.o                           \
      $(GRAPH_DIR)graph.o             \
      $(QUEUE_DIR)queue.o             \
      $(STACK_DIR)stack.o             \
      $(UTILS_MEM_DIR)utilities-mem.o \
      $(UTILS_MOD_DIR)utilities-mod.o 

bfs-test : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

bfs-test.o                      : bfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h
bfs.o                           : bfs.h                           \
                                  $(GRAPH_DIR)graph.h             \
                                  $(QUEUE_DIR)queue.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(GRAPH_DIR)graph.o             : $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(QUEUE_DIR)queue.o             : $(QUEUE_DIR)queue.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o : $(UTILS_MOD_DIR)utilities-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f bfs-test $(OBJ)
