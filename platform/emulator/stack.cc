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
#include "error.hh"
#include "stack.hh"

void Stack::resizeOutline(int n)
{
  resize(((getMaxSize())*3)/2);  // faster than size*1.5
  ensureFree(n);
}

void Stack::resize(int newSize)
{
#ifdef DEBUG_STACK
  warning("Resizing stack from %d to %d\n",getMaxSize(),newSize);
#endif
  DebugCheck(newSize <= 0,error("Resizing stack <= 0\n"));
  int used = tos-array;
  array = reallocate(array, getMaxSize(), newSize);
  if(!array)
    error("Cannot realloc stack memory at %s:%d.", __FILE__, __LINE__);
  tos      = array+used;
  stackEnd = array+newSize;
}

StackEntry *Stack::reallocate(StackEntry *p, int oldsize, int newsize)
{
  return (StackEntry*) realloc(p,sizeof(StackEntry) * newsize);
}
