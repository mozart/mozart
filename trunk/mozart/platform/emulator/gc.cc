/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Per Brand (perbrand@sics.se)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

/****************************************************************************
 ****************************************************************************/

// mm2: with pseudo shallow guard dead value may appear in the wrong
// space, because threads are not sited
#define DEEP_GARBAGE

#include "gc.hh"
#include "var_base.hh"
#include "fdomn.hh"
#include "dictionary.hh"
#include "os.hh"
#include "value.hh"
#include "codearea.hh"
#include "var_fd.hh"
#include "var_fs.hh"
#include "var_bool.hh"
#include "var_of.hh"
#include "var_ct.hh"
#include "var_future.hh"
#include "var_simple.hh"
#include "var_ext.hh"
#include "thr_int.hh"
#include "solve.hh"
#include "debug.hh"
#include "pointer-marks.hh"
#include "dpInterface.hh"
#include "gname.hh"
#include "interFault.hh"

// loeckelt (for big fsets)
#include "mozart_cpi.hh"

// hack alert: usage #pragma interface requires this
#ifdef OUTLINE
#define inline
#endif

/*
 * isCollecting: collection is running
 * isInGc:       garbage collector does garbage collection, otherwise it clones
 *
 */

Bool isCollecting = NO;
static Bool isInGc;

#ifdef CS_PROFILE
int32 * cs_copy_start = NULL;
int32 * cs_orig_start = NULL;
int     cs_copy_size  = 0;
#endif

/*
 * Forward reference
 */

static void gcCode(CodeArea *block);
static void gcCode(ProgramCounter PC);

/*
 * CHECKSPACE -- check if object is really copied from heap
 *   has as set of macros:
 *    INITCHECKSPACE - save pointer to from-space & print from-space
 *    NOTINTOSPACE   - assert not in to-space
 *    INTOSPACE      - assert in to-space
 * NOTE: this works only for chunk
 */

#ifdef DEBUG_GC

static MemChunks *fromSpace;

Bool inToSpace(void *p) {
  return (!isInGc || p==NULL || MemChunks::list->inChunkChain(p));
}

Bool notInToSpace(void *p) {
  return (!isInGc || p==NULL || !MemChunks::list->inChunkChain(p));
}

Bool inFromSpace(void *p) {
  return (!isInGc || p==NULL || fromSpace->inChunkChain(p));
}

void initCheckSpace() {
  fromSpace = MemChunks::list;
  DebugGCT(printf("FROM-SPACE:\n");
	   fromSpace->print();)
}

void exitCheckSpace() {
  DebugGCT(printf("TO-SPACE:\n");
	   MemChunks::list->print();)
}


#define GCDBG_INFROMSPACE(P)  Assert(inFromSpace(P))
#define GCDBG_NOTINTOSPACE(P) Assert(notInToSpace(P))
#define GCDBG_INTOSPACE(P)    Assert(inToSpace(P))
#define GCDBG_INITSPACE       initCheckSpace()
#define GCDBG_EXITSPACE       exitCheckSpace()

#else

#define GCDBG_INFROMSPACE(P)
#define GCDBG_NOTINTOSPACE(P)
#define GCDBG_INTOSPACE(P)
#define GCDBG_INITSPACE
#define GCDBG_EXITSPACE

#endif




/*
 * Allocate and copy memory blocks.
 * 
 */

inline
void * gcReallocStatic(void * p, size_t sz) {
  // Use for blocks where size is known statically at compile time
  DebugCheck(sz%sizeof(int) != 0,
	     OZ_error("gcReallocStatic: can only handle word sized blocks"););

  if (sz > 12) {
    return memcpy(freeListMalloc(sz), p, sz);
  } else {
    register int32 * frm = (int32 *) p;
    register int32 * to  = (int32 *) freeListMalloc(sz);
  
    switch(sz) {
    case 12:
      to[2]=frm[2];
    case 8:
      to[1]=frm[1];
    case 4:
      to[0]=frm[0];
      break;
    default:
      Assert(0);
    }

    return to;
  }
}


/*
 * The garbage collector uses an explicit recursion stack. The stack
 * items are references where garbage collection must continue.
 *
 * The items are tagged pointers, the tag gives which routine must 
 * continue, whereas the pointer itself says with which entity.
 *
 */

enum TypeOfPtr {
  PTR_LTUPLE,
  PTR_SRECORD,
  PTR_BOARD,
  PTR_ACTOR,
  PTR_THREAD,
  PTR_PROPAGATOR,
  PTR_CVAR,
  PTR_CONSTTERM,
  PTR_EXTENSION
};


typedef TaggedRef TypedPtr;

class GcStack: public Stack {
public:
  GcStack() : Stack(1024, Stack_WithMalloc) {}
  ~GcStack() {}
  
  void push(void * ptr, TypeOfPtr type) {
    Stack::push((StackEntry) makeTaggedRef2p((TypeOfTerm) type, ptr));
  }

  void recurse(void);

};

static GcStack gcStack;


/*
 * The copy trail: CpTrail
 *
 * During copying fields that are overwritten must be saved in order 
 * to reestablish the space that has been copied.
 *
 */

class CpTrail: public Stack {
public:
  CpTrail() : Stack(1024, Stack_WithMalloc) {}
  ~CpTrail() {}
  
  void save(int * p) {
    // Save content and address
    ensureFree(2);
    push((StackEntry) *p, NO);
    push((StackEntry) p,  NO);
  }

  void unwind(void) {
    while (!isEmpty()) {
      int * p = (int *) pop();
      int   v = (int)   pop();
      *p = v;
    } 
  }
};

static CpTrail cpTrail;


/****************************************************************************
 * GCMARK
 ****************************************************************************/

/* 
 * set: only used in conjunction with the function setHeapCell ???
 */

/*
 * Check if object in from-space (elem) is already collected.
 *   Then return the forward pointer to to-space.
 */
#define CHECKCOLLECTED(elem,Type)  \
if (GCISMARKED(elem)) {return (Type) GCUNMARK(elem);}


/*
 * Write a marked forward pointer (pointing into the to-space)
 * into a structure in the from-space.
 *
 *  If mode is IN_GC, store value in cell ptr only;
 *  else save cell at ptr and also store in this cell.
 *
 */
inline
void storeFwdMode(Bool isInGc, int32* fromPtr, void *newValue, 
		  Bool domark=OK) {
  if (!isInGc)
    cpTrail.save(fromPtr);

  GCDBG_INFROMSPACE(fromPtr);

  *fromPtr = domark ? GCMARK(newValue) : ToInt32(newValue);
}

inline
void storeFwd(int32* fromPtr, void *newValue, Bool domark=OK) {
  storeFwdMode(isInGc, fromPtr, newValue, domark);
}

#define storeFwdField(d,t) \
  storeFwd((int32*) d->gcGetMarkField(), t, NO); d->gcMark(t);

//*****************************************************************************
//               Functions to gc external references into heap
//*****************************************************************************

class ExtRefNode;
static ExtRefNode *extRefs = NULL;

class ExtRefNode {
public:
  USEHEAPMEMORY;

  TaggedRef *elem;
  ExtRefNode *next;

  ExtRefNode(TaggedRef *el, ExtRefNode *n): elem(el), next(n){ Assert(elem); }

  void remove()                  { elem = NULL; }
  ExtRefNode *add(TaggedRef *el) { return new ExtRefNode(el,this); }
  
  ExtRefNode *gc()
  {
    ExtRefNode *aux = this;
    ExtRefNode *ret = NULL;
    while(aux) {
      if (aux->elem) {
	ret = new ExtRefNode(aux->elem,ret);
	OZ_collectHeapTerm(*ret->elem,*ret->elem);
      }
      aux = aux->next;
    }
    return ret;
  }


  ExtRefNode *protect(TaggedRef *el)
  {
    Assert(oz_isRef(*el) || !oz_isVariable(*el));
    Assert(!find(el));
    return add(el);
  }


  Bool unprotect(TaggedRef *el) 
  {
    Assert(el);
    ExtRefNode *aux = extRefs;
    while(aux) {
      if (aux->elem==el) {
	aux->remove();
	return OK;
      }
      aux = aux->next;
    }
    return NO;
  }


  ExtRefNode *find(TaggedRef *el)
  {
    Assert(el);
    ExtRefNode *aux = extRefs;
    while(aux) {
      if (aux->elem==el)
	break;
      aux = aux->next;
    }
    return aux;
  }
};


inline
Bool needsCollection(Literal *l)
{
  if (l->isAtom()) return NO;
  Name *nm = (Name*) l;
  return nm->isOnHeap();
}


Bool needsNoCollection(TaggedRef t)
{
  Assert(t != makeTaggedNULL());

  TypeOfTerm tag = tagTypeOf(t);
  return isSmallIntTag(tag) ||
	 isLiteralTag(tag) && !needsCollection(tagged2Literal(t));
}


Bool oz_protect(TaggedRef *ref)
{
  extRefs = extRefs->protect(ref);
  return OK;
}

/* protect a ref, that will never change its initial value
 *  --> no need to remember it, if it's a small int or atom 
 */
Bool oz_staticProtect(TaggedRef *ref)
{
  if (needsNoCollection(*ref))
    return OK;

  return oz_protect(ref);
}

