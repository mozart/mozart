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

#include "stack.hh"

void Stack::realloc(int n)
{
  int used = tos-array;
  array = (StackEntry*) ::realloc(array, sizeof(StackEntry) * (size += 10000));
  if(!array)
    error("Cannot realloc stack memory at %s:%d.", __FILE__, __LINE__);
  tos = array+used;
  stackEnd= array+size;
  ensureFree(n);
}
