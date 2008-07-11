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
#include "var_of.hh"
#include "thr_int.hh"


OZ_BI_define(BIaddr,1,1)
{
  oz_declareIN(0,val);

  OZ_RETURN_INT(ToInt32(tagged2Addr(val)));
} OZ_BI_end


OZ_BI_define(BIchunkWidth, 1,1)
{
  OZ_Term ch =  OZ_in(0);

  DEREF(ch, chPtr);

  Assert(!oz_isRef(ch));
  if (oz_isVarOrRef(ch)) {
    oz_suspendOn(makeTaggedRef(chPtr));
  }
  if (oz_isChunk(ch)) {
    int w;
    switch (tagged2Const(ch)->getType()) {
    case Co_Class: 
      if (!tagged2OzClass(ch)->isComplete())   // call distribution layer
	return (*distClassGet)(tagged2OzClass(ch));
      w = tagged2OzClass(ch)->getWidth();
      break;
    case Co_Object:
      if (! tagged2Object(ch)->getClass()->isComplete())   // distribution
	return (*distClassGet)(tagged2Object(ch)->getClass());
      w = tagged2Object(ch)->getWidth();
      break;
    case Co_Chunk: 
      w = tagged2SChunk(ch)->getWidth();
      break;
    default:
      w = 0;
      break;
    }
    OZ_RETURN(makeTaggedSmallInt(w));
  }    
  oz_typeError(0,"Chunk");
} OZ_BI_end


OZ_BI_define(BIisRecordVarB,1,1)
{
  TaggedRef t = OZ_in(0); 
  DEREF(t, tPtr);
  Assert(!oz_isRef(t));
  if (oz_isLTupleOrRef(t) || oz_isLiteral(t) || oz_isSRecord(t) ||
      isGenOFSVar(t)) {
    OZ_RETURN(oz_true());
  } else {
    OZ_RETURN(oz_false());
  }
} OZ_BI_end


OZ_BI_define(_getsBoundDummy, 1,0)
{
  return oz_unify(OZ_in(0),oz_true());
} OZ_BI_end

OZ_Term BI_GetsBoundDummy;

void browser_init(void) {
  BI_GetsBoundDummy = 
    makeTaggedConst(new Builtin("Browser", "getsBound (dummy)",1,0, 
				_getsBoundDummy, OK));
}


OZ_BI_define(BIgetsBoundB, 2, 0)
{
  oz_declareDerefIN(0,v);
  
  Assert(!oz_isRef(v));
  if (oz_isVarOrRef(v)){
    RefsArray * args = RefsArray::allocate(1, NO);
    args->setArg(0,OZ_in(1));

    Thread *thr =
      (Thread *) OZ_makeSuspendedThread(BI_GetsBoundDummy, args->getArgsRef(),1);
    OZ_Return ret = oz_var_addQuietSusp(vPtr, thr);
    if (ret == PROCEED) oz_wakeupThread(thr);
    if (ret != SUSPEND) return ret;
  }

  return PROCEED;		// no result yet;
} OZ_BI_end


OZ_BI_define(BIchunkArityBrowser,1,1)
{
  OZ_Term ch =  OZ_in(0);

  DEREF(ch, chPtr);

  Assert(!oz_isRef(ch));
  if (oz_isVarOrRef(ch)) {
    oz_suspendOn(makeTaggedRef(chPtr));
  }

  if (oz_isChunk(ch)) {
    TaggedRef as;
    switch (tagged2Const(ch)->getType()) {
    case Co_Class : 
      if (!tagged2OzClass(ch)->isComplete())   // call distribution layer
	return (*distClassGet)(tagged2OzClass(ch));
      as = tagged2OzClass(ch)->getArityList();
      break;
    case Co_Object: 
      if (! tagged2Object(ch)->getClass()->isComplete())   // distribution
	return (*distClassGet)(tagged2Object(ch)->getClass());
      as = tagged2Object(ch)->getArityList();
      break;
    case Co_Chunk : 
      as = tagged2SChunk(ch)->getArityList();
      break;
    default:
      as = oz_nil();
      break;
    }
    OZ_RETURN(as);
  }

  oz_typeError(0,"Chunk");
} OZ_BI_end


OZ_BI_define(BIgetTermSize,3,1) {
  oz_declareIN(0,t);
  oz_declareIntIN(1,depth);
  oz_declareIntIN(2,width);
  OZ_RETURN_INT(OZ_termGetSize(t, depth, width));
} OZ_BI_end


OZ_BI_define(BIvarSpace,1,1)
{
  oz_declareIN(0,v);
  void *addr;

  DEREF(v,vPtr);

  Assert(!oz_isRef(v));
  if (oz_isVar(v)) {
    OzVariable *ov = tagged2Var(v);
    addr = ov->getBoardInternal();
  } else {
    addr = 0;
  }

  OZ_RETURN_INT(ToInt32(addr));
} OZ_BI_end


OZ_BI_define(BIprocLoc,1,3)
{
  oz_declareIN(0,v);
  DEREF(v,vPtr);

  if (oz_isAbstraction(v)) {
    Abstraction *a = tagged2Abstraction(v);
    PrTabEntry *p = a->getPred();
    if (p) {
      OZ_out(0) = p->getFile();
      OZ_out(1) = OZ_int(p->getLine());
      OZ_out(2) = OZ_int(p->getColumn());
    } else {
      OZ_out(0) = AtomProcedure;
      OZ_out(1) = AtomDash;
      OZ_out(2) = AtomDash;
    }
    return (PROCEED);
  } else if (oz_isBuiltin(v)) {
    OZ_out(0) = AtomBBuiltin;
    OZ_out(1) = AtomDash;
    OZ_out(2) = AtomDash;
    return (PROCEED);
  } else {
    return oz_raise(E_ERROR,E_SYSTEM,"BIprocLoc: no procedure",1,v);
  }
} OZ_BI_end


OZ_BI_define(BIshortName,1,1)
{
  OZ_declareAtom(0,longN);
#ifdef WINDOWS
  char shortN[2048];
  int n = GetShortPathName(longN, shortN, sizeof(shortN));
  if (n == 0 || n >= sizeof(shortN))
    return OZ_FAILED;
  else
    OZ_RETURN_ATOM(shortN);
#else
  OZ_RETURN_ATOM(longN);
#endif
} OZ_BI_end

/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modBrowser-if.cc"

#endif

