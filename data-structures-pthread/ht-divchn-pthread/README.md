A hash table can be modified by threads calling insert, remove, and/or delete operations concurrently. The hash table design provides the following guarantees with respect to the final state of a hash table, which is defined as a pair of i) a load factor, and ii) a set S consisting of sets of key-element pairs, where the number of sets in S is equal to the number of slots in the hash table\*:
- a single final state is guaranteed after concurrent insert, remove, and/or delete operations if the sets of keys used by threads are disjoint,
- a single final state is guaranteed according to a user-defined element comparison function (e.g. min, max)** after concurrent insert operations if the sets of keys used by threads are not disjoint.
   
A hash table always reaches a final state, including the single final state if it is guaranteed, because chaining does not limit the number of insertions. According to the hash table design, a thread completes a batch operation i) before a load factor upper bound is exceeded, ii) before the hash table grows when the load factor bound is temporarily*** exceeded, or iii) after the hash table reaches its maximum count of slots on a given system and the load factor is no longer bounded. In ii) and iii) a thread is guaranteed to complete its operation, because for any load factor upper bound, if it is exceeded, the hash table does not limit the number of insertions due to chaining.

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms, e.g. in multithreaded looping in an adjacency list.

\* each set in S formally includes a unique token

** other reduction functions to be added in future versions (e.g. add, multiply, and, or, xor)

*** unless the growth step that follows does not lower the load factor sufficiently because the maximum count of slots on a given system is reached during the growth step and the load factor is no longer bounded

![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-open.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-closed.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/slot-to-lock-chn.jpg)
