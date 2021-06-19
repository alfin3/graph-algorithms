
A hash table is modified by threads calling insert, remove, and/or delete operations concurrently. The design provides the following guarantees with respect to the final state of a hash table, defined as a pair of i) a load factor, and ii) the set of sets of key-element pairs, where each set is the set of key-element pairs at a slot of a hash table:
- a single final state is guaranteed with respect to concurrent insert, remove, and/or delete operations if the sets of keys used by threads are disjoint,
- if insert operations are called by more than one thread concurrently and the sets of keys used by threads are not disjoint, then a single final state of the hash table is guaranteed according to a user-defined reduction function (e.g. min, max, add, multiply, and, or, xor of key-associated elements),
- because chaining does not limit the number of insertions, each thread that passed the first critical section is guaranteed to complete its batch operation before the hash table grows, although a load factor upper bound (alpha) is temporarily* exceeded**.

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms, e.g. in multithreaded looping in an adjacency list.

\* unless the maximum representable count of hash table slots was reached and the growth step did not lower the load factor below alpha, in which case alpha no longer provides a load factor upper bound

** to the extent dependent on the number of threads that passed the first critical section and their batch sizes

![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-open.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-closed.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/slot-to-lock-chn.jpg)
