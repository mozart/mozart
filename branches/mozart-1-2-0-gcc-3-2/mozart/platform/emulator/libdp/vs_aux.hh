/*
 *  Authors:
 *    Konstantin Popov
 * 
 *  Contributors:
 *
 *  Copyright:
 *    Konstantin Popov 1997-1998
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __VS_AUX_HH
#define __VS_AUX_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "base.hh"

#ifdef VIRTUALSITES

#include "stack.hh"

//
// Fixed-size stack of integers. Used e.g. for keeping free chunks in
// a segment;
class FixedSizeStack {
private:
  int size;
  // 'bottom' is a pointer to the first cell, and
  // 'limit'  is a pointer to the first unusable cell,
  // 'tos' is a pointer to a next free cell;
  int *bottom, *limit, *tos;

  //
public:
  FixedSizeStack(int sizeIn) : size(sizeIn) {
    bottom = (int *) malloc(sizeof(int) * size);
    limit = bottom + size;
    tos = bottom;
  }
  ~FixedSizeStack() {
    free(bottom);
    DebugCode(bottom = limit = tos = (int *) -1);
  }

  //
  Bool isEmpty() {
    Assert(tos >= bottom);
    return ((Bool) (tos == bottom));
  }
  int getSize() { return (tos-bottom); }
  void push(int i) {
    Assert(tos < limit);
    *tos++ = i;
  }
  int pop() { 
    Assert(!isEmpty());
    tos--;
    return (*tos);
  }
  void purge() { tos = bottom; }
};

//
// A stack of (void*), whose elements can be also accessed aka in an
// array. Used e.g. for keeping track of chunk pool segments by the
// chunk pool manager;
class PtrStackArray : private Stack {
public:
  //
  PtrStackArray() : Stack(128, Stack_WithMalloc) {}
  ~PtrStackArray() {}

  //
  // "stack" interface;
  int getSize() { return (Stack::getUsed()); }
  void push(void *p) { Stack::push((StackEntry) p); }
  void *pop() { return ((void *) Stack::pop()); }

  //
  // "array" interface;
  void* operator[] (int elem) {
    Assert(elem < getSize());
    return (*(array + elem));
  }
};

#endif // VIRTUALSITES
#endif // __VS_AUX_HH