Bool oz_unprotect(TaggedRef *ref)
{
  ExtRefNode *aux = extRefs->find(ref);

  if (aux == NULL)
    return NO;

  aux->remove();
  return OK;
} 

/*
 * Garbage collection needs to be aware of certain objects, e.g.,
 * since these objects store references into the heap. The gc-routine
 * of `GCMeManager' is called after all collection has been done, such
 * that the individual gc routines of the objects can avoid copying
 * references that are only established by themselves (in other words,
 * memory leaks can be avoided).
 */

#ifdef NEW_NAMER
GCMeManager * GCMeManager::_head;
#endif

/*
 * The variable copying stack: VarFix
 *
 * When during garbage collection or during copying a variable V is
 * encountered that has not been collected so far and V is not direct, 
 + that is, V has been reached by a reference chain, V cannot be copied
 * directly.
 *
 * So, push the location of the reference on VarFix and replace its content
 * by a reference to the old variable, as to shorten the ref-chain.
 *
 * Later V might be reached directly, that fixes V's location. After 
 * collection has finished, VarFix tracks this new location to
 * and fixes the occurence on VarFix.
 *
 */

class VarFix: public Stack {
public:
  VarFix() : Stack(1024, Stack_WithMalloc) {}
  ~VarFix() {}

  void defer(TaggedRef * var, TaggedRef * ref) {
    Assert(var);
    Stack::push((StackEntry) ref);
    *ref = makeTaggedRef(var); 
  }

  void fix(void);

};

static VarFix varFix;



/*
 * Routines to check whether entities are local to space to be cloned
 *
 * - Boards are marked
 * - For suspension list entries use isInTree 
 *   (This routine is only needed for debugging, since in a stable space
 *    there are no external suspensions)
 *
 */


/*
 * Copying: the board to be copied
 */
static Board * fromCopyBoard;



/*
 * Copying: Check if suspension entry is local to copyBoard.
 */

Bool Board::isInTree(void) {
  Assert(!isInGc);
  Assert(this);

  Board * b = this;

  while (1) {
    Assert (!b->isCommitted());

    if (b == fromCopyBoard) 
      return OK;
  
    if (b->isMarkedGlobal()) 
      return NO;

    b = b->getParentAndTest();
  
    if (!b) 
      return NO;
  }

}



/****************************************************************************
 * Collect all types of terms
 ****************************************************************************/

// This procedure derefences cluster chains and collects only the object at 
// the end of such a chain.


#define RAGCTag (1<<31)

inline Bool refsArrayIsMarked(RefsArray r)
{
  return (r[-1]&RAGCTag);
}

inline void refsArrayMark(RefsArray r, void *ptr)
{
  storeFwd((int32*)&r[-1],ToPointer(ToInt32(ptr)|RAGCTag),NO);
}

inline RefsArray refsArrayUnmark(RefsArray r)
{
  return (RefsArray) ToPointer(r[-1]&(~(RAGCTag)|mallocBase));
}


// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc tag set --> has already been copied

RefsArray gcRefsArray(RefsArray r) {
  if (r == NULL)
    return r;

  GCDBG_NOTINTOSPACE(r);

  if (refsArrayIsMarked(r)) {
    return refsArrayUnmark(r);
  }

  Assert(!isFreedRefsArray(r));

  int sz = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(sz,NO);

  refsArrayMark(r,aux);

  OZ_collectHeapBlock(r, aux, sz);
  
  return aux;
}

inline
Abstraction *gcAbstraction(Abstraction *a) {
  return (Abstraction *) a->gcConstTerm();
}

inline  
Bool Board::gcIsMarked(void) {
  return (Bool) gcField;
}

inline
void Board::gcMark(Board * fwd) {
  if (!isInGc)
    cpTrail.save((int32 *) &gcField);
  gcField = fwd;
}

inline
Board * Board::gcGetFwd(void) {
  Assert(gcIsMarked());
  return gcField;
}

Board * Board::gcBoard() {
  GCDBG_INFROMSPACE(this);

  if (!this) return 0;

  Board * bb = derefBoard();

  Assert(bb);

  if (bb->gcIsMarked())
    return bb->gcGetFwd();
  
  if (!bb->gcIsAlive()) 
    return 0;

  Assert(isInGc || bb->isInTree()); 

  Board *ret = (Board *) oz_hrealloc(bb, sizeof(Board));

  gcStack.push(ret,PTR_BOARD);

  bb->gcMark(ret);

  return ret;
}

void dogcGName(GName *gn) {
  if (gn && isInGc) 
    gcGName(gn);
}

/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

Name *Name::gcName() {
  CHECKCOLLECTED(homeOrGName, Name *);
  GName * gn = NULL;

  if (hasGName()) {
    gn = getGName1();
  }

  if (isInGc && isOnHeap() ||
      !isInGc && !(GETBOARD(this))->isMarkedGlobal()) {

    Name *aux = (Name*) gcReallocStatic(this,sizeof(Name));

    storeFwd(&homeOrGName, aux);
    
    if (gn) {
      dogcGName(gn);
    } else {
      aux->homeOrGName = 
	ToInt32(((Board*)ToPointer(aux->homeOrGName))->gcBoard());
    }

    return aux;

  } else {
    dogcGName(gn);
    return this;
  }
}

inline 
Literal *Literal::gc() {
  if (isAtom()) return this;
  return ((Name*) this)->gcName();
}

Object *Object::gcObject() {
  return (Object *) gcConstTerm();
}

/*
 *  Preserve runnable threads which home board is dead, because 
 * solve counters have to be updated (while, of course, discard 
 * non-runnable ones);
 *  If threads is dead, returns (Thread *) NULL;
 */

inline
Thread *Thread::gcThreadInline() {
  if (!this)
    return (Thread *) NULL;

  if (gcIsMarked())
    return (Thread *) gcGetFwd();

  if (isDeadThread()) 
    return (Thread *) NULL;

  //  Some invariants:
  // nothing can be copied (IN_TC) until stability;

  // first class threads: must only copy thread when local to solve!!!
  if (!isInGc && (GETBOARD(this))->isMarkedGlobal())
    return this;

  Assert(isInGc || !isRunnable());

#ifdef DEBUG_THREADCOUNT
  if (!(isInGc || !isRunnable())) {
    printf("runnable propagator while cloning\n");
    fflush(stdout);
  }
#endif

  // Note that runnable threads can be also counted 
  // in solve actors (for stability check), and, therefore, 
  // might not just dissappear!
  if (isSuspended() && !GETBOARD(this)->gcIsAlive())
    return (Thread *) NULL;
  
  Thread * newThread = (Thread *) gcReallocStatic(this, sizeof(Thread));

  gcStack.push(newThread, PTR_THREAD);

  storeFwdField(this, newThread);

  return newThread;
}

Thread * Thread::gcThread(void) {
  return gcThreadInline();
}

//-----------------------------------------------------------------------------
// gc routines of `Propagator'

inline
Bool Propagator::gcIsMarked(void)
{
  return ISMARKEDFLAG(P_gcmark);
}

Bool Propagator::gcIsMarkedOutlined(void)
{
  return gcIsMarked();
}

inline 
void Propagator::gcMark(Propagator * fwd)
{
  Assert(!gcIsMarked());

  if (!isInGc)
    cpTrail.save((int32 *) &_flags);

  _flags = (int32) fwd;
  MARKFLAG(P_gcmark);
}

inline
void ** Propagator::gcGetMarkField(void)
{
  return (void **) (void *) &_flags;
}

inline 
Propagator * Propagator::gcGetFwd(void)
{
  return UNMARKFLAGTO(Propagator *, P_gcmark);
}

Propagator * Propagator::gcGetFwdOutlined(void)
{
  return gcGetFwd();
}

inline 
Propagator * Propagator::gcPropagator(void)
{
  if (gcIsMarked())
    return (Propagator *) gcGetFwd();

  if (isDeadPropagator())
    return NULL;

  Board * bb = _b;

  if (bb && bb->gcIsAlive()) {
    Assert(isInGc || (!isRunnable()));
    
    Assert(!bb->derefBoard()->isMarkedGlobal());

    Propagator * newPropagator 
      = (Propagator *) heapMalloc(sizeof(Propagator));
    
    gcStack.push(newPropagator, PTR_PROPAGATOR);
    
    newPropagator->_b     = bb;
    newPropagator->_p     = this->_p;
    newPropagator->_flags = this->_flags;

    this->gcMark(newPropagator);
    
    return newPropagator;
  } 
  return NULL;
}

Propagator * Propagator::gcPropagatorOutlined(void)
{
  return gcPropagator();
}

inline 
void Propagator::gcRecurse(void)
{

  Assert(_b);

  _b = _b->gcBoard();

  Assert(_b);

  _p = _p->gc();

  Assert(_p);

  _p->updateHeapRefs(isInGc);

}

inline 
Suspension Suspension::gcSuspension(void)
{
  if (_isThread()) {
    return _getThread()->gcThreadInline();
  } else {
    return _getPropagator()->gcPropagator();
  }
}

/*
 *  We reverse the order of the list, but this should be no problem.
 *
 * kost@ : ... in any case, this is complaint with the 
 * 'The Definition of Kernel Oz';
 *
 */
