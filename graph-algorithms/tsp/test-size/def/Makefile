#
# Combinatorial generation of top translation units, followed by compilation
# and linking to evaluate executable sizes.
#
# 217 type sets for vertices and weights result in the compilation of 217 executables.
#
# Changing the compiler and optimization level enables evaluating their effect on
# executable sizes across type sets in top translation units.
#
# The use of "make -jN" for parallel execution significantly speeds up compilation
# and linking. N may be the number of logical cores on a system.
#
# Optionally, on x86-64 processors in 64-bit environments, BUILD_MODE=M32
# or BUILD_MODE=M64 can be used, which may require "apt-get install gcc-multilib".
#
# Usage example:
#   make -j24
#   make clean
#   make clean-all
#

UPPER_DIR         = ../../../../
UTILS_SIZE_VT_DIR = $(UPPER_DIR)utilities-test-size/vt-types/
UTILS_SIZE_WT_DIR = $(UPPER_DIR)utilities-test-size/wt-types/

VT_FILES := $(wildcard $(UTILS_SIZE_VT_DIR)*.c)
WT_FILES := $(filter-out $(UTILS_SIZE_WT_DIR)wt-compt%,\
              $(wildcard $(UTILS_SIZE_WT_DIR)*.c))

iter_vt_wt = $(foreach vt_path,$(1),\
               $(call iter_wt_helper,$(2),$(vt_path)))

iter_wt_helper = $(foreach wt_path,$(1),\
                   $(call gen_src_name_vpath_wpath,\
                     $(join tsp-$(basename $(notdir $(2)))-,$(notdir $(wt_path))),$(2),$(wt_path)))

gen_src_name_vpath_wpath = $(file >$(1))\
                           $(file >>$(1),#include "start-end/tsp-test-start.c")\
                           $(file >>$(1),#include "$(2)")\
                           $(file >>$(1),#include "$(3)")\
                           $(file >>$(1),#include "start-end/tsp-test-end.c")\
                           $(1)

GEN_FILES   := $(call iter_vt_wt,$(VT_FILES),$(WT_FILES))
GEN_TARGETS := $(basename $(GEN_FILES))
GEN_OBJECTS := $(addsuffix .o,$(GEN_TARGETS))

# Prepare compilation.

BUILD_MODE            = DEF
CFLAGS_BUILD_MODE_M64 = -std=c90 -m64 -Wpedantic
CFLAGS_BUILD_MODE_M32 = -std=c90 -m32 -Wpedantic
CFLAGS_BUILD_MODE_DEF = -std=c90 -Wpedantic
CFLAGS_BUILD_MODE     = $(CFLAGS_BUILD_MODE_$(BUILD_MODE))

CC = gcc # clang

TSP_DIR       = $(UPPER_DIR)graph-algorithms/tsp/
DS_DIR        = $(UPPER_DIR)data-structures/
GRAPH_DIR     = $(DS_DIR)graph/
STACK_DIR     = $(DS_DIR)stack/
UTILS_MEM_DIR = $(UPPER_DIR)utilities/utilities-mem/
UTILS_MOD_DIR = $(UPPER_DIR)utilities/utilities-mod/
UTILS_LIM_DIR = $(UPPER_DIR)utilities/utilities-lim/

CFLAGS = -I$(TSP_DIR)                                 \
         -I$(GRAPH_DIR)                               \
         -I$(STACK_DIR)                               \
         -I$(UTILS_MEM_DIR)                           \
         -I$(UTILS_MOD_DIR)                           \
         -I$(UTILS_LIM_DIR)                           \
         ${CFLAGS_BUILD_MODE} -Wall -Wextra -flto -O3 # or other optimization

LOWER_OBJECTS = $(TSP_DIR)tsp.o                 \
                $(GRAPH_DIR)graph.o             \
                $(STACK_DIR)stack.o             \
                $(UTILS_MEM_DIR)utilities-mem.o \
                $(UTILS_MOD_DIR)utilities-mod.o

# Compile using static pattern rules.

.PHONY : all clean clean-all

all : $(GEN_TARGETS)

$(GEN_TARGETS) : %: %.o $(LOWER_OBJECTS)
	$(CC) $(CFLAGS) $< $(LOWER_OBJECTS) -o $@

$(GEN_OBJECTS)                  : %.o: %.c                        \
                                  $(TSP_DIR)tsp.h                 \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h \
                                  $(UTILS_LIM_DIR)utilities-lim.h
$(TSP_DIR)tsp.o                 : $(TSP_DIR)tsp.h                 \
                                  $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h \
                                  $(UTILS_MOD_DIR)utilities-mod.h \
                                  $(UTILS_LIM_DIR)utilities-lim.h
$(GRAPH_DIR)graph.o             : $(GRAPH_DIR)graph.h             \
                                  $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(STACK_DIR)stack.o             : $(STACK_DIR)stack.h             \
                                  $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MEM_DIR)utilities-mem.o : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_MOD_DIR)utilities-mod.o : $(UTILS_MOD_DIR)utilities-mod.h \
                                  $(UTILS_LIM_DIR)utilities-lim.h

clean :
	rm -f *.c *.o $(LOWER_OBJECTS)
clean-all :
	rm -f tsp-vt* $(LOWER_OBJECTS)
