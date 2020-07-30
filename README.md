# c-cpp-toolbox

In each directory with a Makefile, run:

`make`

`./executable_name`

`make clean` or `make clean-all`

**Design and implementation highlights:**

`./data_structures_c/heap-uint32/`

A dynamic set of upto 2^32 - 2 generic priority values and generic elements in the (min) heap form, where a priority value is an object of basic type (e.g. char, int, long, double), and an element is any object in memory.

A hash table enables heap search in O(1 + alpha) time in expectation under the simple uniform hashing assumption, where alpha is a constant load factor upper bound. Tests across priority value types are provided with respect to element objects within continuous memory blocks as well as multi-layered element objects in memory.

`./data_structures_c/ht-div-uint32/`

A hash table with generic hash keys and generic elements, where a hash key is an object within a continuous memory block (e.g. a basic type, array, struct), and an element is any object in memory. 

The implementation is based on a division method for hashing into 2^32-1 slots and a chaining method for resolving collisions. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to element objects within continuous memory blocks as well as multi-layered element objects in memory.

`./data_structures_c/utilities-ds/`

The provided implementation of random_range_uint64 and random_range_uint32 demonstrates the construction of uniformly random numbers exceeding the range of a random number generator by conditioning the construction process with a Bernoulli random variable on the lowest bit in the high bits outside the range of the generator.

`./graph_algorithms_c/dfs/`

An emulation of a recursion on a dynamically allocated stack data structure is provided in order to prevent an overflow of the memory stack.
