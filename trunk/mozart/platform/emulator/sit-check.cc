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
  scTrail.save((int *) u); *u=makeTaggedMarkInt(0);

#define MARKFIELD(d) \
  scTrail.save((int *) d->cacGetMarkField()); d->cacMark(NULL);

/*
 * Test whether a space is good
 *
 */
#define ISGOOD(space) ((space)->hasMark())

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
  setMark();

  checkSituatedBlock(x,1);

  scStack.check();

  unsetGlobalMarks();
  unsetMark();

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
      (void) am.addSuspendVarListInline(f_ptr);
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

    // due to sharing, it is possible for the same term to be pushed
    // twice onto the ScStack.  Therefore, whenever we pop an item
    // from the ScStack, we must check whether it is already marked.
    
    if (oz_isLTuple(x)) {
      LTuple * lt = tagged2LTuple(x);
      if (!lt->cacIsMarked()) {
	checkSituatedBlock(lt->getRef(), 2);      
	MARKFIELD(lt);
      }
    } else {
      Assert(oz_isSRecord(x));
      SRecord * sr = tagged2SRecord(x);
      if (!sr->cacIsMarked()) {
	TaggedRef * r = sr->getRef();
	int n = sr->getWidth();
	MARKFIELD(sr);
	checkSituatedBlock(r, n);
      }
    }
    
  }
  
}

/*
 * Here comes the stuff
 *
 */


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
  case Co_Extension: {
    OZ_Extension * ex = const2Extension(this);
    Assert(ex);
    Board * bb = (Board *) (ex->__getSpaceInternal());
    if (!bb || ISGOOD(bb))
      return OK;
    MARKFIELD(this);
    return NO;
  }
  break;

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
    switch (tagged2ltag(x)) {
    case LTAG_REF00:
      if (!x) 
	continue;
    case LTAG_REF01:
    case LTAG_REF10:
    case LTAG_REF11:
      x_ptr = tagged2Ref(x);
      x     = *x_ptr;
      goto again;
      
    case LTAG_MARK0:
    case LTAG_MARK1:
    case LTAG_SMALLINT:
      continue;

    case LTAG_LITERAL:
      if (!tagged2Literal(x)->checkSituatedness())
	bads = oz_cons(x,bads);
      continue;
      
    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
      if (!tagged2LTuple(x)->cacIsMarked())
	scStack.push(x);
      continue;

    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      if (!tagged2SRecord(x)->cacIsMarked())
	scStack.push(x);
      continue;
     
    case LTAG_CONST0:
    case LTAG_CONST1:
      if (!tagged2Const(x)->checkSituatedness())
	bads = oz_cons(x,bads);
      continue;

    case LTAG_VAR0: 
    case LTAG_VAR1: 
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

