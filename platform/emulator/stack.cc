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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
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


