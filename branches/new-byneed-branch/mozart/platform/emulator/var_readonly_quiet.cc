/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    derived from var_future.cc by Raphael Collet <raph@info.ucl.ac.be>
 *    and Alfred Spiessens <fsp@info.ucl.ac.be>
 * 
 *  Copyright:
 *    Denys Duchier (1998)
 *    Michael Mehl (1998)
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_readonly_quiet.hh"
#endif

#include "var_readonly_quiet.hh"
#include "dpInterface.hh"
#include "builtins.hh"
#include "unify.hh"
#include "thr_int.hh"
#include "value.hh"
#include "atoms.hh"

// this built-in allows us to create a read-only variable
OZ_BI_define(BInewReadOnly,0,1)
{
  Board *bb = oz_currentBoard();
  TaggedRef r = oz_newReadOnly(bb);
  OZ_RETURN(r);
} OZ_BI_end

// this builtin is used by the definition of '!!' only
/*
OZ_BI_define(BIbindReadOnly,2,0)
{
  OZ_Term var = OZ_in(0);
  DEREF(var,varPtr);
  // OZ_Term val = OZ_in(1);
  oz_declareNonvarIN(1, val);

  oz_bindReadOnly(varPtr,val);
  return PROCEED;
} OZ_BI_end
*/

OZ_Return QuietReadOnly::bind(TaggedRef *vPtr, TaggedRef t)
{
  if (oz_isLocalVar(this)) {
    return am.addSuspendVarListInline(vPtr);
  } else {
    oz_bindVar(this,vPtr, t);
    return PROCEED;
  }
}

OZ_Return QuietReadOnly::forceBind(TaggedRef *vPtr, TaggedRef t)
{
  if (*vPtr != oz_deref(t))
    oz_bindVar(this,vPtr,t);
  return PROCEED;
}

OZ_Return QuietReadOnly::unify(TaggedRef *vPtr, TaggedRef *tPtr)
{
  return bind(vPtr,makeTaggedRef(tPtr));
}

// this method forces the read-only to become needed; it kicks the
// suspension list and turns the variable into a ReadOnly.
OZ_Return QuietReadOnly::becomeNeeded()
{
  // step 1: release the current suspension list
  oz_checkSuspensionList(this, pc_all);
  // step 2: mutate into a SimpleVar
  setType(OZ_VAR_READONLY);
  // disposeS();
  return PROCEED;
}

// use this method for adding demanding suspensions only!
OZ_Return QuietReadOnly::addSusp(TaggedRef *tPtr, Suspendable * susp) {
  // release the current suspension list and mutate into a ReadOnly
  becomeNeeded();
  // add susp into the ReadOnly's suspension list
  addSuspSVar(susp);
  return SUSPEND;
}

void QuietReadOnly::printStream(ostream &out,int depth)
{
  out << "<quiet readonly>";
}

// this builtin/propagator is only internally available
/*
OZ_BI_define(BIvarToReadOnly,2,0)
{
  oz_declareDerefIN(0,v);
  if (oz_isVarOrRef(v)) {
    if (oz_isReadOnly(v)) {
      if (((ReadOnly*)tagged2Var(v))->isFailed()) {
	v = makeTaggedRef(vPtr);
	goto bind_fut;
      }
      else {
	((ReadOnly*)tagged2Var(v))->addSuspSVar(oz_currentThread());
	return SUSPEND;
      }
    }
    else {
      oz_suspendOnPtr(vPtr);
    }
  }
 bind_fut:
  oz_declareDerefIN(1,f);
  oz_bindReadOnly(fPtr,v);
  return PROCEED;
} OZ_BI_end
*/
