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

#include <stdio.h>

#include "error.h"
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

  // memory management: default via malloc/free
  virtual StackEntry *allocate(int n);
  virtual void deallocate(StackEntry *p, int n);
  virtual StackEntry *reallocate(StackEntry *p, int oldsize, int newsize);

public:

  Stack(int sz = 10000);
  ~Stack() { deallocate(array,size*sizeof(StackEntry)); }

  Bool empty(void) { return (tos <= array) ? OK : NO; }

  void ensureFree(int n)
  {
    if (stackEnd <= tos+n) {
      resize((int)(size*1.5));
      ensureFree(n);
    }
  }
  
  void checkConsistency()
  {
    DebugCheck(((tos < array) || (tos > array+size)),
	       error("Stack inconsistent"));
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
    DebugCheck(empty() == OK, error("Cannot pop from empty stack."););
    tos -= n;
    return *tos;
  }

  int getMaxSize()  { return size; }
  int getUsed()     { return tos-array; }
};


#endif //__STACK_H__
