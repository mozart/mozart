/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifdef __GNUC__
#pragma implementation "stack.hh"
#endif

#include <malloc.h>
#include "stack.hh"

void Stack::resize(int newSize)
{
  DebugCheckT(printf("Resizing stack from %d to %d\n",size,newSize));
  DebugCheck(newSize <= 0,error("Resizing stack <= 0\n"));
  int used = tos-array;
  array = reallocate(array, size, newSize);
  if(!array)
    error("Cannot realloc stack memory at %s:%d.", __FILE__, __LINE__);
  size     = newSize;
  tos      = array+used;
  stackEnd = array+size;
}


Stack::Stack(int sz, void *(*allocfun)(size_t t))
{
  size = sz;
  array = (StackEntry*) allocfun(size*sizeof(StackEntry));
  if(!array)
    error("Cannot alloc stack memory at %s:%d.", __FILE__, __LINE__);
  tos = array;
  stackEnd = array+size;
}



void Stack::deallocate(StackEntry *p, int n)
{
  free(p);
}

StackEntry *Stack::reallocate(StackEntry *p, int oldsize, int newsize)
{
  return (StackEntry*) realloc(p,sizeof(StackEntry) * newsize);
}
