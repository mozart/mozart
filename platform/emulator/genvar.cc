/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "fdgenvar.hh"
#include "am.hh"

Bool GenCVariable::unifyGenCVariables = OK;


GenCVariable::GenCVariable(TypeOfGenCVariable t, TaggedRef pn,
			   Board *n)
:type(t), SVariable(n==NULL ? Board::GetCurrent() : n, pn){}


Bool GenCVariable::isLocalVariable(void){
  Board *home = getHome1();
  
  return (home == Board::GetCurrent() ||
	  home->getBoardDeref() == Board::GetCurrent() )
    ? OK : NO;
}


void GenCVariable::bind(TaggedRef *vPtr, TaggedRef var,
			TaggedRef *tPtr, TaggedRef term){
  if (tagged2CVar(var)->isLocalVariable() == NO)
    am.trail.pushRef(vPtr, var);

  if (isCVar(term) == OK)
    term = makeTaggedRef(tPtr);
  
  *vPtr = term;
} // GenCVariable::bind


void GenCVariable::propagate(TaggedRef var, TaggedRef term){
  am.checkSuspensionList(var, term);
}


void GenCVariable::propagate(TaggedRef var, TaggedRef *tPtr){
  am.checkSuspensionList(var, makeTaggedRef(tPtr));
}


SuspList* GenCVariable::propagate(TaggedRef var, SuspList* &sl,
					 TaggedRef term)
{
  sl = am.checkSuspensionList(tagged2SuspVar(var), var, sl, term);
}

Bool GenCVariable::unify(TaggedRef* tptr1, TaggedRef term1,
				TypeOfTerm ttag1,
				TaggedRef* tptr2, TaggedRef term2,
				TypeOfTerm ttag2) {
  switch (type){
  case FDVariable:
    return ((GenFDVariable*)this)->unifyFD(tptr1, term1, ttag1,
					   tptr2, term2, ttag2);
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }
    return NO;
} // GenCVariable::unify


size_t GenCVariable::getSize(void){
  switch (type){
  case FDVariable:
    return ((GenFDVariable*)this)->getSize();
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    break;
  }
  return 0;
} // GenCVariable::getSize



ProgramCounter GenCVariable::index(ProgramCounter elseLabel, 
					  IHashTable* table)
{
  switch (type){
  case FDVariable:
    return ((GenFDVariable*)this)->index(elseLabel, table);
  default:
    error("Unexpected type generic variable at %s:%d.",
	  __FILE__, __LINE__);
    return NOCODE;
  }  
} // GenCVariable::index


#ifdef OUTLINE
#define inline
#include "genvar.icc"
#undef inline
#endif

