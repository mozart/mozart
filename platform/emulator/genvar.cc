/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#if defined(__GNUC__)
#pragma implementation "genvar.hh"
#endif

#include "fdgenvar.hh"
#include "am.hh"

Bool GenCVariable::unifyGenCVariables = OK;


GenCVariable::GenCVariable(TypeOfGenCVariable t, TaggedRef pn,
                           Board * n)
:type(t), SVariable(n==NULL ? am.currentBoard : n, pn){}


Bool GenCVariable::isLocalVariable(void){
  Board * home = getHome1();

  return (home == am.currentBoard ||
          home->getBoardDeref() == am.currentBoard)
    ? OK : NO;
}


void GenCVariable::bind(TaggedRef * vPtr, TaggedRef var, Bool varIsLocal,
                        TaggedRef * tPtr, TaggedRef term)
{
  if (varIsLocal == NO)
    am.trail.pushRef(vPtr, var);

  if (isCVar(term) == OK)
    term = makeTaggedRef(tPtr);

  *vPtr = term;
}


void GenCVariable::bind(TaggedRef * vPtr, TaggedRef var,
                        TaggedRef * tPtr, TaggedRef term)
{
  bind(vPtr, var,
       tagged2CVar(var)->isLocalVariable(),
       tPtr, term);
}


void GenCVariable::propagate(TaggedRef var, TaggedRef term){
  am.checkSuspensionList(var, term);
}


void GenCVariable::propagate(TaggedRef var, TaggedRef *tPtr){
  am.checkSuspensionList(var, makeTaggedRef(tPtr));
}


SuspList * GenCVariable::propagate(TaggedRef var, SuspList * &sl,
                                   TaggedRef term)
{
  sl = am.checkSuspensionList(tagged2SuspVar(var), var, sl, term);
}


#ifdef OUTLINE
#define inline
#include "genvar.icc"
#undef inline
#endif
