/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "perdiovar.hh"
#endif

#include "am.hh"
#include "genvar.hh"

void handleAsk(TaggedRef *pvar, TaggedRef other)
{
  Assert(isPerdioVar(*pvar));

  RefsArray args=allocateRefsArray(2,NO);
  args[0] = makeTaggedRef(pvar);
  if (other==makeTaggedNULL()) {
    args[1] = OZ_atom("ask");
  } else {
    DEREF(other,otherPtr,_1);
    if (isPerdioVar(other)) {
      args[1] = OZ_mkTupleC("askUnify",1,makeTaggedRef(otherPtr));
    } else {
      args[1] = OZ_atom("ask");
    }
  }
}

Bool PerdioVar::unifyPerdioVar(TaggedRef * vptr, TaggedRef * tptr, Bool prop)
{
}

Bool PerdioVar::valid(TaggedRef *varPtr, TaggedRef v)
{
  Assert(!isRef(v) && !isAnyVar(v));

  handleAsk(varPtr);

  return TRUE;
}


//-----------------------------------------------------------------------------
// Implementation of interface functions

OZ_C_proc_begin(PerdioVar_is, 2)
{
  return OZ_unify(OZ_getCArg(1),
                  isPerdioVar(deref(OZ_getCArg(0)))?NameTrue:NameFalse);
}
OZ_C_proc_end

// ---------------------------------------------------------------------
// Distributed stuff
// ---------------------------------------------------------------------


static
BIspec pvarSpec[] = {
  {"PerdioVar.is",            2, PerdioVar_is},

  {0,0,0,0}
};

void BIinitPerdioVar()
{
  BIaddSpec(pvarSpec);
}
