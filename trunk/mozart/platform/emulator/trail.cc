/*
 *  Authors:
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Kostja Popov, 1999
 *    Christian Schulte, 1999
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

#if defined(INTERFACE)
#pragma implementation "trail.hh"
#endif

#include "trail.hh"
#include "var_base.hh"
#include "thr_int.hh"

Trail trail;

/*
 * Pushing
 *
 */

void Trail::pushBind(TaggedRef *varPtr) {
  ensureFree(3);
  Stack::push((StackEntry) varPtr,             NO);
  Stack::push((StackEntry) ToPointer(*varPtr), NO);
  Stack::push((StackEntry) Te_Bind,            NO);
}

void Trail::pushVariable(TaggedRef * varPtr) {
  OzVariable * v = tagged2Var(*varPtr);

  if (v->isTrailed())
    return;

  OzVariable * c = oz_var_copyForTrail(v);

  Assert(c);

  ensureFree(3);
  Stack::push((StackEntry) varPtr,      NO);
  Stack::push((StackEntry) c,           NO);
  Stack::push((StackEntry) Te_Variable, NO);

  v->setTrailed();

}

void Trail::pushMark(void) {
  // All variables marked as trailed must be unmarked!

  StackEntry * top = tos-1;

  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      goto exit;
    case Te_Variable: {
      TaggedRef * varPtr = (TaggedRef *) *(top-2);
      Assert(oz_isVar(*varPtr));
      OzVariable * v = tagged2Var(*varPtr);
      Assert(v->isTrailed());
      v->unsetTrailed();
      break;
    }
    default:
      break;
    }
    top -= 3;
  } while (OK);

 exit:
  Stack::push((StackEntry) Te_Mark);

}

void Trail::test(void) {
  // All variables marked as trailed must be unmarked!

  StackEntry * top = tos-1;

  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      goto exit;
    case Te_Variable: {
      TaggedRef * varPtr = (TaggedRef *) *(top-2);
      Assert(oz_isVar(*varPtr));
      break;
    }
    default:
      break;
    }
    top -= 3;
  } while (OK);

 exit:
  return;
}



/*
 * Popping
 *
 */

inline
void Trail::popBind(TaggedRef *&val, TaggedRef &old) {
  Assert(getTeType() == Te_Bind);
  (void) Stack::pop();
  old = (TaggedRef)  ToInt32(Stack::pop());
  val = (TaggedRef*) Stack::pop();
}

inline
void Trail::popVariable(TaggedRef *&varPtr, OzVariable *&orig) {
  Assert(getTeType() == Te_Variable);
  (void) Stack::pop();
  orig   = (OzVariable *) Stack::pop();
  varPtr = (TaggedRef *)  Stack::pop();
}


void Trail::popMark(void) {
  Assert(isEmptyChunk());
  (void) Stack::pop();

  StackEntry * top = tos-1;

  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      return;
    case Te_Variable: {
      TaggedRef * varPtr = (TaggedRef *) *(top-2);
      Assert(oz_isVar(*varPtr));
      OzVariable * v = tagged2Var(*varPtr);
      Assert(!v->isTrailed());
      v->setTrailed();
      break;
    }
    default:
      break;
    }
    top -= 3;
  } while (OK);

}


/*
 * Deinstallation of trail
 *
 */

inline
void unBind(TaggedRef *p, TaggedRef t) {
  Assert(oz_isVar(t));
  *p = t;
}

#define AssureThread \
  if (!t) t=oz_newThreadPropagate(b);

TaggedRef Trail::unwind(Board * b) {

  TaggedRef s = AtomNil;

  if (!isEmptyChunk()) {

    Thread * t = (Thread *) NULL;

    int hasNoRunnable = !b->hasRunnableThreads();

    do {

      switch (getTeType()) {

      case Te_Bind: {

	TaggedRef * refPtr, value;
	
	popBind(refPtr, value);
	Assert(oz_isRef(*refPtr) || !oz_isVar(*refPtr));
	Assert(oz_isVar(value));
	
	s = oz_cons(oz_cons(makeTaggedRef(refPtr),*refPtr),s);
	
	TaggedRef vv= *refPtr;
	DEREF(vv,vvPtr);

	Assert(!oz_isRef(vv));
	if (hasNoRunnable && oz_isVarOrRef(vv) && !oz_var_hasSuspAt(vv,b)) {
	  AssureThread;
	  oz_var_addSusp(vvPtr,t);
	}

	unBind(refPtr, value);
	
	// value is always global variable, so add always a thread;
	if (hasNoRunnable && !oz_var_hasSuspAt(*refPtr,b)) {
	  AssureThread;
	  if (oz_var_addSusp(refPtr,t)!=SUSPEND) {
	    Assert(0);
	  }
	}
	
	break;
      }
      
      case Te_Variable: {
	TaggedRef * varPtr;
	OzVariable * copy;
	popVariable(varPtr, copy);
	
	Assert(oz_isVar(*varPtr));
	
	oz_var_restoreFromCopy(tagged2Var(*varPtr), copy);
	
	Assert(tagged2Var(*varPtr)->isTrailed());
	
	tagged2Var(*varPtr)->unsetTrailed();
	
	if (hasNoRunnable && !oz_var_hasSuspAt(*varPtr,b)) {
	  AssureThread;
	  oz_var_addSusp(varPtr, t);
	}
	
	s = oz_cons(oz_cons(makeTaggedRef(varPtr),
			    makeTaggedRef(newTaggedVar(copy))),
		    s);
	
	break;
      }

      default:
	break;
      }
    } while (!isEmptyChunk());

  }
  
  popMark();

  return s;
}


void Trail::unwindFailed(void) {

  do {

    switch (getTeType()) {

    case Te_Bind: {
      TaggedRef *refPtr;
      TaggedRef value;
      popBind(refPtr,value);
      unBind(refPtr,value);
      break;
    }

    case Te_Variable: {
      TaggedRef * varPtr;
      OzVariable * copy;
      popVariable(varPtr, copy);

      Assert(oz_isVar(*varPtr));

      oz_var_restoreFromCopy(tagged2Var(*varPtr), copy);

      Assert(tagged2Var(*varPtr)->isTrailed());

      tagged2Var(*varPtr)->unsetTrailed();

      break;
    }

    case Te_Mark:
      popMark();
      return;

    default:
      Assert(0);
      break;
    }

  } while (1);

}

void Trail::unwindEqEq(void) {

  am.emptySuspendVarList();

  do {

    switch (getTeType()) {

    case Te_Bind: {
      TaggedRef *refPtr;
      TaggedRef value;
      popBind(refPtr,value);

      Assert(oz_isVar(value));

      TaggedRef oldVal = makeTaggedRef(refPtr);
      DEREF(oldVal,ptrOldVal);

      unBind(refPtr,value);

      Assert(!oz_isRef(oldVal));
      if (oz_isVarOrRef(oldVal))
	(void) oz_addSuspendVarList(ptrOldVal);

      (void) oz_addSuspendVarList(refPtr);

      break;
    }

    case Te_Variable: {
      TaggedRef * varPtr;
      OzVariable * copy;
      popVariable(varPtr, copy);

      Assert(oz_isVar(*varPtr));

      oz_var_restoreFromCopy(tagged2Var(*varPtr), copy);

      Assert(tagged2Var(*varPtr)->isTrailed());

      tagged2Var(*varPtr)->unsetTrailed();

      (void) oz_addSuspendVarList(varPtr);

      break;
    }

    case Te_Mark:
      popMark();
      return;

    default:
      Assert(0);
      break;
    }

  } while (1);

}