inline
SuspList * SuspList::gc(void) {
  SuspList * ret = NULL;

  for (SuspList * help = this; help != NULL; help = help->getNext()) {
    Suspension susp = help->getSuspension().gcSuspension();
    if (! susp.isNull()) {
      ret = new SuspList(susp, ret);
    }
  }

  return (ret);
}




inline  
Bool OzVariable::gcIsMarked(void) {
  return IsMarkedPointer(suspList);
}


Bool OzVariable::gcIsMarkedOutlined(void) {
  return gcIsMarked();
}

inline
void OzVariable::gcMark(Bool isInGc, TaggedRef * fwd) {
  Assert(!gcIsMarked());
  if (!isInGc)
    cpTrail.save((int32 *) &suspList);
  suspList = (SuspList *) MarkPointer(fwd);
}

inline
TaggedRef * OzVariable::gcGetFwd(void) {
  Assert(gcIsMarked());
  return (TaggedRef *) UnMarkPointer(suspList);
}

TaggedRef * OzVariable::gcGetFwdOutlined(void) {
  return gcGetFwd();
}

inline
void OZ_FiniteDomainImpl::gc(void) {
  copyExtension();
}

inline
void OzFDVariable::gc(OzFDVariable * frm) {
  finiteDomain = frm->finiteDomain;
  ((OZ_FiniteDomainImpl *) &finiteDomain)->gc();
  
  for (int i = fd_prop_any; i--; )
    fdSuspList[i] = frm->fdSuspList[i]->gc();
}

inline
void OzBoolVariable::gc(OzBoolVariable * frm) {
  store_patch = frm->store_patch;  
}

inline
void OzFSVariable::gc(OzFSVariable * frm) {
  _fset = frm->_fset;

#ifdef BIGFSET
  _fset.copyExtension();
#endif

  for (int i = fs_prop_any; i--; )
    fsSuspList[i] = frm->fsSuspList[i]->gc();
}


OzVariable * OzCtVariable::gc(void) 
{
  OzCtVariable * to = new OzCtVariable(* (OzCtVariable*) this);

  // common stuff
  to->u        = u;
  to->suspList = suspList;
  to->setHome(getHome1());
  
  // stuff specific to `OzCtVariable's
  to->_constraint = _constraint;
  to->_definition = _definition;

  // gc suspension lists
  int noOfSuspLists = getNoOfSuspLists();
  // copy
  to->_susp_lists = (SuspList **) 
    heapMalloc(sizeof(SuspList *) * noOfSuspLists);
  // collect
  for (int i = noOfSuspLists; i--; )
   to->_susp_lists[i] = _susp_lists[i]->gc();

  return to;
}


void OzCtVariable::gcRecurse(void)
{
  // constraint (must go in `gcRecurse' since it may contain recursion
  _constraint = _constraint->copy();  
}


OzVariable * OzVariable::gcVar(void) {
  GCDBG_INFROMSPACE(this);

  Assert(!gcIsMarked())
    
  Board * bb = getHome1()->gcBoard();
  
#ifdef DEEP_GARBAGE
  if (!bb) return 0;
#endif
  Assert(bb);

  SuspList * sl = suspList->gc();
  
  OzVariable * to;
  
  switch (getType()){
  case OZ_VAR_FD:
    to = new OzFDVariable((DummyClass *)0);
    ((OzFDVariable *) to)->gc((OzFDVariable *) this);
    to->u        = u;
    to->suspList = sl;
    to->setHome(bb);
    return to;

  case OZ_VAR_BOOL:
    to = new OzBoolVariable((DummyClass*)0);
    ((OzBoolVariable *) to)->gc((OzBoolVariable *) this);
    to->u        = u;
    to->suspList = sl;
    to->setHome(bb);
    return to;

  case OZ_VAR_FS:
    to = new OzFSVariable((DummyClass*)0);
    ((OzFSVariable *) to)->gc((OzFSVariable *) this);
    to->u        = u;
    to->suspList = sl;
    to->setHome(bb);
    return to;


  case OZ_VAR_SIMPLE: to = ((SimpleVar *)this)->gc(); break;
  case OZ_VAR_FUTURE: to = ((Future *)this)->gc(); break;
  case OZ_VAR_OF:     to = new OzOFVariable(*(OzOFVariable*) this);break;
  case OZ_VAR_CT:     to = ((OzCtVariable*)this)->gc(); break;
  case OZ_VAR_EXT:    to = ((ExtVar *)this)->gcV(); break;
  default: 
    Assert(0);
  }

  // The generic part
  Assert(!isInGc || this->getHome1() != bb);

  gcStack.push(to, PTR_CVAR);

  to->suspList = sl;
  to->setHome(bb);

  return to;
}

inline
DynamicTable * DynamicTable::gc(void) {
  Assert(isPwrTwo(size));

  // Copy the table:
  DynamicTable * to = (DynamicTable *) heapMalloc((size-1)*sizeof(HashElement)
						  + sizeof(DynamicTable));
  to->numelem = numelem;
  to->size    = size;

  HashElement * ft = table;
  HashElement * tt = to->table;

  for (dt_index i=size; i--; ) 
    if (ft[i].ident) {
      OZ_collectHeapTerm(ft[i].ident, tt[i].ident);
      OZ_collectHeapTerm(ft[i].value, tt[i].value);
    } else {
      tt[i].ident = makeTaggedNULL();
      tt[i].value = makeTaggedNULL();
    }

  return to;
}


void OzOFVariable::gcRecurse(void) {
  OZ_collectHeapTerm(label,label);
  // Update the pointer in the copied block:
  dynamictable=dynamictable->gc();
}


void OzVariable::gcVarRecurse(void) {
  
  switch (getType()) {
  case OZ_VAR_SIMPLE:  ((SimpleVar *)this)->gcRecurse(); break;
  case OZ_VAR_FUTURE:  ((Future *)this)->gcRecurse(); break;
  case OZ_VAR_BOOL:    Assert(0); break;
  case OZ_VAR_FD:      Assert(0); break;
  case OZ_VAR_OF:      ((OzOFVariable*)this)->gcRecurse(); break;
  case OZ_VAR_FS:      Assert(0); break;
  case OZ_VAR_CT:      ((OzCtVariable*)this)->gcRecurse(); break;
  case OZ_VAR_EXT:     ((ExtVar *)this)->gcRecurseV(); break;
  default: 
    Assert(0);
  }

}


/*
 * Float
 * WARNING: the value field of floats has no bit left for a gc mark
 *   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
 */

inline
Float *Float::gc() {
  Assert(isInGc);

  return newFloat(value);
}


inline
FSetValue * FSetValue::gc(void) {
  Assert(isInGc);

#ifdef BIGFSET
  FSetValue *retval = (FSetValue *) oz_hrealloc(this, sizeof(FSetValue));
  retval->_IN.copyExtension();
  return retval;
#else
  return (FSetValue *) oz_hrealloc(this, sizeof(FSetValue));
#endif
}


BigInt * BigInt::gc() {
  Assert(isInGc);
  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  return ret;
}


void Script::gc() {

  if (first){
    int sz = numbOfCons*sizeof(Equation);
    
    Equation *aux = (Equation*) heapMalloc(sz);

    for (int i = numbOfCons; i--; ){
#ifdef DEBUG_CHECK
      //  This is the very useful consistency check.
      //  'Equations' with non-variable at the left side are figured out;
      TaggedRef auxTerm = first[i].left;
      TaggedRef *auxTermPtr;
      if (!isInGc && oz_isRef(auxTerm)) {
	do {
	  if (auxTerm == makeTaggedNULL ())
	    OZ_error ("NULL in script");
	  if (GCISMARKED(auxTerm)) {
	    auxTerm = ToInt32(GCUNMARK(auxTerm));
	    continue;
	  }
	  if (oz_isRef (auxTerm)) {
	    auxTermPtr = tagged2Ref (auxTerm);
	    auxTerm = *auxTermPtr;
	    continue;
	  }
	  if (oz_isVariable (auxTerm))
	    break;   // ok;
	  OZ_error ("non-variable is found at left side in Script");
	} while (1);
      }
#endif
      OZ_collectHeapTerm(first[i].left,  aux[i].left); 
      OZ_collectHeapTerm(first[i].right, aux[i].right);
    }
    
    first = aux;
  }
}


/*
 *  Thread items methods;
 *
 */
//
//  RunnableThreadBody;


RunnableThreadBody *RunnableThreadBody::gcRTBody () {
  RunnableThreadBody *ret = 
    (RunnableThreadBody *) gcReallocStatic(this, sizeof(RunnableThreadBody));

  taskStack.gc(&ret->taskStack);

  return (ret);
}

/* collect LTuple, SRecord */

inline
LTuple * LTuple::gc() {
  // Does basically nothing, the real stuff is in gcRecurse

  GCDBG_INFROMSPACE(this);

  if (GCISMARKED(args[0]))
    return (LTuple *) GCUNMARK(args[0]);
  
  LTuple * to = (LTuple *) heapMalloc(sizeof(LTuple));

  // Save the content
  to->args[0] = args[0];

  // Do not store foreward! gcRecurse takes care of this!
  args[0] = GCMARK(to->args);

  gcStack.push(this, PTR_LTUPLE);

  return to;
}

