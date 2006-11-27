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
#include "var_ext.hh"
#include "thr_int.hh"
#include "GeVar.hh"

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

void Trail::pushGeVariable(TaggedRef * varPtr, TaggedRef varLocal) {
  //La variable debe ser global
  OzVariable *v = extVar2Var(oz_getExtVar(*varPtr));

  Assert(v);
  Assert(!v->isTrailed());

  ensureFree(3);
  
  Assert(oz_isVar(*varPtr));
  //  Assert(oz_isGeVar(*varPtr));
  Stack::push((StackEntry) varPtr,      NO);
  // inserta la variable global para seguir trabajando con la local.
  Stack::push((StackEntry) v,           NO);
  Stack::push((StackEntry) Te_GeVariable, NO);
  

  v->setTrailed();

  *varPtr = varLocal;

  //Igual que en board.cc:554
  //  Assert(oz_isVar(*varPtr));
  /*  if(oz_isVar(*varPtr)) {
    GeVar *vl = static_cast<GeVar*>(oz_getExtVar(varLocal));
    GenericSpace *gs = oz_currentBoard()->getGenericSpace();    
    gs->setVarRef(vl->getIndex(),makeTaggedRef(varPtr));
    }*/


}

void Trail::pushMark(void) {
  // All variables marked as trailed must be unmarked!

  StackEntry * top = tos-1;

  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      goto exit;
    case Te_GeVariable: {
      //      cout<<"pushMark Te_GeVariable"<<endl; fflush(stdout);
      OzVariable *v = (OzVariable *) * (top-1);
      Assert(v->isTrailed());
      v->unsetTrailed();
      break;
    }
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
    case Te_GeVariable:
      cout<<"test Te_GeVariable"<<endl; fflush(stdout);
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

inline
void Trail::popGeVariable(TaggedRef *&varPtr, OzVariable *&orig) {
  Assert(getTeType() == Te_GeVariable);
  (void) Stack::pop();
  orig   = (OzVariable *) Stack::pop();
  varPtr = (TaggedRef *)  Stack::pop();

  //  Assert(oz_isVar(*varPtr));

  //if current space is failed isn't necessary to change the OZ_Term vector in GenericSpace
  //unwindFailed
  if(oz_currentBoard()->isFailed() == BoTag_Failed) return;
  /*
  if(oz_isVar(*varPtr)) {
    //cout<<"Restoring the local reference in OZ_Term vector ..."<<endl; fflush(stdout);
    OzVariable *lvar = tagged2Var(*varPtr);
    GeVar *v1 = static_cast<GeVar*>(var2ExtVar(lvar));
    //    cout<<"oz_currentBoard(): "<<(oz_currentBoard())->isFailed()<<endl; fflush(stdout);
    GenericSpace *gs = oz_currentBoard()->getGenericSpace();
    gs->setVarRef(v1->getIndex(),makeTaggedRef(newTaggedVar(lvar)));
    //cout<<"end Restoring the local reference in OZ_Term vector ..."<<endl; fflush(stdout);
  }
  */
}

void Trail::popMark(void) {
  Assert(isEmptyChunk());
  (void) Stack::pop();

  StackEntry * top = tos-1;

  do {
    switch ((TeType) (int) *top) {
    case Te_Mark:
      return;
    case Te_GeVariable: {
      //      cout<<"popMark Te_GeVariable"<<endl; fflush(stdout);
      OzVariable *v = (OzVariable *) * (top-1);
      Assert(!v->isTrailed());
      v->setTrailed();
      break;
    }
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
  //printf("unwind\n");fflush(stdout);
  TaggedRef s = AtomNil;

  if (!isEmptyChunk()) {
    //printf("isEmptyChunk()\n");fflush(stdout);
    Thread * t = (Thread *) NULL;

    int hasNoRunnable = !b->hasRunnableThreads();

    do {

      switch (getTeType()) {

      case Te_Bind: {
	//printf("Te_Bind\n");fflush(stdout);
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
      case Te_GeVariable: {
	//printf("Te_GeVariable\n");fflush(stdout);
	TaggedRef *varPtr;
	OzVariable *glb,*lvar;
	popGeVariable(varPtr,glb);
	cout<<"unwind"<<endl; fflush(stdout);
	if(oz_isInt(*varPtr)) {
	  s = oz_cons(oz_cons(makeTaggedRef(varPtr), *varPtr), s);
	//printf("trail.cc Te_GeVariable else\n");fflush(stdout);
	}
	else {
	  if (!oz_isVar(*varPtr))
	    *varPtr=oz_deref(*varPtr);
	  Assert(oz_isVar(*varPtr));
	  OzVariable *lv = tagged2Var(*varPtr);
	  TaggedRef tmplv = *newTaggedVar(lv);	
	  s = oz_cons(oz_cons(makeTaggedRef(varPtr), tmplv),s);
	}
	TaggedRef varTmp = makeTaggedRef(newTaggedVar(glb));
	//	unBind(varPtr,oz_deref(varTmp));
	*varPtr = oz_deref(varTmp);
	AssureThread;
	oz_var_addSusp(varPtr, t);
	Assert(tagged2Var(*varPtr)->isTrailed());
	tagged2Var(*varPtr)->unsetTrailed();	 

	//cout<<"termino unwind: "<<endl; fflush(stdout);

	// Ojo -> pensar!!
	/*if (hasNoRunnable && !oz_var_hasSuspAt(*varPtr,b)) {
	  AssureThread;
	  oz_var_addSusp(varPtr, t);
	  }*/

	break;  
      }
      case Te_Variable: {
	//printf("Te_Variable\n");fflush(stdout);
	TaggedRef * varPtr;
	OzVariable * copy;
	popVariable(varPtr, copy);
	
	Assert(oz_isVar(*varPtr));
	
	oz_var_restoreFromCopy(tagged2Var(*varPtr), copy);
	
	Assert(tagged2Var(*varPtr)->isTrailed());
	
	tagged2Var(*varPtr)->unsetTrailed();
	
	if (hasNoRunnable && !oz_var_hasSuspAt(*varPtr,b)) {
	  //cout<<"AssureThread Te_Variable"<<endl; fflush(stdout);
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

    case Te_GeVariable: {
      //cout<<"unwindFailed Te_GeVariable"<<endl; fflush(stdout);
      TaggedRef *varPtr;
      OzVariable *glb;
      popGeVariable(varPtr,glb);

      if (!oz_isVar(*varPtr))
	*varPtr=oz_deref(*varPtr);      
      if(!oz_isInt(*varPtr)) {
	Assert(oz_isVar(*varPtr));
      
	OzVariable *lv = tagged2Var(*varPtr);
	TaggedRef varTmp = makeTaggedRef(newTaggedVar(glb));
	*varPtr = oz_deref(varTmp);

	//	*varPtr = makeTaggedRef(newTaggedVar(glb));
	
	Assert(tagged2Var(*varPtr)->isTrailed());
	tagged2Var(*varPtr)->unsetTrailed();	 
      }
      else { cout<<"Es entero"<<endl; fflush(stdout); }

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

    case Te_GeVariable: {
      //cout<<"unwindEqEq Te_GeVariable"<<endl; fflush(stdout);
      TaggedRef *varPtr;
      OzVariable *glb;
      popGeVariable(varPtr,glb);
      Assert(oz_isVar(*varPtr));
      
      OzVariable *lv = tagged2Var(*varPtr);
      
      *varPtr = makeTaggedRef(newTaggedVar(glb));

      Assert(tagged2Var(*varPtr)->isTrailed());
      tagged2Var(*varPtr)->unsetTrailed();	 

      //      (void) oz_addSuspendVarList(varPtr);
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

