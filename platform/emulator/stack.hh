/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
*/

#ifndef __STACK_H__
#define __STACK_H__

#ifdef INTERFACE
#pragma interface
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include "mem.hh"

//*****************************************************************************
//                Definition of class Stack
//*****************************************************************************

typedef void* StackEntry;

typedef enum {
  Stack_WithMalloc,
  Stack_WithFreelist
} StackAllocation;

class Stack {
protected:
  StackEntry *tos;   // top of stack: pointer to first UNUSED cell
  StackEntry *array;
  StackEntry *stackEnd;

  StackAllocation stkalloc;
  void resize(int newSize);

  void reallocate(int newsize);

  void deallocate(StackEntry *p, int n)
  {
    if (stkalloc==Stack_WithMalloc)
      free(p);
    else
      freeListDispose(p, n*sizeof(StackEntry));
  }

  void allocate(int sz, int alloc)
  {
    int auxsz = sz*sizeof(StackEntry);
    array = alloc==Stack_WithMalloc
         ? (StackEntry*)malloc(auxsz):(StackEntry*)freeListMalloc(auxsz);
    if(array==NULL)
      error("Cannot alloc stack memory at %s:%d.", __FILE__, __LINE__);
    tos = array;
    stackEnd = array+sz;
  }

  void allocate(int sz) { allocate(sz,stkalloc); }


public:
  Stack(int sz, StackAllocation alloc) : stkalloc(alloc) { allocate(sz,alloc); }
  ~Stack() { deallocate(array,stackEnd-array); }

  void mkEmpty(void) { tos = array; }
  Bool isEmpty(void) { return (tos <= array); }
  StackEntry *ensureFree(int n)
  {
    StackEntry *ret = tos;
    if (stackEnd <= tos+n) {
      resize(n);
      ret = tos;
    }
    return ret;
  }
  void checkConsistency()
  {
    Assert((tos >= array) && (tos <= stackEnd));
  }

  void push(StackEntry value, Bool check=OK)
  {
    checkConsistency();
    if (check) { ensureFree(1); }
    *tos = value;
    tos++;
  }
  StackEntry pop(int n=1)
  {
    checkConsistency();
    Assert(isEmpty() == NO);
    tos -= n;
    return *tos;
  }

  int getMaxSize()  { return (stackEnd-array); }
  int getUsed()     { return (tos-array); }
};


#endif //__STACK_H__
