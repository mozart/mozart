/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez <aarbelaez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *			Andres Felipe Barco <anfelbar@univalle.edu.co>
 *			Victor Rivera Zuniga <varivera@javerianacali.edu.co>
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
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

#ifndef __GEOZ_INT_PROP_BUILTINS_CC__
#define __GEOZ_INT_PROP_BUILTINS_CC__

#include "IntVarMacros.hh"

using namespace Gecode;
using namespace Gecode::Int;

OZ_BI_define(int_count,5,0)
{
  DeclareGSpace(sp);
  IntVarArgs arreglo = getIntVarArgs(OZ_in(0));
  IntView x1,x3;
  IntRelType rl = getIntRelType(OZ_in(2));
  IntConLevel cl = getIntConLevel(OZ_in(4));
  
  if(OZ_isGeIntVar(OZ_deref(OZ_in(1))))
    x1 = intOrIntView(OZ_in(1));
  else if (OZ_isInt(OZ_in(1))) {
    OZ_declareInt(1,valTmp);
    IntVar *tmp = new IntVar(sp,valTmp,valTmp);
    IntView view(*tmp);
    delete tmp;
    x1 = view;
  }
  else
    return OZ_typeError(1,"The second argument must be either Int or GeIntVar");
  
  if(OZ_isGeIntVar(OZ_deref(OZ_in(3))))
    x3 = intOrIntView(OZ_in(3));
  else if (OZ_isInt(OZ_in(3))) {
    OZ_declareInt(3,valTmp);
    IntVar *tmp = new IntVar(sp,valTmp,valTmp);
    IntView view(*tmp);
    delete tmp;
    x3 = view;
  }
  else
    return OZ_typeError(3,"The threeth argument must be either Int or GeIntVar");
  try {
    count(sp,arreglo,x1,rl,x3,cl);
  }
  catch(Exception e) {
   RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);

} OZ_BI_end

OZ_BI_define(int_sortedness,3,0)
{
  DeclareGSpace(sp);
  IntVarArgs a1 = getIntVarArgs(OZ_in(0));
  IntVarArgs a2 = getIntVarArgs(OZ_in(1));
  OZ_declareInt(2,conLevel);

  try{

    sorted(sp,a1,a2,(IntConLevel)conLevel);

  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end

OZ_BI_define(int_assign,2,0)
{
  DeclareGSpace(sp);
  IntVarArgs a = getIntVarArgs(OZ_in(0));
  DeclareIntAssignType(1,at);
  
  try{
    assign(sp,a,at);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end

OZ_BI_define(int_reified,3,0)
{
  DeclareGSpace(sp);
  IntSet dom = getIntSet(OZ_in(0));
  IntView v0 = intOrIntView(OZ_in(1));
  BoolVar *b0 = boolOrBoolVar(OZ_in(2));
  
  try{
    Gecode::dom(sp,v0,dom,*b0);
    delete b0;
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end

OZ_BI_define(int_ext,2,0) 
{
  DeclareGSpace(sp);
  IntVarArgs ar1 = getIntVarArgs(OZ_in(0));
  /*OZ_Term _t = OZ_in(1);
  OZ_Term inputl = OZ_arityList(_t);
  OZ_Term inputs = OZ_subtree(_t, OZ_head(inputl));
  int istate     = OZ_intToC(inputs);
  OZ_Term tl    = OZ_subtree(_t, OZ_head(OZ_tail(inputl)));
  Gecode::DFA::Transition trans[OZ_length(tl)];
  int sizel = OZ_length(tl);
  for(int i=0; OZ_isCons(tl); tl=OZ_tail(tl)) {
  //for(int i=0; i < sizel; tl=OZ_tail(tl)) {
    TransitionS(trans[i++], OZ_head(tl));
  }
  OZ_Term fstates = OZ_subtree(_t, OZ_head(OZ_tail(OZ_tail(inputl))));
  fstates = oz_deref(fstates);
  int fl[OZ_length(fstates)];
  sizel = OZ_length(fstates);
  for(int i=0; OZ_isCons(fstates); fstates=OZ_tail(fstates)) {
    fl[i++] = OZ_intToC(OZ_head(fstates));
  }
  Gecode::DFA dfa(istate, trans, fl);
  */
  DeclareDFA(1,dfa);
  try {
    extensional(sp, ar1, dfa);
  }
  catch(Exception e) {
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(sp);
} OZ_BI_end


#ifndef MODULES_LINK_STATIC
#include "../modGFDP-if.cc"
#endif

#endif
