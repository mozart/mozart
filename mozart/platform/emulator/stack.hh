/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __STACK_H__
#define __STACK_H__

#ifdef INTERFACE
#pragma interface
#endif

#include <stdlib.h>
#include <stdio.h>

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
      oz_freeListDispose(p, n*sizeof(StackEntry));
  }
  
  void allocate(int sz, int alloc)
  {
    int auxsz = sz*sizeof(StackEntry);
    array = alloc==Stack_WithMalloc 
         ? (StackEntry*)malloc(auxsz):(StackEntry*)oz_freeListMalloc(auxsz);
    Assert(array);
    tos = array;
    stackEnd = array+sz;
  }

  void allocate(int sz) { allocate(sz,stkalloc); }
  

public:
  Stack(int sz, StackAllocation alloc) : stkalloc(alloc) { allocate(sz,alloc); }
  ~Stack() { deallocate(array,stackEnd-array); }

  void dispose(void) { deallocate(array,stackEnd-array); }
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

  StackEntry topElem() { return *(tos-1); }

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

class FastStack {
private:
  StackEntry * first, * start, * tos, * end;
  void resize(void);
public:
  void init(void);
  void exit(void);
  FastStack(void);
  ~FastStack(void) {}
  int isEmpty(void) {
    return start==tos;
  }
  void push1(StackEntry e1) {
    StackEntry * t = tos;
    *(t+0) = e1;
    t += 1;
    tos = t;
    if (t > end) resize();
  }
  void push2(StackEntry e1, StackEntry e2) {
    StackEntry * t = tos;
    *(t+0) = e1;
    *(t+1) = e2;
    t += 2;
    tos = t;
    if (t > end) resize();
  }
  void pop1(StackEntry &e1) {
    StackEntry * t = tos-1;
    tos = t;
    e1 = *(t);
  }
  void pop2(StackEntry &e1, StackEntry &e2) {
    StackEntry * t = tos-2;
    tos = t;
    e2 = *(t+1);
    e1 = *(t+0);
  }
};

#endif //__STACK_H__
