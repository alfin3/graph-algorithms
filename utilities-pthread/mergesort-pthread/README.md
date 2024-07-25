Merge sort algorithm with decoupled parallel sorting and parallel merging during recursion. The algorithm provides \Theta(n/log^{2}n) theoretical parallelism within the dynamic multithreading model.

The design decouples merge and sort parallelisms for optimization purposes.

The implementation provides i) a set of parameters for setting the constant base case upper bounds for switching from parallel sorting to serial sorting and from parallel merging to serial merging during recursion, and ii) a macro for setting the constant upper bound for the number of recursive calls placed on the stack of a thread across sorting and merging operations, thereby enabling the optimization of the parallelism and the associated overhead across input ranges and hardware settings.

The elements of an input array are generic and the sorting is performed according to a user-defined comparison function.

On a machine with 24 logical cores, with a 13th Gen Intel(R) Core(TM) i7-13700HX Processor with Hyper-Threading on 8 of 16 physical cores, the initial optimization of the base case upper bound parameters resulted in speedups of approximately 11.22X and 9.67X in comparison to serial qsort (stdlib.h) on arrays of 2**28 random integer and double elements respectively.

<div align="center">
    <img src="../../readme/mergesort-pthread-15.png"/>
    <img src="../../readme/mergesort-pthread-20.png"/>
</div>
