
A hash table is accessed by threads calling insert, remove, and/or delete operations concurrently. The design provides the following guarantees:

- the final state of the hash table is guaranteed with respect to concurrent insert, remove, and/or delete operations if the key sets between threads are disjoint
- if insert operations are called by more than one thread concurrently and the key sets are not disjoint, then the final state of the hash table is guaranteed according to an insertion predicate (e.g. max, min, sum of key-associated elements)
- because chaining does not limit the number of insertions, each thread that passed the first critical section is guaranteed to complete its batch operation before the hash table grows, although a load factor bound may be exceeded to the extent dependent on the number of threads that passed the first critical section and their batch sizes

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms, e.g. in multithreaded looping across an adjacency list.

![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-open.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-closed.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/slot-to-lock-chn.jpg)
