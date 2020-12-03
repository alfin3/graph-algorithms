/**
   dphil.h

   Declarations of functions for running solutions of the "Dining 
   Philosophers" problem.

   Adopted from https://sites.cs.ucsb.edu/~rich/class/cs170/notes/,
   with added modifications and fixes:
   -  state modifying functions take a state pointer and an integer id of
      a thread as parameters; the thread argument struct is moved to
      dphil-driver.c, where the thread entry function is defined,
   -  some variables were renamed or eliminated; some functions were
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
   Tells a thread with id to perform a pickup operation.
*/
void state_pickup(void *state, int id);

/**
   Tells a thread with id to perform a putdown operation.
*/
void state_putdown(void *state, int id);

#define MAX_NUM_THREADS (25)

#endif
