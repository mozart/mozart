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
public:

  Stack(int sz = 10000) {
    size = sz;
    array = (StackEntry*) malloc(sizeof(StackEntry) * size);
    if(!array)
      error("Cannot alloc stack memory at %s:%d.", __FILE__, __LINE__);
    tos = array;
    stackEnd = array+size;
  }

  Bool empty(void){
    return (tos <= array) ? OK : NO;
  }

  void realloc(int n);
  
  void ensureFree(int n)
  {
    if (stackEnd <= tos+n) {
      realloc(n);
    }
  }
  
  void push(StackEntry value, Bool check=OK)
  {
    if ( check ) {
      ensureFree(1);
    }
    *tos = value;
    tos++;
  };

  StackEntry pop(void)
  {
    DebugCheck(empty() == OK,
	       error("Cannot pop from empty stack."););
    tos--;
    return *tos;
  };
}; // Stack


#endif //__STACK_H__
