/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "cpi.hh"

// check stream for validity, init closed and eostr, init nextGet and nextPut
void OZ_Stream::setFlags(void) 
{
  closed = eostr = FALSE;
  valid = TRUE;
  OZ_Term t = tail;

  DEREF(t, tptr, ttag);
  
  if (isNil(t)) {
    eostr = closed = TRUE;
    return;
  } else if (isNotCVar(ttag)) {
    eostr = TRUE;
    return;
  } else if (isCons(t)) {
    return;
  }
  valid = FALSE;
  eostr = closed = TRUE;
}

OZ_Term OZ_Stream::get(void)
{
  if (closed || eostr) {
    return 0;
  } 

  OZ_Term deref_tail = deref(tail);
  OZ_Term r = head(deref_tail);
  tail = makeTaggedRef(tailRef(deref_tail));
  setFlags();
  return r;
}

OZ_Boolean OZ_Stream::leave(void) 
{
  while (!closed && !eostr) {
    if (!valid) 
      return FALSE;
    get();
  }
  if (closed) 
    return FALSE ;

  DEREF(tail, tailptr, tailtag);

  addSuspAnyVar(tailptr, am.currentThread);
  return TRUE;
}

void OZ_Stream::fail(void) 
{
}

// End of File
//-----------------------------------------------------------------------------
