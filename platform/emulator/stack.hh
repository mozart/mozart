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

#ifdef __GNUC__
#pragma interface
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include "error.hh"
#include "types.hh"

//*****************************************************************************
//                Definition of class Stack
//*****************************************************************************


typedef void* StackEntry;

class Stack {
protected:
  int size;
  StackEntry *tos;   // top of stack: pointer to first UNUSED cell
  StackEntry *array;
  StackEntry *stackEnd;

  virtual void resize(int newSize);
  void resizeOutline(int newSize);

  // memory management: default via malloc/free
  void allocate(int sz, void *(*allocfun)(size_t t));
  virtual void deallocate(StackEntry *p, int n);
  virtual StackEntry *reallocate(StackEntry *p, int oldsize, int newsize);

public:

  Stack(int sz = 1000, void *(*allocfun)(size_t t) = malloc);

  virtual ~Stack() { deallocate(array,size); }

  Bool isEmpty(void) { return (tos <= array); }
  StackEntry *ensureFree(int n)
  {
    StackEntry *ret = tos;
    if (stackEnd <= tos+n) {
      resizeOutline(n);
      ret = tos;
    }
    return ret;
  }

  void checkConsistency()
  {
    Assert((tos >= array) && (tos <= array+size));
  }

  void push(StackEntry value, Bool check=OK)
  {
    checkConsistency();
    if (check) { ensureFree(1); }
    *tos = value;
    tos++;
  };

  StackEntry pop(int n=1)
  {
    checkConsistency();
    Assert(isEmpty() == NO);
    tos -= n;
    return *tos;
  }

  int getMaxSize()  { return size; }
  int getUsed()     { return tos-array; }
};


#endif //__STACK_H__