inline
SRecord *SRecord::gcSRecord() {
  if (this==NULL)
    return NULL;

  CHECKCOLLECTED(label, SRecord *);
  
  int len = (getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord);

  SRecord *ret = (SRecord*) heapMalloc(len);
  
  ret->label       = label;
  ret->recordArity = recordArity;

  storeFwd((int32*)&label, ret);

  gcStack.push(this, PTR_SRECORD);

  return ret;
}

// mm2
Thread *Thread::gcDeadThread() {
  Assert(isDeadThread());

  Thread *newThread = (Thread *) gcReallocStatic(this,sizeof(Thread));

  GCDBG_INTOSPACE(am.rootBoardGC());
  newThread->setBoardInternal(am.rootBoardGC());

  storeFwdField(this, newThread);
  setSelf(0);
  setAbstr(0);

  return (newThread);
}


OZ_Propagator * OZ_Propagator::gc(void) {
  OZ_Propagator * to = (OZ_Propagator *) oz_hrealloc(this, sizeOf());

  return to;
}

inline
void Thread::gcRecurse ()
{
  Board *newBoard = getBoardInternal()->gcBoard ();
  if (!newBoard) {
    // 
    //  The following assertion holds because suspended threads
    // which home board is dead are filtered out during 
    // 'Thread::gcThread ()';
    Assert (isRunnable ());
      
    // 
    //  Actually, there are two cases: for runnable threads with 
    // a taskstack, and without it (note that the last case covers 
    // also the GC'ing of propagators);
    Board *notificationBoard=getBoardInternal()->gcGetNotificationBoard();
    setBoardInternal(notificationBoard->gcBoard());
      
    GETBOARD(this)->incSuspCount ();
      
    //
    //  Convert the thread to a 'wakeup' type, and just throw away
    // the body;
    setWakeUpTypeGC ();
    item.threadBody = (RunnableThreadBody *) NULL;
    return;
  } else {
    setBoardInternal(newBoard);
  }

  //
  switch (getThrType ()) {
  case S_RTHREAD:
    item.threadBody = item.threadBody->gcRTBody ();
    setSelf(getSelf()->gcObject());
    break; 

  case S_WAKEUP: 
    //  should not contain any reference;
    Assert(item.threadBody == (RunnableThreadBody *) NULL);
    break;

  default:
    Assert(0);
  }
}

ForeignPointer * ForeignPointer::gc(void) {
  ForeignPointer * ret =
    (ForeignPointer*) gcReallocStatic(this,sizeof(ForeignPointer));
  ret->ptr = ptr;

  storeFwdField(this, ret);
  return ret;
}

// ===================================================================
// Extension

TaggedRef gcExtension(TaggedRef term)
{
  OZ_Extension *ex = oz_tagged2Extension(term);
  if (ex == NULL) {
    return makeTaggedNULL();
  }

  // hack alert: write forward into vtable!
  if ((*(int32*)ex)&1) {
    return oz_makeTaggedExtension((OZ_Extension *)ToPointer((*(int32*)ex)&~1));
  }

  Board *bb=(Board*)(ex->__getSpaceInternal());
  if (bb) {
    bb = bb->derefBoard();
    if (!bb->gcIsAlive()) return makeTaggedNULL();
    if (!isInGc && bb->isMarkedGlobal()) return term;
  }
  OZ_Extension *ret = ex->gcV();
  if (bb) ret->__setSpaceInternal(bb);

  gcStack.push(ret,PTR_EXTENSION);

  int32 *fromPtr = (int32*)ex;
  if (!isInGc) cpTrail.save(fromPtr);
  *fromPtr = ToInt32(ret)|1;

  return oz_makeTaggedExtension(ret);
}

void gcExtensionRecurse(OZ_Extension *ex)
{
  Board *bb=(Board*) (ex->__getSpaceInternal());
  if (bb) ex->__setSpaceInternal(bb->gcBoard());
  ex->gcRecurseV();
}

// ===================================================================
// Finalization

extern OZ_Term guardian_list;
extern OZ_Term finalize_list;
extern OZ_Term finalize_handler;

void gc_finalize()
{
  // go through the old guardian list
  OZ_Term old_guardian_list = guardian_list;
  guardian_list = finalize_list = oz_nil();
  if (old_guardian_list==0) return;
  while (!oz_isNil(old_guardian_list)) {
    OZ_Term pair = oz_head(old_guardian_list);
    old_guardian_list = oz_tail(old_guardian_list);
    OZ_Term obj = oz_head(pair);
    switch (tagTypeOf(obj)) {
    case EXT	:
      // same check as Michael's hack in gcExtension
      if ((*(int32*)oz_tagged2Extension(obj))&1)
	// reachable through live data
	guardian_list = oz_cons(pair,guardian_list);
      else
	// unreachable
	finalize_list = oz_cons(pair,finalize_list);
      break;
    case OZCONST:
      if (tagged2Const(obj)->gcIsMarked())
	// reachable through live data
	guardian_list = oz_cons(pair,guardian_list);
      else
	// unreachable
	finalize_list = oz_cons(pair,finalize_list);
      break;
    default	:
      Assert(0);
    }
  }
  // gc both these list normally.
  // since these lists have been freshly consed in the new half space
  // this simply means to go through both and gc the pairs
  // in the head of each cons
  for (OZ_Term l=guardian_list;!oz_isNil(l);l=oz_tail(l)) {
    LTuple *t = tagged2LTuple(l);
    OZ_collectHeapTerm(*t->getRefHead(),*t->getRefHead());
  }
  for (OZ_Term l1=finalize_list;!oz_isNil(l1);l1=oz_tail(l1)) {
    LTuple *t = tagged2LTuple(l1);
    OZ_collectHeapTerm(*t->getRefHead(),*t->getRefHead());
  }
  // if the finalize_list is not empty, we must create a new
  // thread (at top level) to effect the finalization phase
  if (!oz_isNil(finalize_list)) {
    Thread* thr = oz_newThreadInject(oz_rootBoard());
    thr->pushCall(finalize_handler,finalize_list);
    finalize_list = oz_nil();
  }
}


inline
void gcTagged(TaggedRef & frm, TaggedRef & to) {
#ifdef DEBUG_GC
  Assert(!isInGc || !fromSpace->inChunkChain(&to));
#endif

  TaggedRef aux = frm;

  switch (tagTypeOf(aux)) {
    
  case REF:
    /* initalized but unused cell in register array */
    if (aux == makeTaggedNULL()) {
      to = aux;
      return;
    }
    // fall through

  case REFTAG2:
  case REFTAG3:
  case REFTAG4:
    {
      TaggedRef * aux_ptr;
    
      do {
	aux_ptr = tagged2Ref(aux);
	aux     = *aux_ptr;
      } while (oz_isRef(aux));
      
      switch (tagTypeOf(aux)) {
	// The following cases never occur, but to allow for better code 
      case REF: case REFTAG2: case REFTAG3: case REFTAG4: {}
	// All the following jumps are resolved to jumps in the switch-table!
      case GCTAG:     goto DO_GCTAG;
      case SMALLINT:  goto DO_SMALLINT;
      case FSETVALUE: goto DO_FSETVALUE;
      case LITERAL:   goto DO_LITERAL;
      case EXT:       goto DO_EXT;
      case LTUPLE:    goto DO_LTUPLE;
      case SRECORD:   goto DO_SRECORD;
      case OZFLOAT:   goto DO_OZFLOAT;
      case OZCONST:   goto DO_OZCONST;

      case UNUSED_VAR: // FUT

      case UVAR: // non-direct var: delay collection
	{
	  Board * bb = tagged2VarHome(aux)->derefBoard();
	  
	  if (!isInGc && bb->isMarkedGlobal()) {
	    to = makeTaggedRef(aux_ptr);
	    return;
	  }
	  
	  Assert(isInGc || !bb->isMarkedGlobal());
	  
	  bb = bb->gcBoard();

#ifdef DEEP_GARBAGE
	  if (!bb) {
	    to = 0;
	    return;
	  }
#endif
	  Assert(bb);
	  varFix.defer(aux_ptr, &to);
	  return;
	}

      case CVAR:
	{
	  OzVariable * cv = tagged2CVar(aux);

	  if (cv->gcIsMarked()) {
	    Assert(tagTypeOf(*(cv->gcGetFwd())) == CVAR);
	    to = makeTaggedRef(cv->gcGetFwd());
	  } else if (isInGc || !(GETBOARD(cv))->isMarkedGlobal()) {
	    Assert(isInGc || !(GETBOARD(cv))->isMarkedGlobal());
	    OzVariable *new_cv=cv->gcVar();

#ifdef DEEP_GARBAGE
	    if (!new_cv) {
	      to=0;
	      return;
	    }
#endif
	    Assert(new_cv);

	    TaggedRef * var_ptr = newTaggedCVar(new_cv); 
	    to = makeTaggedRef(var_ptr);
	    cv->gcMark(isInGc, var_ptr);
	  } else {
	    to = makeTaggedRef(aux_ptr);
	  }
	  return;
	}
    
      }
      
      Assert(NO);
    }
      
  case GCTAG: DO_GCTAG:
    to = makeTaggedRef((TaggedRef*) GCUNMARK(aux));
    // This can lead to not shortened ref chains together with
    // the CONS forwarding: if a CONS cell is collected, then every
    // reference to the first element becomes a ref. May try this:
    // if (!isVar(*to)) to=deref(to); (no, cycles... CS)
    return;
    
  case SMALLINT: DO_SMALLINT:
    to = aux;
    return;

  case FSETVALUE: DO_FSETVALUE:
    if (isInGc) {
      to = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(aux))->gc()); 
    } else {
      to = aux;
    }
    return;
    
  case LITERAL: DO_LITERAL:
      to = makeTaggedLiteral(tagged2Literal(aux)->gc()); 
      return;

  case EXT: DO_EXT:
    to = gcExtension(aux);
    return;
    
  case LTUPLE: DO_LTUPLE:
    to = makeTaggedLTuple(tagged2LTuple(aux)->gc()); 
    return;

  case SRECORD: DO_SRECORD:
    to = makeTaggedSRecord(tagged2SRecord(aux)->gcSRecord()); 
    return;
    
  case OZFLOAT: DO_OZFLOAT:
    if (isInGc) {
      to = makeTaggedFloat(tagged2Float(aux)->gc());   
    } else {
      to = aux;
    }
    return;

  case OZCONST: DO_OZCONST:
