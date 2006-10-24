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

#if defined(INTERFACE)
#pragma implementation "stack.hh"
#endif

#include "base.hh"
#include "mem.hh"
#include "stack.hh"

#include <stdlib.h>

inline
void Stack::reallocate(int newsize) 
{
  StackEntry *savearray = array;
  int oldsize = getMaxSize();

  allocate(newsize);

  memcpy(array, savearray, oldsize * sizeof(StackEntry));
  
  if (stkalloc==Stack_WithMalloc)
    free(savearray);
  else
    oz_freeListDispose(savearray,sizeof(StackEntry)*oldsize);
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

#define FST_FIRST_SIZE 2048
#define FST_MARGIN     4

FastStack::FastStack(void) {
  first = (StackEntry *) malloc(FST_FIRST_SIZE * sizeof(StackEntry));
}

void FastStack::init(void) {
  start = first;
  tos   = first;
  end   = first + (FST_FIRST_SIZE - FST_MARGIN);
}

void FastStack::exit(void) {
  if (start != first)
    free(start);
}

void FastStack::resize(void) {
  int s  = (end - start) + FST_MARGIN;
  int u  = tos - start;
  int ns = (s * 3) >> 1;
  StackEntry * nst = (StackEntry *) malloc(ns * sizeof(StackEntry));
  memcpy(nst, start, u*sizeof(StackEntry));
  if (start != first)
    free(start);
  start = nst;
  tos   = nst + u;
  end   = nst + (ns - FST_MARGIN);
}

