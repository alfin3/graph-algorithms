# graph-algorithms

Graph algorithms and supporting data structures with a hash table parameter, generic (any type) edge weights, low memory footprint, and general C89/C90 and C99 portability across systems without conditional compilation.

The presented approach reflects the perspective that graph problems often reduce to hashing problems, where an ordered set of a graph topology, an algorithm, and a hash function maps to a distribution of hash values. Information about a graph topology can often be obtained from a problem statement and/or the prior knowledge of a domain. Additional information, such as the indegree of each vertex in a graph or a subset of vertices, can be stored at the time of graph construction. Based on the preliminary data, the provided implementation motivates the development and evaluation of graph-conscious hashing methods.

It is anticipated that the following examples of graph-conscious hashing methods will be evaluated in the near future:

- multiplication and division constant numbers in multiplication and division-based hash tables will be (automatically) selected to minimize collisions among the most frequent hash keys as determined at the time of graph construction,
- a contiguous buffer for storing the key element pairs of frequent hash keys, as determined at the time of graph construction or dynamically during an algorithm run, will be maintained in order to decrease the probability of eviction of the cache blocks with frequent hash keys and their elements spanned by the buffer throughout an algorithm run, across LRU and LRU-related replacement policies (i.e. a frequent hash key helps keep another frequent hash key and its element in cache).


The approach may be particularly suitable for computing and optimizing the exact solutions of small instances of NP-hard problems in memory-constrained environments. Small instances of NP-hard problems may require little space as graphs in memory. However, computing an exact solution may require extensive memory resources without hashing. The provided division and multiplication-based hash tables, portable under C89/C90 and C99, enable the hashing of contiguous blocks of memory thereby accommodating the hashing of sets of vertices in exact solutions of NP-hard problems.

Compilation was completed with gcc 7.5 in a 64-bit environment with and without -m32. Vectorization and cache efficiency are being profiled, which may result in implementation changes. Additional information relating to the style of the provided implementations is available at https://wiki.sei.cmu.edu/confluence/display/c/3+Recommendations.

In each directory with a Makefile, run:

`make` or optionally if available `make BUILD_MODE=M32` or `make BUILD_MODE=M64`

`./executable-name`

`make clean` or `make clean-all`

**Highlights:**

`./data-structures-pthread/ht-divchn-pthread/`

A hash table with generic hash keys and generic elements that is concurrently accessible and modifiable. The implementation is based on a division method for hashing and a chaining method for resolving collisions.

A hash table is modified by threads calling insert, remove, and/or delete operations concurrently. The design provides the following guarantees with respect to the final state of a hash table, defined as a pair of i) a load factor, and ii) the set of sets of key-element pairs for each slot of the hash table, after all operations are completed:
- a single final state is guaranteed with respect to concurrent insert, remove, and/or delete operations if the sets of keys used by threads are disjoint,
- if insert operations are called by more than one thread concurrently and the sets of keys used by threads are not disjoint, then a single final state of the hash table is guaranteed according to a user-defined reduction function (e.g. min, max, add, multiply, and, or, xor of key-associated elements),
- because chaining does not limit the number of insertions, each thread is theoretically guaranteed to complete its batch operation i) before a load factor upper bound (alpha) is exceeded, ii) before the hash table grows as a part of a called insert operation when alpha is temporarily exceeded, or iii) after the hash table reaches its maximum count of slots on a given system and alpha no longer bounds the load factor.

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms, e.g. in multithreaded looping in an adjacency list. Tests across load factor upper bounds and key sizes exceeding the basic types are provided with respect to elements within contiguous and noncontiguous memory blocks. The implementation requires that CHAR_BIT * sizeof(size_t) is greater or equal to 16 and is even, as well as pthreads API.

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

`./utilities-pthread/mergesort-pthread/`

A merge sort algorithm with parallel sorting and parallel merging for sorting arrays of generic elements with \Theta(n/log^{2}n) theoretical parallelism within the dynamic multithreading model.

The implementation provides i) a set of parameters for setting the constant base case upper bounds for switching from parallel sorting to serial sorting and from parallel merging to serial merging during recursion, and ii) a macro for setting the constant upper bound for the number of recursive calls placed on the stack of a thread across sorting and merging operations, thereby enabling the optimization of the parallelism and concurrency-associated overhead across input ranges and hardware settings. 