#ifdef DEEP_GARBAGE
    {
      ConstTerm *ct = tagged2Const(aux)->gcConstTerm();
      to = ct ? makeTaggedConst(ct) : 0;
      return;
    }
#endif
    to = makeTaggedConst(tagged2Const(aux)->gcConstTerm());
    return;

  case UNUSED_VAR:  // FUT
    Assert(0); OZ_error("impossible");
    return;
  case UVAR: // direct var
    {
      Board * bb = tagged2VarHome(aux)->derefBoard();
      
      Assert(bb);

      if (isInGc || !bb->isMarkedGlobal()) {
	Assert(isInGc || !bb->isMarkedGlobal());
	bb = bb->gcBoard();
	Assert(bb);
	to = makeTaggedUVar(bb);
      } else {
	frm = makeTaggedRef(&to);
	to  = aux;
      }
      storeFwdMode(isInGc, (int32 *)&frm, &to);
    }
    return;

  case CVAR: // direct cvar
    //mm2: maybe this is dead code???
    {
      OzVariable * cv = tagged2CVar(aux);

      if (cv->gcIsMarked()) {
	Assert(tagTypeOf(*(cv->gcGetFwd())) == CVAR);
	to = makeTaggedRef(cv->gcGetFwd());
      } else if (isInGc || !(GETBOARD(cv))->isMarkedGlobal()) {
	to = makeTaggedCVar(cv->gcVar());
	cv->gcMark(isInGc, &to);
      } else {
	// We cannot copy the variable, but we have already copied
	// their taggedref, so we change the original variable to a ref 
	// of the copy.
	// After pushing on the update stack the
	// the original variable is replaced by a reference!
	Assert(!isInGc);
	frm = makeTaggedRef(&to);
	to  = aux;
	storeFwdMode(NO, (int32*) &frm, &to);
      }

    }
    return;
    
  }
}

// extern
void OZ_collect(OZ_Term *to) {
  OZ_collectHeapTerm(*to,*to);
}

void OZ_collectHeapTerm(TaggedRef & frm, TaggedRef & to) {
  gcTagged(frm, to);
}

void OZ_collectHeapBlock(TaggedRef * frm, TaggedRef * to, int sz) {
  for (int i=sz; i--; )
    gcTagged(frm[i], to[i]);
}

//*****************************************************************************
//                               AM::gc            
//*****************************************************************************


// This method is responsible for the heap garbage collection of the
// abstract machine, ie that all entry points into heap are properly 
// treated and references to variables are properly updated
void AM::gc(int msgLevel) {

  (*gcFrameToProxy)();

  isCollecting = OK;
  isInGc       = OK;

  ozstat.initGcMsg(msgLevel);
  
  MemChunks * oldChain = MemChunks::list;

#ifndef NEW_NAMER
  oz_varCleanup();  /* drop bound variables */
#endif

  GCDBG_INITSPACE;

  initMemoryManagement();

  for (int j=NumberOfXRegisters; j--; )
    xRegs[j] = 0;
  
  Assert(trail.isEmpty());
  Assert(cachedSelf==0);
  Assert(ozstat.currAbstr==NULL);
  Assert(_shallowHeapTop==0);
  Assert(_rootBoard);

  _rootBoard = _rootBoard->gcBoard();   // must go first!
  setCurrent(_currentBoard->gcBoard(),NO);

  aritytable.gc ();
  threadsPool.doGC();

#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc ();
#endif
  
  // mm2: Assert(isEmptySuspendVarList());
  emptySuspendVarList();

  OZ_collectHeapTerm(defaultExceptionHdl,defaultExceptionHdl);
  OZ_collectHeapTerm(debugStreamTail,debugStreamTail);

  CodeArea::gcCodeAreaStart();
  PrTabEntry::gcPrTabEntries();
  extRefs = extRefs->gc();
  
  OZ_collectHeapTerm(finalize_handler,finalize_handler);
  gcStack.recurse();
  gc_finalize();
  gcDeferWatchers();
  (*gcPerdioRoots)();
  gcStack.recurse();

  (*gcBorrowTableUnusedFrames)();
  gcStack.recurse();

#ifdef NEW_NAMER
  GCMeManager::gc();
  gcStack.recurse();
#endif
// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  varFix.fix();
  Assert(gcStack.isEmpty());


  GT.gcGNameTable();
  //   MERGECON gcPerdioFinal();
  gcSiteTable();
  (*gcPerdioFinal)();
  Assert(gcStack.isEmpty());

  GCDBG_EXITSPACE;

  CodeArea::gcCollectCodeBlocks();
  AbstractionEntry::freeUnusedEntries();

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  cachedStack = NULL;

  ozstat.printGcMsg(msgLevel);
  
  isCollecting = NO;
} // AM::gc


/*
 * After collection has finished, update variable references
 *
 */
void VarFix::fix(void) {

  if (isEmpty())
    return;
  
  do {
    TaggedRef * to = (TaggedRef *) pop();

    Assert(oz_isRef(*to));

    TaggedRef * aux_ptr = tagged2Ref(*to);
    TaggedRef   aux     = *aux_ptr;

    TaggedRef * to_ptr  = 
      (tagTypeOf(aux) == UVAR) ?
      newTaggedUVar(tagged2VarHome(aux)->derefBoard()->gcGetFwd()) :
      (TaggedRef *) GCUNMARK(aux);
    
    Assert(tagTypeOf(aux) == UVAR || tagTypeOf(aux) == GCTAG);
    
    *to = makeTaggedRef(to_ptr);
    storeFwd((int32 *) aux_ptr, to_ptr);
    
  } while (!isEmpty());

}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */

#ifdef CS_PROFILE
static Bool across_redid = NO;
#endif

Board* AM::copyTree(Board* bb) {

#ifdef CS_PROFILE
  across_redid  = NO;
  across_chunks = NO;
#endif

  isCollecting = OK;
  isInGc       = NO;

  unsigned int starttime = 0;

  if (ozconf.timeDetailed)
    starttime = osUserTime();

#ifdef CS_PROFILE
redo: 
  if (across_redid)
    OZ_error("Redoing cloning across chunk boundaries. Fuck!\n");
  
  if (across_chunks)
    across_redid = OK;

  across_chunks = NO;

  cs_orig_start = (int32 *) heapTop;
#endif

  Assert(!bb->isCommitted());
  fromCopyBoard = bb;

  bb->setGlobalMarks();

  Board * toCopyBoard = bb->gcBoard();

  Assert(toCopyBoard);

  gcStack.recurse();

  varFix.fix();
  
#ifdef NEW_NAMER
  if (am.isPropagatorLocation()) {
    GCMeManager::copyTree();
  }
#endif

  cpTrail.unwind();
  
  bb->unsetGlobalMarks();

  fromCopyBoard = NULL;
  
#ifdef CS_PROFILE
  if (across_chunks) {
    printf("Allocation across heap chunks. Redoing.\n");
    goto redo;
  }

  cs_copy_size = cs_orig_start - ((int32 *) heapTop);

  cs_orig_start = (int32 *) heapTop;
  
  cs_copy_start = (int32*) malloc(4 * cs_copy_size + 256);

  {
    int n = cs_copy_size;

    while (n) {
      *(cs_copy_start + n) = *(cs_orig_start + n);
      n--;
    }

  }
#endif

  if (ozconf.timeDetailed)
    ozstat.timeForCopy.incf(osUserTime()-starttime);

  isCollecting = NO;

  return toCopyBoard;
}

//*****************************************************************************
//                                GC-METHODS
//*****************************************************************************


/*
 * class Arity is not allocated on heap!
 * but must collect the list && the values in keytable
 */
