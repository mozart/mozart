/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "stack.hh"
#endif

#include <malloc.h>

#include "types.hh"
#include "mem.hh"
#include "stack.hh"

inline
void Stack::reallocate(int newsize)
{
  StackEntry *savearray = array;
  int oldsize = getMaxSize();

  allocate(newsize);

  for (int i=0; i < oldsize; i++) {
    array[i] = savearray[i];
  }

  if (stkalloc==Stack_WithMalloc)
    free(savearray);
  else
    freeListDispose(savearray,sizeof(StackEntry)*oldsize);
}



/* resize by at least "incSize" */
void Stack::resize(int incSize)
{
  Assert(incSize > 0);
  int allocsize = max((getMaxSize()*3)/2,RESIZESTACKMINSIZE);

#ifdef DEBUG_STACK
  message("Resizing stack from %d to %d\n",getMaxSize(),allocsize);
#endif
  int used = tos-array;
  reallocate(allocsize);
  tos      = array+used;

  ensureFree(incSize);
}
