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


#include "genvar.hh"
#include "am.hh"

GenCVariable::GenCVariable(TypeOfGenCVariable t, TaggedRef pn, Board * n) :
  SVariable(n == NULL ? am.currentBoard : n, pn)
{
  setType(t);
}


Bool GenCVariable::isLocalVariable(void)
{
  Board * home_board = getHome1();

  return (home_board == am.currentBoard ||
          home_board->getBoardDeref() == am.currentBoard)
    ? OK : NO;
}

void GenCVariable::propagate(TaggedRef var, SuspList * &sl, TaggedRef term,
                             Bool unifyVars)
{
  sl = am.checkSuspensionList(tagged2SuspVar(var), var, sl, term, NULL, unifyVars);
}


#ifdef OUTLINE
#define inline
#include "genvar.icc"
#undef inline
#endif