inline
void Arity::gc()
{
  Arity *aux = this;
  while(aux) {
    if (!aux->isTuple()) {
      for (int i = aux->getSize(); i--; ) {
	if (aux->table[i].key) {
	  OZ_collectHeapTerm(aux->table[i].key,aux->table[i].key);
	}
      }
    }
    OZ_collectHeapTerm(aux->list,aux->list);
    aux = aux->next;
  }
}

void ArityTable::gc() {
  for (int i = size; i--; ) {
    if (table[i] != NULL)
      (table[i])->gc();
  }
}

void PrTabEntry::gcPrTabEntries()
{
  PrTabEntry *aux = allPrTabEntries;
  while(aux) {
    OZ_collectHeapTerm(aux->info,aux->info);    
    OZ_collectHeapTerm(aux->file,aux->file);    
    OZ_collectHeapTerm(aux->printname,aux->printname);    
    aux = aux->next;
  }
}

void AbstractionEntry::freeUnusedEntries()
{
  AbstractionEntry *aux = allEntries;
  allEntries = NULL;
  while (aux) {
    AbstractionEntry *aux1 = aux->next;
    if (aux->collected || 
	aux->abstr==NULL) { // RS: HACK alert: compiler might have reference to
                            // abstraction entries: how to detect them??
      aux->collected = NO;
      aux->next  = allEntries;
      allEntries = aux;
    } else {
      delete aux;
    }
    aux = aux1;
  }
}


void AbstractionEntry::gcAbstractionEntry()
{
  if (this==NULL || collected) return;

  collected = OK;
  abstr = gcAbstraction(abstr);
}

void ThreadsPool::doGC () {
  // Assert(_currentThread==NULL); // TMUELLER
  threadBodyFreeList = (RunnableThreadBody *) NULL;

  hiQueue.gc();
  midQueue.gc();
  lowQueue.gc();
}

#ifdef LINKED_QUEUES
void ThreadQueue::gc() {
  ThreadQueueIterator iter(this);
  Thread*ptr;
  head=tail=0;
  head_index=tail_index=size=0;
  while ((ptr=iter.getNext())) enqueue(ptr->gcThread());
}
#else
void ThreadQueue::gc() {
  int newsize = suggestNewSize();
  Thread ** newqueue = 
    (Thread **) heapMalloc ((size_t) (sizeof(Thread *) * newsize));
  
  int asize   = size;
  int ahead   = head;
  int newhead = 0;
  
  while (asize--) {
    newqueue[newhead++] = queue[ahead]->gcThread();
    ahead = ahead + 1;
    if (ahead==maxsize)
      ahead = 0;
  }

   maxsize = newsize;
   queue   = newqueue;
   head    = 0;
   tail    = size - 1;
}
#endif /* !LINKED_QUEUES */

#ifdef LINKED_QUEUES
LocalPropagatorQueue * LocalPropagatorQueue::gc() {
  if (this==0) return 0;
  // when a space is being cloned, it must be stable
  // which means that it local propagation must be empty
  // and this is catched by the conditional above.
  // if we fall through, then we must be in GC
  Assert(isInGc);
  Assert(!isEmpty());
  LocalPropagatorQueue * q = new LocalPropagatorQueue();
  q->lpq_thr = lpq_thr->gcThread();
  LocalPropagatorQueueIterator iter(this);
  Propagator*ptr;
  while ((ptr=iter.getNext())) q->enqueue(ptr->gcPropagator());
  // do not return the blocks to the free list since they
  // are in old space, just forget everything
  zeroAll();
  return q;
}
#else
LocalPropagatorQueue * LocalPropagatorQueue::gc() {
  if (!this) 
    return NULL;
  
  Assert(isInGc);
  Assert(!isEmpty());

  LocalPropagatorQueue * new_lpq = new LocalPropagatorQueue (suggestNewSize());

  // gc local thread queue thread
  new_lpq->lpq_thr = lpq_thr->gcThread();

  // gc and copy entries
  for ( ; !isEmpty(); new_lpq->enqueue(dequeue()->gcPropagator()));

  return new_lpq;
}
#endif /* !LINKED_QUEUES */

// Note: the order of the list must be maintained
inline
OrderedSuspList * OrderedSuspList::gc() {
  OrderedSuspList * ret = NULL, * help = this, ** p = & ret;

  while (help != NULL) {

    Propagator * aux = help->_p->gcPropagator();

    if (aux) {
      *p = new OrderedSuspList(aux, NULL);
      p = & (*p)->_n;
    }

    help = help->_n;
  }

  return (ret);
}


//*********************************************************************
//                           NODEs
//*********************************************************************

// failure interface for local tertiarys
inline void maybeGCForFailure(Tertiary *t){
  if(t->getInfo()!=NULL) (*gcEntityInfo)(t);}

inline
void ConstTermWithHome::gcConstTermWithHome()
{
  if (hasGName()) {
    dogcGName(getGName1());
  } else {
    setBoard(getBoardInternal()->gcBoard());
  }
}

void ConstTerm::gcConstRecurse()
{
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;

      switch(o->getTertType()) {
      case Te_Local:   
	o->setBoard(GETBOARD(o)->gcBoard()); 
	maybeGCForFailure(o);
	break;
      case Te_Proxy:   // PER-LOOK is this possible?
	(*gcProxyRecurse)(o); 
	(*gcEntityInfo)(o);
	break;
      case Te_Manager: 
	(*gcManagerRecurse)(o); 
	(*gcEntityInfo)(o);
	break;
      default:         Assert(0);
      }

      o->setClass(o->getClass()->gcClass());
      o->setFreeRecord(o->getFreeRecord()->gcSRecord());
      RecOrCell state = o->getState();
      if (stateIsCell(state)) {
	if (o->isLocal() && getCell(state)->isLocal()) {
	  TaggedRef newstate = ((CellLocal*) getCell(state))->getValue();
	  o->setState(tagged2SRecord(oz_deref(newstate))->gcSRecord());
	} else {
	  o->setState((Tertiary*) getCell(state)->gcConstTerm());
	}
      } else {
	o->setState(getRecord(state)->gcSRecord());
      }
      o->lock = (OzLock*) o->getLock()->gcConstTerm();
      break;
    }
    
  case Co_Class:
    {
      ObjectClass *cl = (ObjectClass *) this;
      cl->gcConstTermWithHome();
      cl->fastMethods    = (OzDictionary*) cl->fastMethods->gcConstTerm();
      cl->defaultMethods = (OzDictionary*) cl->defaultMethods->gcConstTerm();
      cl->features       = cl->features->gcSRecord();
      cl->unfreeFeatures = cl->unfreeFeatures->gcSRecord();
      break;
    }

  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
      a->gcConstTermWithHome();
      if (isInGc)
	gcCode(a->getPred()->getCodeBlock());
      break;
    }
    
  case Co_Cell:
    {
      Tertiary *t=(Tertiary*)this;
      if (t->isLocal()) {
	CellLocal *cl=(CellLocal*)t;
	cl->setBoard(GETBOARD(cl)->gcBoard()); 
	OZ_collectHeapTerm(cl->val,cl->val);
	maybeGCForFailure(t);
      } else {
	(*gcDistCellRecurse)(t);
      }
      break;
    }
    
  case Co_Port:
    {
      Port *p = (Port*) this;
      if (p->isLocal()) {
	p->setBoard(GETBOARD(p)->gcBoard()); /* ATTENTION */
	PortWithStream *pws = (PortWithStream *) this;
	OZ_collectHeapTerm(pws->strm,pws->strm);
	maybeGCForFailure(p);
	break;
      } else {
	(*gcDistPortRecurse)(p);
      }
      break;
    }
  case Co_Space:
    {
      Space *s = (Space *) this;
      Assert(s->getInfo()==NULL);
      if (!s->isProxy()) {
	if (s->solve != (Board *) 1) {
	  s->solve = s->solve->gcBoard();
	}
	if (s->isLocal()) {
	  s->setBoard(GETBOARD(s)->gcBoard());
	}
      }
      break;
    }
    
  case Co_Chunk:
    {
      SChunk *c = (SChunk *) this;
      OZ_collectHeapTerm(c->value,c->value);
      c->gcConstTermWithHome();
      break;
    }
    
  case Co_Array:
    {
      OzArray *a = (OzArray*) this;
      a->gcConstTermWithHome();
      int aw = a->getWidth();
      if (aw > 0) {
	TaggedRef *newargs = (TaggedRef*) heapMalloc(sizeof(TaggedRef) * aw);
	OZ_collectHeapBlock(a->getArgs(), newargs, aw);
	a->args=newargs;
      }
      break;
    }
    
  case Co_Dictionary:
    {
      OzDictionary *dict = (OzDictionary *) this;
      dict->gcConstTermWithHome();
      dict->table = dict->table->gc();
      break;
    }

  case Co_Lock:
    {
      Tertiary *t=(Tertiary*)this;
      if (t->isLocal()) {
	LockLocal *ll = (LockLocal *) this;
	ll->setBoard(GETBOARD(ll)->gcBoard());  /* maybe getBoardInternal() */
	gcPendThreadEmul(&(ll->pending));
	ll->setLocker(ll->getLocker()->gcThread());
	maybeGCForFailure(t);
	break;
      } else {
	(*gcDistLockRecurse)(t);
      }
      break;
    }
    
  default:
    Assert(0);
  }
}

