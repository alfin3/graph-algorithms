
A hash table is modified by threads calling insert, remove, and/or delete operations concurrently. The design provides the following guarantees with respect to the final state of a hash table, defined as a pair of i) a load factor, and ii) the set of sets of key-element pairs for each slot of the hash table:
- a single final state is guaranteed with respect to concurrent insert, remove, and/or delete operations if the sets of keys used by threads are disjoint,
- if insert operations are called by more than one thread concurrently and the sets of keys used by threads are not disjoint, then a single final state of the hash table is guaranteed according to a user-defined reduction function (e.g. min, max, add, multiply, and, or, xor of key-associated elements),
- because chaining does not limit the number of insertions, each thread is theoretically guaranteed to complete its batch operation i) before a load factor upper bound (alpha) is exceeded, ii) before the hash table grows and while alpha is temporarily* exceeded**, or iii) after the hash table reached its maximum count of slots on a given system and alpha no longer bounds the load factor.

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms, e.g. in multithreaded looping in an adjacency list.

\* unless the growth step that follows does not lower the load factor below alpha because the maximum count of slots is reached during the growth step, in which case alpha no longer bounds the load factor

** to the extent dependent on the number of threads that passed the first critical section and their batch sizes

![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-open.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-closed.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/slot-to-lock-chn.jpg)
