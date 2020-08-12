# c-cpp-toolbox

In each directory with a Makefile, run:

`make`

`./executable-name`

`make clean` or `make clean-all`

**Design and implementation highlights:**

`./data-structures-c/heap-uint32/`

A dynamic set of upto 2^32 - 2 generic priority values and generic elements in the (min) heap form, where a priority value is an object of basic type (e.g. char, int, long, double), and an element is any object in memory.

A hash table enables heap search in O(1 + alpha) time in expectation under the simple uniform hashing assumption, where alpha is a constant load factor upper bound. Tests across priority value types are provided with respect to element objects within continuous memory blocks as well as multi-layered element objects in memory.

`./data-structures-c/ht-mul-uint64/`

A hash table with generic hash keys and generic elements, where a hash key is an object within a continuous memory block (e.g. a basic type, array, struct), and an element is any object in memory.

The implementation is based on a multiplication method for hashing into upto 2^63 slots and an open addressing method for resolving collisions. The expected number of probes in a search is upper-bounded by 1/(1 - alpha) under the uniform hashing assumption, where alpha is a load factor upper bound. For every delete or remove operation at most one rehashing operation is performed, resulting in an overhead per delete or remove operation bounded by 1/(1 - alpha) in expectation. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to element objects within continuous memory blocks as well as multi-layered element objects in memory.

`./data-structures-c/ht-div-{uint32, uint64}/`

A hash table with generic hash keys and generic elements, where a hash key is an object within a continuous memory block (e.g. a basic type, array, struct), and an element is any object in memory. 

The implementation is based on a division method for hashing into upto {2^32 - 1,  2^64 - 1} slots and a chaining method for resolving collisions. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to element objects within continuous memory blocks as well as multi-layered element objects in memory.

`./data-structures-c/utilities-ds/`

random_range_{uint32, uint64} demonstrate the construction of uniformly random numbers exceeding the range of a random number generator by conditioning the construction process with a Bernoulli random variable on the lowest bit in the high bits outside the range of the generator.

mem_mod_{uint32, uint64} compute the modulo operation on a memory block by treating each byte of the block in the little-endian order and inductively applying the pertinent modular arithmetic relations, without requiring a little-endian machine. fast_mem_mod_{uint32, uint64} treat a memory block in 8-byte increments in the little-endian order. Given a little-endian machine, the result is equal to the return value of mem_mod_{uint32, uint64}.

`./graph-algorithms-c/dfs/`

An emulation of a recursion on a dynamically allocated stack data structure is provided in order to prevent an overflow of the memory stack.