#define CheckLocal(CONST)			     \
{						     \
   Board *bb=GETBOARD(CONST);			     \
   if (!bb->gcIsAlive()) return NULL;		     \
   if (!isInGc && bb->isMarkedGlobal()) return this; \
}


ConstTerm *ConstTerm::gcConstTerm() {

  if (this == NULL) {
    return NULL;
  }

  if (gcIsMarked())
    return gcGetFwd();

  GName *gn = NULL;

  ConstTerm * ret;

  switch (getType()) {
  case Co_BigInt:
    if (isInGc) {
      ret = ((BigInt *) this)->gc();
      storeFwdField(this, ret);
      return ret;
    } else {
      return this;
    }
  case Co_Abstraction: 
    {
      Abstraction *a = (Abstraction *) this;
      CheckLocal(a);

      Abstraction *newA = Abstraction::newAbstraction(a->getPred(),
						      a->getBoardInternal());
      gcStack.push(newA,PTR_CONSTTERM);
      storeFwdField(this, newA);
      gn = a->getGName1();
      if (gn) {
	newA->setGName(gn);
	dogcGName(gn);
      }
      OZ_collectHeapBlock(a->getGRef(),newA->getGRef(),
			  a->getPred()->getGSize());

      return newA;
    }

  case Co_Object: 
    {
      Object *o = (Object *) this;
      CheckLocal(o);
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(Object));
      gn = o->getGName1();
      break;
    }
  case Co_Class: 
    {
      ObjectClass *cl = (ObjectClass *) this;
      CheckLocal(cl);
      gn = cl->getGName1();
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(ObjectClass));
      break;
    }
  case Co_Cell:
    {
      switch(((Tertiary *)this)->getTertType()){
      case Te_Local:
	CheckLocal((CellLocal*)this);
      case Te_Proxy:
      case Te_Manager:
	ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellManagerEmul));
	break;
      case Te_Frame:{
	ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellFrameEmul));
	break;
      }
      default:{
	Assert(0);
	break;}}
      break;
    }

  case Co_Port:  
    {
      if(((Tertiary *)this)->getTertType()==Te_Proxy) {
	ret = (ConstTerm *) gcReallocStatic(this, SIZEOFPORTPROXY);
	break;}
      if(((Tertiary *)this)->getTertType()==Te_Local) {
	CheckLocal((PortLocal *) this);}
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(PortLocal));
      break;
    }
  case Co_Space:
    {
      Space *sp = (Space *) this;
      CheckLocal(sp);

      Board *bb=GETBOARD(sp);
#ifdef DEEP_GARBAGE
      if (!isInGc && !bb->isInTree()) {
	return 0;
      }
#endif
      Assert(isInGc || bb->isInTree()); 

      ret = (ConstTerm *) gcReallocStatic(this,sizeof(Space));
      break;
    }

  case Co_Chunk:
    {
      SChunk *sc = (SChunk *) this;
      CheckLocal(sc);
      gn = sc->getGName1();
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(SChunk));
      break;
    }

  case Co_Array:
    CheckLocal((OzArray *) this);
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(OzArray));
    break;

  case Co_Dictionary:
    CheckLocal((OzDictionary *) this);
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(OzDictionary));
    break;

  case Co_Lock:
    {
      switch(((Tertiary *)this)->getTertType()) {
      case Te_Local:
	CheckLocal((LockLocal*)this);
      case Te_Proxy:
      case Te_Manager:
	ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockLocal));
	break;
      case Te_Frame:{
	ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockLocal));
	break;
      }
      default:{
	Assert(0);
	break;}}
      break;
    }

  /* builtins are allocate dusing malloc */
  case Co_Builtin:
    return this;

  case Co_Foreign_Pointer:
    return ((ForeignPointer*)this)->gc();
  
  case Co_Resource:{
    ret = (*gcDistResource)(this);
    storeFwdField(this, ret);
    return ret;}
  
  default:
    Assert(0);
    return 0;
  }

  gcStack.push(ret,PTR_CONSTTERM);
  storeFwdField(this, ret);
  dogcGName(gn);
  return ret;
}

/* the purpose of this procedure is to provide an additional entry
   into gc so to be able to distinguish between owned perdio-objects that
   are locally accssible to those that are not - currently this is needed
   only for frames (cells and locks).
   The distinction is that in this procedure the BORROW ENTRY is not marked
   but in gcConstTerm it is marked.
   Note- all other Tertiarys are marked in gcConstRecurse
*/


inline
Bool Actor::gcIsMarked(void) {
  return (Bool) gcField;
}
 
inline
void Actor::gcMark(Actor * fwd) {
  if (!isInGc)
    cpTrail.save((int32 *) &gcField);
  gcField = fwd;
}

inline
Actor * Actor::gcGetFwd(void) {
  Assert(gcIsMarked());
  return gcField;
}

inline
Actor * Actor::gcActor() {
  if (!this)
    return (Actor *) 0;

  Assert(board->derefBoard()->gcIsAlive());

  if (gcIsMarked())
    return gcGetFwd();
  
  Actor * ret = (Actor *) oz_hrealloc(this, sizeof(SolveActor));

  gcStack.push(ret, PTR_ACTOR);
  
  gcMark(ret);

  return ret;
}

void TaskStack::gc(TaskStack *newstack) {

  TaskStack *oldstack = this;

  newstack->allocate(suggestNewSize());
  Frame *oldtop = oldstack->getTop();
  int offset    = oldstack->getUsed();
  Frame *newtop = newstack->array + offset;

  while (1) {
    GetFrame(oldtop,PC,Y,CAP);

    if (isInGc)
      gcCode(PC);

    if (PC == C_EMPTY_STACK) {
      *(--newtop) = PC;
      *(--newtop) = Y;
      *(--newtop) = CAP;
      Assert(newstack->array == newtop);
      newstack->setTop(newstack->array+offset);
      return;
    } else if (PC == C_CATCH_Ptr) {
    } else if (PC == C_XCONT_Ptr) {
      // mm2: opt: only the top task can/should be xcont!!
      ProgramCounter pc   = (ProgramCounter) *(oldtop-1);
      if (isInGc)
	gcCode(pc);
      (void) CodeArea::livenessX(pc,Y,getRefsArraySize(Y));
      Y = gcRefsArray(Y); // X
    } else if (PC == C_LOCK_Ptr) {
      Y = (RefsArray) ((OzLock *) Y)->gcConstTerm();
    } else if (PC == C_LPQ_Ptr) {
      Y = (RefsArray) ((Board *) Y)->gcBoard();
    } else if (PC == C_SET_SELF_Ptr) {
      Y = (RefsArray) ((Object*)Y)->gcConstTerm();
    } else if (PC == C_SET_ABSTR_Ptr) {
      ;
    } else if (PC == C_DEBUG_CONT_Ptr) {
      Y = (RefsArray) ((OzDebug *) Y)->gcOzDebug();
    } else if (PC == C_CALL_CONT_Ptr) {
      /* tt might be a variable, so use this ugly construction */
      *(newtop-2) = Y; /* UGLYYYYYYYYYYYY !!!!!!!! */
      TaggedRef *tt = (TaggedRef*) (newtop-2);
      OZ_collectHeapTerm(*tt,*tt);
      Y = (RefsArray) ToPointer(*tt);
      CAP = (Abstraction *)gcRefsArray((RefsArray)CAP);
    } else if (PC == C_CFUNC_CONT_Ptr) {
      CAP = (Abstraction *)gcRefsArray((RefsArray)CAP);
    } else { // usual continuation
      Y = gcRefsArray(Y);
      CAP = gcAbstraction(CAP);
    }

    *(--newtop) = PC;
    *(--newtop) = Y;
    *(--newtop) = CAP;
  } // while not task stack is empty
}

/*
 * notification board == home board of thread
 * Although this may be discarded/failed, the solve actor must be announced.
 * Therefore this procedures searches for another living board.
 */
Board* Board::gcGetNotificationBoard() {
  if (this == 0) 
    return 0; // no notification board

  Board *bb = this->derefBoard();

  Board *nb = bb;
  
  while (1) {
  
    if (bb->gcIsMarked() || bb->isRoot())  
      return nb;

    Assert(!bb->isCommitted());

    Actor * aa = bb->getActor();
  
    if (bb->isFailed() || aa->isCommitted()) {
      /*
       * notification board must be changed
       */
      bb = GETBOARD(aa);
      nb = bb;   // probably not dead;
      continue;
    }

    if (aa->gcIsMarked())
      return nb;

    bb = GETBOARD(aa);
  }
}

/****************************************************************************
 * Board collection 
 ****************************************************************************/

/*
 * gcIsAlive(bb):
 *   bb is marked collected, not failed and all parents are alive
 *
 */

Bool Board::gcIsAlive() {
  Board *bb = this;

  while (1) {
    bb = bb->derefBoard();

    if (bb->isFailed()) 
      return NO;

    if (bb->isRoot() || bb->gcIsMarked())
      return OK;
    
    Actor *aa = bb->getActor();

    if (aa->isCommitted()) 
      return (NO);

    if (aa->gcIsMarked())
      return OK;

    bb = aa->getBoardInternal();
  }
}

