/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Contributors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *
 *  Copyright:
 *    Michael Mehl, 1997
 *    Kostja Popow, 1997
 *    Ralf Scheidhauer, 1997
 *    Christian Schulte, 1997
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

#include "base.hh"
#include "builtins.hh"
#include "var_base.hh"
#include "space.hh"
#include "thr_int.hh"


OZ_BI_define(BIaddr,1,1)
{
  oz_declareIN(0,val);

  DEREF(val,valPtr,valTag);

  OZ_RETURN_INT((isVariableTag(valTag) && valPtr) ?
                ToInt32(valPtr) :
                ToInt32(tagValueOf2(valTag,val)));
} OZ_BI_end


OZ_BI_define(BIchunkWidth, 1,1)
{
  OZ_Term ch =  OZ_in(0);

  DEREF(ch, chPtr, chTag);

  switch(chTag) {
  case UVAR:
    // FUT
  case CVAR:
    oz_suspendOn(makeTaggedRef(chPtr));

  case OZCONST:
    if (!oz_isChunk(ch)) oz_typeError(0,"Chunk");
    //
    switch (tagged2Const(ch)->getType()) {
    case Co_Class: OZ_RETURN(makeTaggedSmallInt(tagged2ObjectClass(ch)->getWidth()));
    case Co_Object:OZ_RETURN(makeTaggedSmallInt(tagged2Object(ch)->getWidth()));
    case Co_Chunk: OZ_RETURN(makeTaggedSmallInt(tagged2SChunk(ch)->getWidth()));
    default:
      // no features
      OZ_RETURN(makeTaggedSmallInt (0));
    }

  default:
    oz_typeError(0,"Chunk");
  }
} OZ_BI_end


OZ_BI_define(BIisRecordCVarB,1,1)
{
  TaggedRef t = OZ_in(0);
  DEREF(t, tPtr, tag);
  switch (tag) {
  case LTUPLE:
  case LITERAL:
  case SRECORD:
    break;
  case CVAR:
    if (tagged2CVar(t)->getType()!=OZ_VAR_OF)
      OZ_RETURN(oz_false());
    break;
  case UVAR:
    // SVAR
    OZ_RETURN(oz_false());
  default:
    OZ_RETURN(oz_false());
  }
  OZ_RETURN(oz_true());
} OZ_BI_end


OZ_BI_define(_getsBound_dummyB, 2,0)
{
  return oz_unify(OZ_in(1),oz_true());
} OZ_BI_end


OZ_C_proc_begin(BIgetsBoundB, 2)
{
  oz_declareDerefArg(0,v);

  if (isVariableTag(vTag)){
    Thread *thr =
      (Thread *) OZ_makeSuspendedThread (_getsBound_dummyB, OZ_args, OZ_arity);
    OZ_Return ret = oz_var_addSusp(vPtr, thr);
    if (ret == PROCEED) oz_wakeupThread(thr);
    if (ret != SUSPEND) return ret;
  }

  return PROCEED;               // no result yet;
}
OZ_C_proc_end


OZ_BI_define(BIdeepFeed,2,0)
{
  oz_declareNonvarIN(0,c);
  oz_declareIN(1,val);

  if (!oz_isCell(c)) {
    oz_typeError(0,"Cell");
  }

  CellLocal *cell = (CellLocal*)tagged2Tert(c);

  Board *savedNode = am.currentBoard();
  Board *home1 = GETBOARD(cell);

  switch (oz_installPath(home1)) {
  case INST_FAILED:
  case INST_REJECTED:
    OZ_error("deep: install");
  case INST_OK:
    break;
  }

  TaggedRef newVar = oz_newVariable();
  TaggedRef old = cell->exchangeValue(newVar);
  OZ_Return ret = oz_unify(old,oz_cons(val,newVar));

  switch (oz_installPath(savedNode)) {
  case INST_FAILED:
  case INST_REJECTED:
    OZ_error("deep: install back");
  case INST_OK:
    break;
  }

  return ret;
} OZ_BI_end

OZ_BI_define(BIchunkArityBrowser,1,1)
{
  OZ_Term ch =  OZ_in(0);

  DEREF(ch, chPtr, chTag);

  switch(chTag) {
  case UVAR:
    // FUT
  case CVAR:
    oz_suspendOn(makeTaggedRef(chPtr));

  case OZCONST:
    if (!oz_isChunk(ch)) oz_typeError(0,"Chunk");
    //
    switch (tagged2Const(ch)->getType()) {
    case Co_Class : OZ_RETURN(tagged2ObjectClass(ch)->getArityList());
    case Co_Object: OZ_RETURN(tagged2Object(ch)->getArityList());
    case Co_Chunk : OZ_RETURN(tagged2SChunk(ch)->getArityList());
    default:
      // no features
      OZ_RETURN(oz_nil());
    }

  default:
    oz_typeError(0,"Chunk");
  }
} OZ_BI_end



OZ_BI_define(BIgetTermSize,3,1) {
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  oz_declareIntIN(2,width);
  OZ_RETURN_INT(OZ_termGetSize(t, depth, width));
} OZ_BI_end

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modBrowser-if.cc"

#endif
