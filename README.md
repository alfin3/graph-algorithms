# Objective

The minimization of the instruction footprint in memory is an objective in resource-constrained environments and may be beneficial in multithreaded settings. Approaches that are scalable across instruction set architectures are desirable.

Graph algorithms may provide a potential benchmark for the minimization of the instruction footprint. Graph algorithms provide i) modularity across supporting data structures, ii) a notion of algorithmic distance (e.g. Dijkstra is closer to Prim than to TSP), and iii) a sufficiently complex code base with dependencies across translation units, enabling evaluation that is meaningful for real applications.

The provided implementation includes i) the construction of an evaluation environment based on a set of graph algorithms, and ii) the evaluation of an approach to the minimization of the instruction footprint, measured as executable size.

# Design and Implementation

An original approach to in-memory alignment enabled cache-efficient type generics. A set of common graph algorithms were decomposed into small type-specific operations, while utilizing the "understanding" of memcpy by modern compilers. The provided algorithms and data structures were extremely portable.

The implementation only used integer and pointer operations (no floating point except in tests). Given parameter values within the specified ranges, the implementation provided an error message and an exit was executed if an integer overflow was attempted or an allocation was not completed due to insufficient resources. This behavior invariant held across all abstraction levels. A detailed specification accompanied the generic interface to ensure the correct usage.

The rigorous compliance with the C standards and adherence to program safety were combined with an exploration of the boundaries of the C language and its translation, which may potentially contribute to further C standardization and enable new mission-critical applications across a broad hardware spectrum.

# Highlights

[graph-algorithms/dfs/README.md](graph-algorithms/dfs/README.md)<br>
[graph-algorithms/dijkstra/README.md](graph-algorithms/dijkstra/README.md)<br>
[graph-algorithms/prim/README.md](graph-algorithms/prim/README.md)<br>
[graph-algorithms/tsp/README.md](graph-algorithms/tsp/README.md)<br>

The relationship between the executable size and the number of types in a top translation unit was evaluated by compiling 20888 executables across the provided DFS, Dijkstra, Prim, and TSP algorithms. The executable size remained mostly flat as the number of types in top translation units increased. The implementation complied rigorously with the C standards, including with respect to types.

The combinatorial generation of top translation units, shown in the provided Makefiles, enabled the compilation and linking of tens of thousands of executables across algorithms, compilers, and optimization levels. Parallel execution significantly sped up the compilation and linking.

<br>

<div align="center">
    <img src="readme/executable-size-vs-num-types/dfs-o3-proc-name.jpg" width="600"/>
</div>

<div align="center">
    <img src="readme/executable-size-vs-num-types/dijkstra-o2-proc-name.jpg" width="600">
</div>

<div align="center">
    <img src="readme/executable-size-vs-num-types/prim-o2-proc-name.jpg" width="600">
</div>

<div align="center">
    <img src="readme/executable-size-vs-num-types/tsp-o3-proc-name.jpg" width="600">
</div>

<br>

[data-structures-pthread/ht-divchn-pthread/README.md](data-structures-pthread/ht-divchn-pthread/README.md)

A multithreaded highly parametrized generic hash table with chaining for resolving collisions was designed and implemented. It is expected to enable the multithreading of graph algorithms, following a similar relationship between the executable size and the number of types in top translation units.

<br>

<div align="center">
    <img src="readme/divchn-eval/i22-k2-2-lock-opt-ht-growth-a-proc-name.jpg" width="600"/>
</div>

<br>

[utilities-pthread/mergesort-pthread/README.md](utilities-pthread/mergesort-pthread/README.md)

A version of the multithreaded mergesort algorithm was designed and implemented. Its highly portable implementation may provide a multithreaded sorting benchmark for CPU manufacturers aiming to improve thread execution capability.

<br>

<div align="center">
    <img src="readme/mergesort-pthread-eval/2a-i3-i7-sbase-mbase.jpg" width="600"/>
</div>

<br>

Additional information related to the style of the provided implementation is available at https://wiki.sei.cmu.edu/confluence/display/c/3+Recommendations.

In each directory with a Makefile, run:

`make` or optionally if available `make BUILD_MODE=M32` or `make BUILD_MODE=M64`

`./executable-name`

`make clean` or `make clean-all`

# Supplemental Information

[readme/supplemental-info/README.md](readme/supplemental-info/README.md)

Graph-conscious hashing was formulated, reflecting the perspective that the topology of a graph informs an optimal hashing method.