inline
void Board::gcRecurse() {
  Assert(!isCommitted() && !isFailed());
  u.actor = u.actor ? u.actor->gcActor() : 0;

  localPropagatorQueue = localPropagatorQueue->gc();

  script.Script::gc();
}

Distributor * BaseDistributor::gc(void) {
  BaseDistributor * t = 
    (BaseDistributor *) oz_hrealloc(this,sizeof(BaseDistributor));

  OZ_collectHeapTerm(var, t->var);

  return (Distributor *) t;
}

inline
DistBag * DistBag::gc(void) {
  DistBag *  copy = (DistBag *) 0;
  DistBag ** cur  = &copy;
  DistBag *  old  = this;
  
  while (old) {
    if (old->getDist()->isAlive()) {
      DistBag * one = new DistBag(old->getDist()->gc());
      *cur = one;
      cur  = &(one->next);
    }
    old = old->next;
  }
 
  return copy;
}

inline
void SolveActor::gcRecurse () {
  if (isInGc || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
    Assert(board);
  }
  solveBoard = solveBoard->gcBoard();
  Assert(solveBoard);

  OZ_collectHeapTerm(solveVar,solveVar);
  
  OZ_collectHeapTerm(result,result);
  suspList         = suspList->gc();
  bag              = bag->gc();
  nonMonoSuspList  = nonMonoSuspList->gc();
#ifdef CS_PROFILE
  if((copy_size>0) && copy_start && isInGc) {
    free(copy_start);
  }
  orig_start = (int32 *) NULL;
  copy_start = (int32 *) NULL;
  copy_size  = 0;
#endif
}

inline
void Actor::gcRecurse() {
  ((SolveActor *) this)->gcRecurse();
}

//*****************************************************************************
//                           collectGarbage
//*****************************************************************************

inline
void SRecord::gcRecurse() {
  SRecord * to = (SRecord *) GCUNMARK(label);

  OZ_collectHeapTerm(to->label,to->label);

  OZ_collectHeapBlock(getRef(), to->getRef(), getWidth());

}


inline
void LTuple::gcRecurse() {
  LTuple * frm = this;
  LTuple * to  = (LTuple *) GCUNMARK(frm->args[0]);

  // Restore original!
  frm->args[0] = to->args[0];

  TaggedRef aux = oz_deref(to->args[0]);

  // Case : L=L|_
  if (!oz_isLTuple(aux) || tagged2LTuple(aux) != this) {
    OZ_collectHeapTerm(frm->args[0], to->args[0]);
    
    storeFwd((int32 *)frm->args, to->args);
  }

  while (1) {
    // Store forward, order is important, since collection might already 
    // have done a storeFwd, which means that this one will be overwritten
    TaggedRef t = oz_deref(frm->args[1]);

    if (!oz_isCons(t)) {
      OZ_collectHeapTerm(frm->args[1], to->args[1]);
      return;
    }

    frm = tagged2LTuple(t);

    if (GCISMARKED(frm->args[0])) {
      to->args[1] = makeTaggedLTuple((LTuple *) GCUNMARK(frm->args[0]));
      return;
    }

    LTuple * next = (LTuple *) freeListMalloc(sizeof(LTuple));

    to->args[1] = makeTaggedLTuple(next);
    to = next;
    
    OZ_collectHeapTerm(frm->args[0], to->args[0]);

    storeFwd((int32 *)frm->args, to->args);

  } 
  
  Assert(0);
}


void GcStack::recurse(void) {

  while (!isEmpty()) {
    TaggedRef tp  = (TaggedRef) pop();  
    void * ptr    = tagValueOf(tp);
    TypeOfPtr how = (TypeOfPtr) tagTypeOf(tp);
    
    switch(how) {
      
    case PTR_LTUPLE:    
      ((LTuple *) ptr)->gcRecurse();
      break;

    case PTR_SRECORD:   
      ((SRecord *) ptr)->gcRecurse();          
      break;

    case PTR_BOARD:     
      ((Board *) ptr)->gcRecurse();            
      break;
      
    case PTR_ACTOR:     
      ((Actor *) ptr)->gcRecurse();            
      break;
      
    case PTR_THREAD:    
      ((Thread *) ptr)->gcRecurse();           
      break;
      
    case PTR_PROPAGATOR:    
      ((Propagator *) ptr)->gcRecurse();           
      break;
      
    case PTR_CVAR:    
      ((OzVariable *) ptr)->gcVarRecurse();
      break;

    case PTR_CONSTTERM: 
      ((ConstTerm *) ptr)->gcConstRecurse();   
      break;

    case PTR_EXTENSION: 
      gcExtensionRecurse((OZ_Extension *)ptr);
      break;

    default:
      Assert(NO);
    }
  }
}
  



//*****************************************************************************
//             AM methods to launch gc under certain conditions
//*****************************************************************************


// signal handler
void checkGC() {
  Assert(!am.isCritical());
  if (getUsedMemory() > unsigned(ozconf.heapThreshold) && ozconf.gcFlag) {
    am.setSFlag(StartGC);
  }
}

void AM::doGC() {
  Assert(oz_onToplevel());

  /* do gc */
  gc(ozconf.gcVerbosity);

  /* calc limits for next gc */
  int used   = getUsedMemory();
  int wanted = ((ozconf.heapFree == 100) 
		? ozconf.heapMaxSize 
		: max(((long) used) * (100 / (100 - ozconf.heapFree)),
		      ozconf.heapMinSize));

  /* Try to align as much as possible to end of blocksize */
  int block_size = HEAPBLOCKSIZE / KB;
  int block_dist = wanted % block_size; 

  if (block_dist > 0)
    block_dist = block_size - block_dist;

  wanted += min(block_dist,
		(((long) wanted) * ozconf.heapTolerance / 100));

  if (wanted > ozconf.heapMaxSize) {
    if (ozconf.runningUnderEmacs) {
      OZ_warning("\n*** Heap Max Size exceeded: Increasing from %d to %d.\n",
		 ozconf.heapMaxSize,wanted);
      prefixError();
      fflush(stdout);
    }
    ozconf.heapMaxSize = wanted;
  }
  
  ozconf.heapThreshold = wanted;
  
  unsetSFlag(StartGC);
}

OzDebug *OzDebug::gcOzDebug() {
  OzDebug *ret = (OzDebug*) gcReallocStatic(this,sizeof(OzDebug));
  
  ret->Y = gcRefsArray(ret->Y);
  ret->CAP = gcAbstraction(ret->CAP);
  OZ_collectHeapTerm(ret->data,ret->data);
  ret->arguments = gcRefsArray(ret->arguments);

  return ret;
}

// special purpose to gc borrowtable entry which is a variable
TaggedRef gcTagged1(TaggedRef in) {
  TaggedRef x=oz_deref(in);
  Assert(GCISMARKED(x));
  return makeTaggedRef((TaggedRef*)GCUNMARK(x));
}



//*****************************************************************************
//       GC Code Area
//*****************************************************************************

static int codeGCgeneration = CODE_GC_CYLES;

void CodeArea::gcCodeBlock()
{
  if (referenced == NO) {
    referenced = OK;
    gclist->collectGClist();
  }
}

void gcCode(CodeArea *block) {
  Assert(isInGc);
  if (codeGCgeneration!=0)
    return;

  block->gcCodeBlock();
}


void gcCode(ProgramCounter PC) {
  gcCode(CodeArea::findBlock(PC));
}

void CodeGCList::collectGClist()
{
  CodeGCList *aux = this;
  while(aux) {
    for (int i=aux->nextFree; i--; ) {
      switch(aux->block[i].tag) {
      case C_TAGGED:
	OZ_collectHeapTerm(*(TaggedRef*)aux->block[i].pc,
			   *(TaggedRef*)aux->block[i].pc);
	break;
      case C_ABSTRENTRY:
	((AbstractionEntry*)*(aux->block[i].pc))->gcAbstractionEntry();
	break;
      case C_INLINECACHE:
	((InlineCache*)aux->block[i].pc)->invalidate();
	break;
      case C_FREE:
	break;
      default:
	Assert(0);
      }
    }
    aux = aux->next;
  }
}

void CodeArea::gcCodeAreaStart()
{
// #define CODEGCOFF
#ifndef CODEGCOFF
  if (ozconf.codeGCcycles == 0) {
    codeGCgeneration = 1;
  } else if (++codeGCgeneration >= ozconf.codeGCcycles) {
    // switch code GC on
    codeGCgeneration = 0;
    return;
  }
#endif

  CodeArea *code = allBlocks;

  while (code) {
    code->gcCodeBlock();
    code = code->nextBlock;
  }
}

void CodeArea::gcCollectCodeBlocks()
{
  CodeArea *code = allBlocks;
  allBlocks = NULL;
  while (code) {
    if (code->referenced == NO) {
      //message("collected(%x): %d\n",code,code->size*sizeof(ByteCode));
      //displayCode(code->getStart(),5);
      CodeArea *aux = code;
      code = code->nextBlock;
      delete aux;
    } else {
      code->referenced = NO;
      CodeArea *aux    = code;
      code             = code->nextBlock;
      aux->nextBlock   = allBlocks;
      allBlocks        = aux;
    }
  }
}




