/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
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

#include "value.hh"
#include "board.hh"
#include "var_base.hh"
#include "builtins.hh"

#ifdef OUTLINE
#define inline
#endif


/*
 * Trail
 *
 */

class ScTrail: public FastStack {
public:
  ScTrail() : FastStack() {}
  ~ScTrail() {}
  
  void save(int * p) {
    // Save content and address
    push2((StackEntry) *p, (StackEntry) p);
  }

  void unwind(void) {
    while (!isEmpty()) {
      StackEntry e1, e2;
      pop2(e1,e2);
      int * p = (int *) e2;
      int   v = (int)   e1;
      *p = v;
    } 
  }
};

static ScTrail scTrail;



/*
 * Stack
 *
 */

class ScStack: public FastStack {
public:
  ScStack() : FastStack() {}
  ~ScStack() {}
  
  void push(TaggedRef x) {
    FastStack::push1((StackEntry) x);
  }

  void check(void);

};

static ScStack scStack;


/*
 * Marking of visited data structures
 *
 */
#define MARKVAR(u) \
  scTrail.save((int *) u); *u=makeTaggedGcMark(NULL);

#define MARKFIELD(d) \
  scTrail.save((int *) d->cacGetMarkField()); d->cacMark(NULL);

/*
 * Test whether a space is good
 *
 */
#define ISGOOD(space) ((space)->hasMarkOne())

/*
 * Main routine
 *
 */

TaggedRef futs;
TaggedRef bads;

/*
 * Forward decl
 */

void checkSituatedBlock(OZ_Term *, int);


inline
void Board::checkSituatedness(TaggedRef * x, TaggedRef *f,TaggedRef *b) {

  futs = AtomNil;
  bads = AtomNil;

  scTrail.init();
  scStack.init();

  setGlobalMarks();
  setMarkOne();

  checkSituatedBlock(x,1);

  scStack.check();

  unsetGlobalMarks();
  unsetMarkOne();

  scTrail.unwind();

  scTrail.exit();
  scStack.exit();

  *f = futs;
  *b = bads;
}


OZ_Return OZ_checkSituatednessDynamic(Board * s,TaggedRef * x) {
  TaggedRef f,b;

  s->checkSituatedness(x,&f,&b);

  if (!oz_eq(b,AtomNil)) {
    // There is at least a bad guy!
    return oz_raise(E_ERROR,E_KERNEL,"spaceSituatedness",1,b);
  }
    
  if (!oz_eq(f,AtomNil)) {
    // There is at least a future, suspend!
    do {
      Assert(oz_isCons(f));
      TaggedRef h = oz_head(f);
      Assert(oz_isRef(h));
      TaggedRef * f_ptr = tagged2Ref(h);
      Assert(oz_isVar(*f_ptr));
      Assert(tagged2Var(*f_ptr)->getType() == OZ_VAR_FUTURE);
      am.addSuspendVarList(f_ptr);
      f = oz_tail(f);
    } while (!oz_eq(f,AtomNil));
    return SUSPEND;
  }
  return PROCEED;
}
  

/*
 * Recursion
 *
 */

void ScStack::check(void) {

  while (!isEmpty()) {
    StackEntry tp;
    pop1(tp);  
    TaggedRef x = (TaggedRef) tp;

    if (oz_isLTuple(x)) {
      LTuple * lt = tagged2LTuple(x);
      checkSituatedBlock(lt->getRef(), 2);      
      MARKFIELD(lt);
    } else {
      Assert(oz_isSRecord(x));
      SRecord * sr = tagged2SRecord(x);
      TaggedRef * r = sr->getRef();
      int n = sr->getWidth();
      MARKFIELD(sr);
      checkSituatedBlock(r, n);      
    }
    
  }
  
}

/*
 * Here comes the stuff
 *
 */


inline
int checkSituatednessExtension(TaggedRef term) {
  OZ_Extension *ex = tagged2Extension(term);

  Assert(ex);

  // hack alert: write forward into vtable!
  if ((*(int32*)ex)&1)
    return OK;

  Board *bb=(Board*)(ex->__getSpaceInternal());

  if (!bb || ISGOOD(bb))
    return OK;


  int32 *fromPtr = (int32*)ex;

  scTrail.save(fromPtr);
  
  *fromPtr |= 1;

  return NO;
}


inline 
int Name::checkSituatedness(void) {
  if (cacIsMarked())
    return OK;
  
  if (!ISGOOD(getBoardInternal())) {
    MARKFIELD(this);
    return NO;
  }
  return OK;
}

inline 
int Literal::checkSituatedness(void) {
  if (isAtom())
    return OK; 
  else
    return ((Name *) this)->checkSituatedness();
}

inline
int ConstTerm::checkSituatedness(void) {
  Assert(this);
  
  if (cacIsMarked())
    return OK;

  switch (getType()) {
    /*
     * ConstTermWithHome
     *
     */
  case Co_Abstraction: 
  case Co_Chunk:
  case Co_Array:
  case Co_Dictionary:
  case Co_Class: 
    { 
      ConstTermWithHome * ctwh = (ConstTermWithHome *) this;
      if (!ctwh->hasGName() && !ISGOOD(ctwh->getSubBoardInternal())) {
	MARKFIELD(this);
	return NO;
      }
    }
    break;

    /*
     * Tertiary
     *
     */

  case Co_Object: 
  case Co_Cell:
  case Co_Port:  
  case Co_Space:
  case Co_Lock:
    {
      Tertiary * t = (Tertiary *) this;
      if (t->isLocal() && !ISGOOD(t->getBoardLocal())) {
	MARKFIELD(this);
	return NO;
      }
    }
    

  default:
    break;
  }

  return OK;

}


void checkSituatedBlock(OZ_Term * tb, int sz) {
  
  while (sz--) {
    TaggedRef * x_ptr = tb++;
    TaggedRef   x     = *x_ptr;
    
  again:
    switch (tagTypeOf(x)) {
    case TAG_REF:
      if (!x) continue;
    case TAG_REF2:
    case TAG_REF3:
    case TAG_REF4:
      x_ptr = tagged2Ref(x);
      x     = *x_ptr;
      goto again;
      
    case TAG_UNUSED_UVAR:
    case TAG_UNUSED_SVAR:
    case TAG_UNUSED_FSETVALUE:
    case TAG_UNUSED_FLOAT:
    case TAG_GCMARK:
    case TAG_SMALLINT:
      continue;

    case TAG_LITERAL:
      if (!tagged2Literal(x)->checkSituatedness())
	bads = oz_cons(x,bads);
      continue;
      
    case TAG_EXT:
      if (!checkSituatednessExtension(x))
	bads = oz_cons(x,bads);
      continue;
      
    case TAG_LTUPLE:
      if (!tagged2LTuple(x)->cacIsMarked())
	scStack.push(x);
      continue;

    case TAG_SRECORD:
      if (!tagged2SRecord(x)->cacIsMarked())
	scStack.push(x);
      continue;
     
    case TAG_CONST:
      if (!tagged2Const(x)->checkSituatedness())
	bads = oz_cons(x,bads);
      continue;

    case TAG_VAR: 
      {
	OzVariable * cv = tagged2Var(x);
      
	if (!ISGOOD(cv->getBoardInternal())) {
	  if (cv->getType() == OZ_VAR_FUTURE)
	    futs = oz_cons(makeTaggedRef(x_ptr),futs);
	  else
	    bads = oz_cons(makeTaggedRef(x_ptr),bads);
	  MARKVAR(x_ptr);
	}
      }
      continue;
    }
  }
}
