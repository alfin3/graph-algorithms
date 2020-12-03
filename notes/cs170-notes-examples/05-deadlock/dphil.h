/**
   dphil.h

   Declarations of functions for running solutions of the "Dining 
   Philosophers" problem.

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters; the thread argument struct is moved to
      dphil-driver.c, where the thread entry function is defined,
   -  some variables are renamed or eliminated; some functions are
      renamed.
*/

#ifndef DPHIL_H
#define DPHIL_H

/**
   Creates a new state for handling thread synchronization wrt the
   pickup and putdown operations.
*/
void *state_new(int num_phil);

/**
   Disposes a state for thread synchronization, freeing memory resources.
   Currently achieved with Ctrl+C while the driver is looping.
*/

/**
   Performs a pickup operation.
*/
void state_pickup(void *state, int id);

/**
   Performs a putdown operation.
*/
void state_putdown(void *state, int id);

#define MAX_NUM_THREADS (25)

#endif
