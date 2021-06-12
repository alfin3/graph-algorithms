Growth synchronization of hash tables based on chaining for resolving collisions.

A hash table is accessed by threads calling insert, remove, and/or delete operations concurrently. The design provides the following guarantees:

- the final state of the hash table is guaranteed with respect to concurrent insert, remove, and/or delete operations if there is no key overlap between threads
- if insert operations are called by more than one thread and the keys overlap, then the final state of the hash table is guaranteed according to an insertion predicate (e.g. max, min, sum of key-associated elements)
- because chaining does not limit the number of insertions, each thread is guaranteed to complete its operations, although a load factor bound may be exceeded

The provided design and associated guarantees are suited for the use of hash tables in multithreaded graph algorithms.

![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-open.jpg)
![alt text](https://github.com/alfin3/graph-algorithms/blob/master/readme/divchn-diag-gate-closed.jpg)
