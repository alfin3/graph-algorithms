
## Evaluation - December 11, 2021 implementation and June 3, 2024 formatting changes

The relationship between the executable size and the number of types in a top translation unit was evaluated. The evaluation was performed on a 13th Generation Intel(R) Core(TM) i7-13700HX Processor (i7-13700HX).

A top translation unit contained a test routine on random graphs over a set of vertex types. Let L = \{unsigned short, unsigned int, unsigned long\}. In the top translation unit, the set of vertex types was a non-empty subset of L.

Given a compiler and optimization level, the number of the compiled executables was the number of non-empty subsets in the powerset of L. The compiled executables included the executables compiled with a single vertex type. Two versions of Clang and two versions of GCC were used to detect differences in the sizes of the compiled executables.

The executable size remained mostly flat as the number of types in top translation units increased. The implementation complied rigorously with the C89/C90 and C99 standards, including with respect to types. The widths of the integer types were from 2\*\*4 to 2\*\*6 bits.

The combinatorial generation of top translation units, shown in the Makefile in `test-size`, enabled the compilation and linking of tens of thousands of executables across algorithms, compilers, and optimization levels. Parallel execution significantly sped up compilation and linking.

<br>

<div align="center">
    <img src="../../readme/executable-size-vs-num-types/dfs-o2-proc-name.jpg" width="600"/>
</div>

<div align="center">
    <img src="../../readme/executable-size-vs-num-types/dfs-o3-proc-name.jpg" width="600">
</div>

<br>

The executables across 7 type sets of vertices can be compiled and linked in `test-size` as follows:

`make -jN`<br>
`make clean` or `make clean-all`

where `N` may be the number of logical cores on a system.

For questions or comments, please reach out through github.

##
*A i7-13700HX Linux machine was used for the above evaluation.*