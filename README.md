# data-structures-algorithms-c

Graph algorithms and supporting data structures with generic (any type) edge weights, a hash table parameter, low memory footprint, and general C89/C90 portability across systems without conditional compilation.

The implementation provides i) potential space and speed advantages relative to the use of C++ abstractions in settings with limited memory resources, and ii) parameters for hashing-based optimization across graph topologies and hardware settings.

A two-step approach was used to minimize the memory space occupied and used by a program: generic programming and hashing parametrization. The approach reflects the perspective that graph problems often reduce to hashing problems, where the topology of a graph informs an optimal hashing method. 

Generic division and multiplication-based hash tables are provided and are portable under C89/C90 with the only requirement that `CHAR_BIT * sizeof(size_t)` is greater or equal to 16 and is even.

Compilation was completed with gcc 7.5 in a 64-bit environment with and without -m32. Vectorization and cache efficiency have not yet been profiled, which may result in implementation changes. Additional information relating to the style of the provided implementations is available at https://wiki.sei.cmu.edu/confluence/display/c/3+Recommendations. The conversion of tests to C89/C90 is work in progress.

In each directory with a Makefile, run:

`make` or optionally if available `make BUILD_MODE=M32` or `make BUILD_MODE=M64`

`./executable-name`

`make clean` or `make clean-all`

**Highlights:**

`./graph-algorithms/tsp`

An exact solution of TSP without vertex revisiting on graphs with generic weights with a hash table parameter.

The algorithm provides O(2^n n^2) asymptotic run time, where n is the number of vertices in a tour, as well as tour existence detection. A bit array representation enables time and space efficient set membership and union operations over O(2^n) sets.
   
The hash table parameter specifies a hash table used for set hashing operations, and enables the optimization of the associated space and time resources by choice of a hash table and its load factor upper bound. If NULL is passed as a hash table parameter value, a default hash table is used, which contains an array with a count that is equal to n * 2^n, where n is the number of vertices in a graph. If E >> V and V < `CHAR_BIT * sizeof(size_t)`, a default hash table may provide speed advantages by avoiding the computation of hash values. If V is larger and the graph is sparse, a non-default hash table may provide space advantages. Tests across i) default, division-based, and multiplication-based hash tables, as well as ii) edge weight types are provided.

`./graph-algorithms/{dijkstra, prim}`

Dijkstra's and Prim’s algorithms on graphs with generic weights with a hash table parameter.

The hash table parameter specifies a hash table used for in-heap operations, and enables the optimization of space and time resources associated with heap operations in the algorithm routines by choice of a hash table and its load factor upper bound. If NULL is passed as a hash table parameter value, a default hash table is used, which contains an index array with a count that is equal to the number of vertices in the graph. If E >> V, a default hash table may provide speed advantages by avoiding the computation of hash values. If V is large and the graph is sparse, a non-default hash table may provide space advantages. Tests across i) default, division-based, and multiplication-based hash tables, as well as ii) edge weight types are provided.

`./data-structures/heap/`

A generic (min) heap with a hash table parameter. The implementation provides a dynamic set in the min heap form for contiguous and noncontiguous elements in memory associated with priority values of basic type (e.g. char, int, long, double).

The hash table parameter specifies a hash table used for in-heap search and modifications, and enables the optimization of space and time resources associated with heap operations by choice of a hash table and its load factor upper bound. Tests across i) division- and mutliplication-based hash tables, ii) contiguous and noncontiguous elements, and iii) priority types are provided.

`./data-structures/ht-mul/`

A hash table with generic hash keys and generic elements, where a hash key is an object within a contiguous memory block (e.g. a basic type, array, struct), and an element is within a contiguous or noncontiguous memory block.

The implementation is based on a multiplication method for hashing and double hashing as an open addressing strategy for resolving collisions. The expected number of probes in a search is upper-bounded by 1/(1 - alpha) under the uniform hashing assumption, where alpha is a load factor upper bound. For every delete and remove operation at most one (re)insert operation is performed, resulting in a probing overhead per delete and remove operation bounded by 1/(1 - alpha) in expectation, and thus a 2/(1 – alpha) overall upper bound for the expected number of probes per delete and remove operation by linearity of expectation. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to elements within contiguous and noncontiguous memory blocks. The implementation requires that `CHAR_BIT * sizeof(size_t)` is greater or equal to 16 and is even.

`./data-structures/ht-div/`

A hash table with generic hash keys and generic elements, where a hash key is an object within a contiguous memory block (e.g. a basic type, array, struct), and an element is within a contiguous or noncontiguous memory block. 

The implementation is based on a division method for hashing and a chaining method for resolving collisions. Due to chaining, the number of keys and elements that can be inserted is not limited by the hash table implementation. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to elements within contiguous and noncontiguous memory blocks. The implementation requires that `CHAR_BIT * sizeof(size_t)` is greater or equal to 16 and is even.

`./utilities/utilities-mod/`

Utility functions in the area of modular arithmetic generalized to size_t. mem_mod computes the modulo operation on a memory block by treating each byte of the block in the little-endian order and inductively applying modular arithmetic relations, without requiring a little-endian machine. fast_mem_mod treats a memory block in sizeof(size_t)-byte increments. Given a little-endian machine, the result is equal to the return value of mem_mod. The implementation requires that `CHAR_BIT * sizeof(size_t)` is even.
