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

#include "cac.hh"

#include "tagged.hh"
#include "board.hh"
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
#include "debug.hh"
#include "pointer-marks.hh"
#include "dpInterface.hh"
#include "gname.hh"
#include "interFault.hh"
#include "weakdict.hh"

// loeckelt (for big fsets)
#include "mozart_cpi.hh"

// hack alert: usage #pragma interface requires this
#ifdef OUTLINE
#define inline
#endif

/*
 * isCollecting: collection is running
 *
 */

#ifdef DEBUG_CHECK
#ifdef G_COLLECT
Bool isCollecting = NO;
#else
extern Bool isCollecting;
#endif
#endif

#ifdef S_CLONE
#ifdef CS_PROFILE
int32 * cs_copy_start = NULL;
int32 * cs_orig_start = NULL;
int     cs_copy_size  = 0;
#endif
#endif

#ifdef G_COLLECT

/*
 * Forward reference
 */

static void gCollectCode(CodeArea *block);
static void gCollectCode(ProgramCounter PC);

#else

inline
void oz_sCloneTerm(TaggedRef & f, TaggedRef & t) {
  OZ_sCloneBlock(&f, &t, 1);
}

#endif

/*
 * Depending on whether garbage collection or cloning use heap allocation
 *
 */

#ifdef G_COLLECT
#define CAC_MALLOC(sz) heapMalloc((sz))
#else
#define CAC_MALLOC(sz) freeListMalloc((sz))
#endif


/*
 * CHECKSPACE -- check if object is really copied from heap
 *   has as set of macros:
 *    INITCHECKSPACE - save pointer to from-space & print from-space
 *    NOTINTOSPACE   - assert not in to-space
 *    INTOSPACE      - assert in to-space
 * NOTE: this works only for chunk
 */

#if defined(DEBUG_GC) && defined(G_COLLECT)

static MemChunks *fromSpace;

Bool inToSpace(void *p) {
  return (p==NULL || MemChunks::list->inChunkChain(p));
}

Bool notInToSpace(void *p) {
  return (p==NULL || !MemChunks::list->inChunkChain(p));
}

Bool inFromSpace(void *p) {
  return (p==NULL || fromSpace->inChunkChain(p));
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
void * _cacReallocStatic(void * p, size_t sz) {
  // Use for blocks where size is known statically at compile time
  DebugCheck(sz%sizeof(int) != 0,
             OZ_error("_cacReallocStatic: can only handle word sized blocks"););

  if (sz > 24) {
    return memcpy(CAC_MALLOC(sz), p, sz);
  } else {
    register int32 * frm = (int32 *) p;
    register int32 * to  = (int32 *) CAC_MALLOC(sz);

    switch(sz) {
    case 24:
      to[0]=frm[0];
      to[1]=frm[1];
      to[2]=frm[2];
      to[3]=frm[3];
      to[4]=frm[4];
      to[5]=frm[5];
      break;
    case 20:
      to[0]=frm[0];
      to[1]=frm[1];
      to[2]=frm[2];
      to[3]=frm[3];
      to[4]=frm[4];
      break;
    case 16:
      to[0]=frm[0];
      to[1]=frm[1];
      to[2]=frm[2];
      to[3]=frm[3];
      break;
    case 12:
      to[0]=frm[0];
      to[1]=frm[1];
      to[2]=frm[2];
      break;
    case 8:
      to[0]=frm[0];
      to[1]=frm[1];
      break;
    case 4:
      to[0]=frm[0];
      break;
    default:
      Assert(0);
    }

    return to;
  }
}


#ifdef S_CLONE

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

  void save(int * p, Bool check) {
    // Save content and address
    ensureFree(2); // ALways check for now ;-(
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

#endif

/*
 * GCMARK
 *
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
 */
inline
void _cacStoreFwdNoMark(int32* fromPtr, void *newValue, Bool check) {
#ifdef S_CLONE
  cpTrail.save(fromPtr,check);
#else
  GCDBG_INFROMSPACE(fromPtr);
#endif

  *fromPtr = ToInt32(newValue);
}

inline
void _cacStoreFwdMark(int32* fromPtr, void *newValue, Bool check) {
#ifdef S_CLONE
  cpTrail.save(fromPtr,check);
#else
  GCDBG_INFROMSPACE(fromPtr);
#endif

  *fromPtr = GCMARK(newValue);
}

#define storeFwdField(d,t,c) \
  _cacStoreFwdNoMark((int32*) d->cacGetMarkField(), t, c); d->cacMark(t);

#ifdef G_COLLECT

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

  ExtRefNode *gCollect()
  {
    ExtRefNode *aux = this;
    ExtRefNode *ret = NULL;
    while(aux) {
      if (aux->elem) {
        ret = new ExtRefNode(aux->elem,ret);
        oz_gCollectTerm(*ret->elem,*ret->elem);
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

#endif

#ifdef G_COLLECT

VarFix varFix;
CacStack cacStack;

#endif


/****************************************************************************
 * Collect all types of terms
 ****************************************************************************/

// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc tag set --> has already been copied

#define RAGCTag (1<<31)

inline
Bool _cacRefsArrayIsMarked(RefsArray r) {
  return (r[-1]&RAGCTag);
}

inline
void _cacRefsArrayMark(RefsArray r, void *ptr) {
  _cacStoreFwdNoMark((int32*)&r[-1],ToPointer(ToInt32(ptr)|RAGCTag),OK);
}

inline
RefsArray _cacRefsArrayUnmark(RefsArray r) {
  return (RefsArray) ToPointer(r[-1]&(~(RAGCTag)|mallocBase));
}

inline
RefsArray _cacRefsArray(RefsArray r) {
  if (r == NULL)
    return r;

  GCDBG_NOTINTOSPACE(r);

  if (_cacRefsArrayIsMarked(r)) {
    return _cacRefsArrayUnmark(r);
  }

  Assert(!isFreedRefsArray(r));

  int sz = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(sz,NO);

  _cacRefsArrayMark(r,aux);

  OZ_cacBlock(r, aux, sz);

  return aux;
}

inline
Abstraction * _cacAbstraction(Abstraction * a) {
  return a ? ((Abstraction *) a->_cacConstTerm()) : a;
}

#ifdef G_COLLECT

#define NEEDSCOPYING(bb) (OK)

#else

#ifdef DEBUG_CHECK

inline
int NEEDSCOPYING(Board * bb) {
 return !bb->hasMarkOne();
}

#else

#define NEEDSCOPYING(bb) (!(bb)->hasMarkOne())

#endif

#endif


inline
void Board::_cacMark(Board * fwd) {
  Assert(!cacIsMarked());
#ifdef S_CLONE
  cpTrail.save((int32 *) &parentAndFlags, NO);
#endif
  parentAndFlags.set((void *) fwd, BoTag_MarkTwo);
}

inline
Board * Board::_cacBoard() {
  GCDBG_INFROMSPACE(this);

  // Do not clone a space above or collect a space above root ;-(
  Assert(this && !hasMarkOne());

  Board * bb = derefBoard();

  Assert(bb);

  if (bb->cacIsMarked())
    return bb->cacGetFwd();

  Assert(bb->cacIsAlive());

  Board *ret = (Board *) _cacReallocStatic(bb, sizeof(Board));

  cacStack.push(ret,PTR_BOARD);

  bb->_cacMark(ret);

  return ret;
}

#ifdef G_COLLECT

void dogcGName(GName *gn) {
  if (gn)
    gCollectGName(gn);
}

#else

#define dogcGName(no)

#endif


/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

inline
Name *Name::_cacName() {
  CHECKCOLLECTED(homeOrGName, Name *);
#ifdef G_COLLECT
  GName * gn = NULL;

  if (hasGName()) {
    gn = getGName1();
  }
#endif

#ifdef G_COLLECT
  if (isOnHeap()) {
#else
  if (!getBoardInternal()->hasMarkOne()) {
#endif

    Name *aux = (Name*) _cacReallocStatic(this,sizeof(Name));

    _cacStoreFwdMark(&homeOrGName, aux, NO);

#ifdef G_COLLECT
    if (gn) {
      gCollectGName(gn);
      return aux;
    }
#endif
    aux->homeOrGName =
      ToInt32(((Board*)ToPointer(aux->homeOrGName))->_cacBoard());

    return aux;

  } else {
#ifdef G_COLLECT
    if (gn)
      gCollectGName(gn);
#endif
    return this;
  }
}

inline
Literal * Literal::_cac(void) {
  if (isAtom())
    return this;

  return ((Name*) this)->_cacName();
}

inline
Object * Object::_cacObjectInline(void) {
  return this ? ((Object *) _cacConstTerm()) : this;
}

Object * Object::_cacObject(void) {
  return _cacObjectInline();
}


inline
void OzVariable::_cacMark(TaggedRef * fwd) {
  Assert(!cacIsMarked());
#ifdef S_CLONE
  cpTrail.save((int32 *) &suspList, NO);
#endif
  suspList = (SuspList *) MarkPointer(fwd,1);
}

inline
void OzFDVariable::_cac(Board * bb) {
  ((OZ_FiniteDomainImpl *) &finiteDomain)->copyExtension();

  cacLocalSuspList(bb, &(fdSuspList[0]), fd_prop_any);
}

inline
void OzFSVariable::_cac(Board * bb) {

#ifdef BIGFSET
  _fset.copyExtension();
#endif

  cacLocalSuspList(bb, &(fsSuspList[0]), fs_prop_any);
}

inline
void OzCtVariable::_cac(Board * bb) {
  // suspension lists
  int noOfSuspLists = getNoOfSuspLists();

  // copy
  SuspList ** new_susp_lists = (SuspList **)
    heapMalloc(sizeof(SuspList *) * noOfSuspLists);
  for (int i = noOfSuspLists; i--; )
    new_susp_lists[i] = _susp_lists[0];
  _susp_lists = new_susp_lists;
  // collect
  cacLocalSuspList(bb, _susp_lists, noOfSuspLists);

}

inline
void OzCtVariable::_cacRecurse(void)
{
  // constraint (must go in `Recurse' since it may contain recursion
  _constraint = _constraint->copy();
}

inline
OzVariable * OzVariable::_cacVarInline(void) {
  GCDBG_INFROMSPACE(this);

  Assert(!cacIsMarked());
  Assert(!isTrailed());

  Board * bb = getBoardInternal()->_cacBoard();

  OzVariable * to;

  switch (getType()) {
  case OZ_VAR_EXT:
    to = ((ExtVar *) this)->_cacV();
    cacStack.push(to, PTR_CVAR);
    break;
  case OZ_VAR_SIMPLE:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(SimpleVar));
    break;
  case OZ_VAR_FUTURE:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(Future));
    cacStack.push(to, PTR_CVAR);
    break;
  case OZ_VAR_BOOL:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(OzBoolVariable));
    break;
  case OZ_VAR_OF:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(OzOFVariable));
    cacStack.push(to, PTR_CVAR);
    break;
  case OZ_VAR_FD:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(OzFDVariable));
    ((OzFDVariable *) to)->_cac(bb);
    break;
  case OZ_VAR_FS:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(OzFSVariable));
    ((OzFSVariable *) to)->_cac(bb);
    break;
  case OZ_VAR_CT:
    to = (OzVariable *) _cacReallocStatic(this,sizeof(OzCtVariable));
    ((OzCtVariable*) to)->_cac(bb);
    cacStack.push(to, PTR_CVAR);
    break;
  }

  to->setHome(bb);
  cacSuspList(&(to->suspList),NO);

  return to;

}

OzVariable * OzVariable::_cacVar(void) {
  /*
   * This routine MUST go, it breaks a lot of invariants!
   */
  return _cacVarInline();
}

inline
DynamicTable * DynamicTable::_cac(void) {
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
      if (ft[i].value) {
        OZ_cacBlock(&(ft[i].ident), &(tt[i].ident), 2);
      } else {
        oz_cacTerm(ft[i].ident, tt[i].ident);
        tt[i].value = makeTaggedNULL();
      }
    } else {
      tt[i].ident = makeTaggedNULL();
      tt[i].value = makeTaggedNULL();
    }

  return to;
}

inline
void OzOFVariable::_cacRecurse(void) {
  oz_cacTerm(label,label);
  // Update the pointer in the copied block:
  dynamictable=dynamictable->_cac();
}

inline
void Future::_cacRecurse(void) {
  if (function)
    oz_cacTerm(function,function);
}

void OzVariable::_cacVarRecurse(void) {

  switch (getType()) {
  case OZ_VAR_SIMPLE:  Assert(0); break;
  case OZ_VAR_FUTURE:  ((Future *)this)->_cacRecurse(); break;
  case OZ_VAR_BOOL:    Assert(0); break;
  case OZ_VAR_FD:      Assert(0); break;
  case OZ_VAR_OF:      ((OzOFVariable*)this)->_cacRecurse(); break;
  case OZ_VAR_FS:      Assert(0); break;
  case OZ_VAR_CT:      ((OzCtVariable*)this)->_cacRecurse(); break;
  case OZ_VAR_EXT:     ((ExtVar *)this)->_cacRecurseV(); break;
  default:
    Assert(0);
  }

}


#ifdef G_COLLECT

inline
Float *Float::gCollect(void) {
  return newFloat(value);
}


inline
FSetValue * FSetValue::gCollect(void) {

#ifdef BIGFSET
  FSetValue *retval = (FSetValue *) oz_hrealloc(this, sizeof(FSetValue));
  retval->_IN.copyExtension();
  return retval;
#else
  return (FSetValue *) oz_hrealloc(this, sizeof(FSetValue));
#endif
}

inline
BigInt * BigInt::gCollect(void) {
  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  return ret;
}

#endif


/*
 *  Thread items methods;
 *
 */

/* collect LTuple, SRecord */

inline
LTuple * LTuple::_cac(void) {
  // Does basically nothing, the real stuff is in Recurse

  GCDBG_INFROMSPACE(this);

  if (GCISMARKED(args[0]))
    return (LTuple *) GCUNMARK(args[0]);

  LTuple * to = (LTuple *) CAC_MALLOC(sizeof(LTuple));

  // Save the content
  to->args[0] = args[0];

  // Do not store foreward! Recurse takes care of this!
  args[0] = GCMARK(to->args);

  cacStack.push(this, PTR_LTUPLE);

  return to;
}

inline
SRecord *SRecord::_cacSRecord() {
  Assert(this);

  CHECKCOLLECTED(label, SRecord *);

  int len = (getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord);

  SRecord *ret = (SRecord*) CAC_MALLOC(len);

  ret->label       = label;
  ret->recordArity = recordArity;

  _cacStoreFwdMark((int32*)&label, ret, NO);

  cacStack.push(this, PTR_SRECORD);

  return ret;
}

#ifdef G_COLLECT

OZ_Propagator * OZ_Propagator::cac(void) {
  OZ_Propagator * to = (OZ_Propagator *) oz_hrealloc(this, sizeOf());

  return to;
}

#endif

// ===================================================================
// Extension

inline
TaggedRef _cacExtension(TaggedRef term) {
  OZ_Extension *ex = oz_tagged2Extension(term);

  Assert(ex);

  // hack alert: write forward into vtable!
  if ((*(int32*)ex)&1) {
    return oz_makeTaggedExtension((OZ_Extension *)ToPointer((*(int32*)ex)&~1));
  }

  Board *bb=(Board*)(ex->__getSpaceInternal());

  if (bb) {
    Assert(bb->cacIsAlive());
    if (!NEEDSCOPYING(bb))
      return term;
  }

  OZ_Extension *ret = ex->_cacV();

  if (bb)
    ret->__setSpaceInternal(bb->_cacBoard());

  cacStack.push(ret,PTR_EXTENSION);

  int32 *fromPtr = (int32*)ex;

#ifdef S_CLONE
  cpTrail.save(fromPtr,NO);
#endif

  *fromPtr = ToInt32(ret)|1;

  return oz_makeTaggedExtension(ret);
}

// ===================================================================
// Weak Dictionaries

#ifdef G_COLLECT

class WeakStack : public Stack {
public:
  WeakStack() : Stack(64,Stack_WithMalloc) {}
  ~WeakStack() {}
  void push(OZ_Term fut,OZ_Term val) {
    Stack::push((StackEntry)fut);
    Stack::push((StackEntry)val);
  }
  void pop(OZ_Term& fut,OZ_Term& val) {
    val = (OZ_Term) Stack::pop();
    fut = (OZ_Term) Stack::pop();
  }
  void recurse(void);
};

static WeakStack weakStack;

void WeakStack::recurse(void)
{
  OZ_Term fut,val;
  while (!isEmpty()) {
    pop(fut,val);
    DEREF(fut,ptr,_);
    oz_bindFuture(ptr,val);
  }
}

// isNowMarked(t) returns true iff
//      t is a marked name, extension, const, var
// the logic is adapted from gcTagged(TaggedRef&,TaggedRef&)
// and simplified according to a suggestion by Christian.
inline int isNowMarked(OZ_Term t)
{
 redo:
  switch (tagTypeOf(t)) {
  case REF:
  case REFTAG2:
  case REFTAG3:
  case REFTAG4:
    {
      TaggedRef * ptr;
      do {
        ptr = tagged2Ref(t);
        t   = *ptr;
      } while (oz_isRef(t));
      goto redo;
    }
  case GCTAG     : goto RETURN_YES;
  case SMALLINT  : goto RETURN_NO;
  case FSETVALUE : goto RETURN_NO;
  case LITERAL   :
    {
      Literal * lit = tagged2Literal(t);
      if (lit->isAtom()) return 1;
      else return ((Name*)lit)->cacIsMarked();
    }
  case EXT       : return ((*(int32*)oz_tagged2Extension(t))&1);
  case LTUPLE    : goto RETURN_NO;
  case SRECORD   : goto RETURN_NO;
  case OZFLOAT   : goto RETURN_NO;
  case OZCONST   : return (tagged2Const(t)->cacIsMarked());
  case UNUSED_VAR: goto IMPOSSIBLE;
  case UVAR      : goto IMPOSSIBLE;
  case CVAR      : return tagged2CVar(t)->cacIsMarked();
  default        : Assert(0);
  }
  return 0;
 RETURN_YES: return 1;
 IMPOSSIBLE: Assert(0);
 RETURN_NO : return 0;
}

void WeakDictionary::gCollectRecurseV(void) {
  if (stream)
    oz_gCollectTerm(stream, stream);
}

void WeakDictionary::sCloneRecurseV(void) {
  Assert(0);
}

void WeakDictionary::weakGC()
{
  int numelem = table->numelem;
  // go through the table and finalize each entry whose value is not
  // marked.  also clear these entries.
  OZ_Term newstream = 0;
  OZ_Term list = 0;
  int count = 0;
  dt_index i;
  for (i=table->size; i--; ) {
    TaggedRef t = table->getValue(i);
    if (t!=0 && !isNowMarked(t)) {
      numelem--;
      if (stream) {
        if (!list) newstream=list=oz_newFuture(oz_rootBoard());
        OZ_Term k = table->getKey(i);
        // collect key and value
        oz_cacTerm(t,t);
        oz_cacTerm(k,k);
        list = oz_cons(oz_pair2(k,t),list);
        count++;
      }
      table->clearValue(i);
    }
  }
  // then update the stream
  if (stream && list) {
    weakStack.push(stream,list);
    stream=newstream;
  }
  // finally collect the table
  DynamicTable * frm = table;
  table = DynamicTable::newDynamicTable(numelem);
  for (i=frm->size;i--;) {
    OZ_Term v = frm->getValue(i);
    if (v!=0) {
      OZ_Term k = frm->getKey(i);
      oz_cacTerm(k,k);
      oz_cacTerm(v,v);
      put(k,v);
    }
  }
}


// ===================================================================
// Finalization

extern OZ_Term guardian_list;
extern OZ_Term finalize_list;
extern OZ_Term finalize_handler;

void gCollect_finalize()
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
    case EXT    :
      // same check as Michael's hack in Extension
      if ((*(int32*)oz_tagged2Extension(obj))&1)
        // reachable through live data
        guardian_list = oz_cons(pair,guardian_list);
      else
        // unreachable
        finalize_list = oz_cons(pair,finalize_list);
      break;
    case OZCONST:
      if (tagged2Const(obj)->cacIsMarked())
        // reachable through live data
        guardian_list = oz_cons(pair,guardian_list);
      else
        // unreachable
        finalize_list = oz_cons(pair,finalize_list);
      break;
    default     :
      Assert(0);
    }
  }
  // gc both these list normally.
  // since these lists have been freshly consed in the new half space
  // this simply means to go through both and gc the pairs
  // in the head of each cons
  for (OZ_Term l=guardian_list;!oz_isNil(l);l=oz_tail(l)) {
    LTuple *t = tagged2LTuple(l);
    oz_gCollectTerm(*t->getRefHead(),*t->getRefHead());
  }
  for (OZ_Term l1=finalize_list;!oz_isNil(l1);l1=oz_tail(l1)) {
    LTuple *t = tagged2LTuple(l1);
    oz_gCollectTerm(*t->getRefHead(),*t->getRefHead());
  }
  // if the finalize_list is not empty, we must create a new
  // thread (at top level) to effect the finalization phase
  if (!oz_isNil(finalize_list)) {
    Thread* thr = oz_newThreadInject(oz_rootBoard());
    thr->pushCall(finalize_handler,finalize_list);
    finalize_list = oz_nil();
  }
}

#endif

void OZ_cacBlock(OZ_Term * frm, OZ_Term * to, const int sz) {

  /*
   * Reserve space on the various stacks
   */

  /*
   * varFix: at maximum one entry per block field
   */

  varFix.ensureFree(sz);

  /*
   *  - REF*, GCTAG, SMALLINT, FSETVALUE, OZFLOAT:
   *     no entry:                               0
   *  - LITERAL:
   *     board:                                  1
   *  - EXT:
   *     board, entry                            2
   *  - LTUPLE, SRECORD:
   *     entry                                   1
   *  - UVAR:
   *     board:                                  1
   *  - CVAR:
   *     entry, board, susplist, loc. susplist:  5
   *  - OZCONST:
   *     UNBOUNDED for arbitray calls, so it must
   *     check locally!
   *     for local calls in this routine, at most:
   *     entry, board, board:                    3
   *
   *
   * conservative estimate: 5 entries
   *
   * DOES NOT WORK YET, BECAUSE OF UNSOLCITED ENTRIES INTO
   * GC ROUTINES
   *
   */

  // cacStack.ensureFree(sz * 5);

#ifdef S_CLONE

  /*
   * copying trail:
   *  - REF*, GCTAG, SMALLINT, FSETVALUE, OZFLOAT, LTUPLE:
   *     no entry:                               0
   *  - LITERAL:
   *     entry, board:                           2
   *  - EXT:
   *     board, entry                            2
   *  - SRECORD:
   *     entry                                   1
   *  - UVAR:
   *     entry, board:                           2
   *  - CVAR:
   *     entry, board:                           2
   *  - OZCONST:
   *    entry, board, board:                     3
   *
   * conservative estimate: 3 entries
   *
   * DOES NOT WORK YET, BECAUSE OF UNSOLCITED ENTRIES INTO
   * GC ROUTINES
   *
   */

  // cpTrail.ensureFree(sz * 2 * 3);

#endif

  for (int i=sz; i--; ) {
    TaggedRef aux = frm[i];

    switch (tagTypeOf(aux)) {

    case REF:
      Assert(aux);
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
            Board * bb = tagged2VarHome(aux);

            if (!NEEDSCOPYING(bb)) {
              to[i] = makeTaggedRef(aux_ptr);
            } else {
              bb = bb->_cacBoard();

              Assert(bb);
              varFix.defer(aux_ptr, &to[i]);
            }

          }
          break;

        case CVAR:
          {
            OzVariable * cv = tagged2CVar(aux);

            if (cv->cacIsMarked()) {
              Assert(tagTypeOf(*(cv->cacGetFwd())) == CVAR);
              to[i] = makeTaggedRef(cv->cacGetFwd());
            } else if (NEEDSCOPYING(cv->getBoardInternal())) {
              OzVariable *new_cv=cv->_cacVarInline();

              Assert(new_cv);

              TaggedRef * var_ptr = newTaggedCVar(new_cv);
              to[i] = makeTaggedRef(var_ptr);
              cv->_cacMark(var_ptr);
            } else {
              to[i] = makeTaggedRef(aux_ptr);
            }
          }
          break;

        }

      }
      break;

    case GCTAG: DO_GCTAG:
      to[i] = makeTaggedRef((TaggedRef*) GCUNMARK(aux));
      // This can lead to not shortened ref chains together with
      // the CONS forwarding: if a CONS cell is collected, then every
      // reference to the first element becomes a ref. May try this:
      // if (!isVar(*to)) to=deref(to); (no, cycles... CS)
      break;

    case SMALLINT: DO_SMALLINT:
      to[i] = aux;
      break;

    case FSETVALUE: DO_FSETVALUE:
#ifdef G_COLLECT
      to[i] = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(aux))->gCollect());
#else
      to[i] = aux;
#endif
      break;

    case LITERAL: DO_LITERAL:
      to[i] = makeTaggedLiteral(tagged2Literal(aux)->_cac());
      break;

    case EXT: DO_EXT:
      to[i] = _cacExtension(aux);
      break;

    case LTUPLE: DO_LTUPLE:
      to[i] = makeTaggedLTuple(tagged2LTuple(aux)->_cac());
      break;

    case SRECORD: DO_SRECORD:
      to[i] = makeTaggedSRecord(tagged2SRecord(aux)->_cacSRecord());
      break;

    case OZFLOAT: DO_OZFLOAT:
#ifdef G_COLLECT
      to[i] = makeTaggedFloat(tagged2Float(aux)->gCollect());
#else
      to[i] = aux;
#endif
      break;

    case OZCONST: DO_OZCONST:
      to[i] = makeTaggedConst(tagged2Const(aux)->_cacConstTerm());
      break;

    case UNUSED_VAR:  // FUT
      Assert(0);
      break;

    case UVAR: // direct var
      {
        Board * bb = tagged2VarHome(aux);

        Assert(bb);

        if (NEEDSCOPYING(bb)) {
          bb = bb->_cacBoard();
          Assert(bb);
          to[i] = makeTaggedUVar(bb);
        } else {
          frm[i] = makeTaggedRef(&to[i]);
          to[i]  = aux;
        }
        _cacStoreFwdMark((int32 *)&frm[i], &to[i], NO);
      }
      break;

    case CVAR: // direct cvar
      {
        OzVariable * cv = tagged2CVar(aux);

        if (cv->cacIsMarked()) {
          Assert(tagTypeOf(*(cv->cacGetFwd())) == CVAR);
          to[i] = makeTaggedRef(cv->cacGetFwd());
        } else if (NEEDSCOPYING(cv->getBoardInternal())) {
          to[i] = makeTaggedCVar(cv->_cacVarInline());
          cv->_cacMark(&to[i]);
        } else {
          // We cannot copy the variable, but we have already copied
          // their taggedref, so we change the original variable to a ref
          // of the copy.
          // After pushing on the update stack the
          // the original variable is replaced by a reference!
          frm[i] = makeTaggedRef(&to[i]);
          to[i]  = aux;
          _cacStoreFwdMark((int32*) &frm[i], &to[i], NO);
        }

      }
      break;

    }
  }

}


#ifdef G_COLLECT

//*****************************************************************************
//                               AM::gc
//*****************************************************************************


// This method is responsible for the heap garbage collection of the
// abstract machine, ie that all entry points into heap are properly
// treated and references to variables are properly updated
void AM::gCollect(int msgLevel) {

  (*gCollectFrameToProxy)();

#ifdef DEBUG_CHECK
  isCollecting = OK;
#endif

  ozstat.initGcMsg(msgLevel);

  MemChunks * oldChain = MemChunks::list;

#ifndef NEW_NAMER
  oz_varCleanup();  /* drop bound variables */
#endif

  GCDBG_INITSPACE;

  initMemoryManagement();

  for (int j=NumberOfXRegisters; j--; )
    xRegs[j] = taggedVoidValue;

  Assert(trail.getUsed() == 1);
  Assert(cachedSelf==0);
  Assert(ozstat.currAbstr==NULL);
  Assert(_inEqEq==FALSE);
  Assert(_rootBoard);

  _rootBoard = _rootBoard->gCollectBoard();   // must go first!
  setCurrent(_currentBoard->gCollectBoard());

  aritytable.gCollect();
  threadsPool.gCollect();

  // mm2: Assert(isEmptySuspendVarList());
  emptySuspendVarList();

  if (defaultExceptionHdl)
    oz_gCollectTerm(defaultExceptionHdl,defaultExceptionHdl);
  oz_gCollectTerm(debugStreamTail,debugStreamTail);

  CodeArea::gCollectCodeAreaStart();
  PrTabEntry::gCollectPrTabEntries();
  extRefs = extRefs->gCollect();

  oz_gCollectTerm(finalize_handler,finalize_handler);
  cacStack.gCollectRecurse();
  gCollect_finalize();
  gCollectWeakDictionaries();
  gCollectDeferWatchers();
  (*gCollectPerdioRoots)();
  cacStack.gCollectRecurse();

  (*gCollectBorrowTableUnusedFrames)();
  cacStack.gCollectRecurse();

#ifdef NEW_NAMER
  GCMeManager::gCollect();
  cacStack.gCollectRecurse();
#endif

  weakStack.recurse();          // must come after namer gc

// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now

  varFix.gCollectFix();

  Assert(cacStack.isEmpty());

  GT.gCollectGNameTable();
  //   MERGECON gcPerdioFinal();
  gCollectSiteTable();
  (*gCollectPerdioFinal)();
  Assert(cacStack.isEmpty());

  GCDBG_EXITSPACE;

  CodeArea::gCollectCollectCodeBlocks();
  AbstractionEntry::freeUnusedEntries();

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  cachedStack = NULL;

  ozstat.printGcMsg(msgLevel);

#ifdef DEBUG_CHECK
  isCollecting = NO;
#endif

} // AM::gc

#endif

/*
 * After collection has finished, update variable references
 *
 */
void VarFix::_cacFix(void) {

  if (isEmpty())
    return;

  do {
    TaggedRef * to = (TaggedRef *) pop();

    Assert(oz_isRef(*to));

    TaggedRef * aux_ptr = tagged2Ref(*to);
    TaggedRef   aux     = *aux_ptr;

    TaggedRef * to_ptr  =
      (tagTypeOf(aux) == UVAR) ?
      newTaggedUVar(tagged2VarHome(aux)->derefBoard()->cacGetFwd()) :
      (TaggedRef *) GCUNMARK(aux);

    Assert(tagTypeOf(aux) == UVAR || tagTypeOf(aux) == GCTAG);

    *to = makeTaggedRef(to_ptr);
    _cacStoreFwdMark((int32 *) aux_ptr, to_ptr, OK);

  } while (!isEmpty());

}


#ifdef S_CLONE

#ifdef CS_PROFILE
static Bool across_redid = NO;
#endif

/*
 * Before copying all spaces but the space to be copied get marked.
 *
 * Important: even committed boards must be marked, since the globality
 * test does not do a dereference!
 *
 */

void Board::setGlobalMarks(void) {
  Assert(!isRoot());

  Board * b = this;

  do {
    b = b->getParentInternal();
    Assert(!b->hasMarkOne());
    b->setMarkOne();
  } while (!b->isRoot());

}

/*
 * Purge marks after copying
 */

void Board::unsetGlobalMarks(void) {
  Assert(!isRoot());

  Board * b = this;

  do {
    b = b->getParentInternal();
    Assert(b->hasMarkOne());
    b->unsetMarkOne();
  } while (!b->isRoot());

}

Board * Board::clone(void) {

#ifdef CS_PROFILE
  across_redid  = NO;
  across_chunks = NO;
#endif

#ifdef DEBUG_CHECK
  isCollecting = OK;
#endif

  unsigned int starttime = 0;

  if (ozconf.timeDetailed)
    starttime = osUserTime();

#ifdef CS_PROFILE
redo:
  if (across_redid)
    OZ_error("Redoing cloning across chunk boundaries. Giving up!\n");

  if (across_chunks)
    across_redid = OK;

  across_chunks = NO;

  cs_orig_start = (int32 *) heapTop;
#endif

  Assert(!isCommitted());

  setGlobalMarks();

  Board * copy = sCloneBoard();

  Assert(copy);

  cacStack.sCloneRecurse();

  varFix.sCloneFix();

#ifdef NEW_NAMER
  if (am.isPropagatorLocation()) {
    GCMeManager::sClone();
  }
#endif

  cpTrail.unwind();

  unsetGlobalMarks();

#ifdef CS_PROFILE
  if (across_chunks) {
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

#ifdef DEBUG_CHECK
  isCollecting = NO;
#endif

  return copy;
}

#endif



#ifdef G_COLLECT

//*****************************************************************************
//                                GC-METHODS
//*****************************************************************************


/*
 * class Arity is not allocated on heap!
 * but must collect the list && the values in keytable
 */
inline
void Arity::gCollect(void) {
  Arity *aux = this;
  while(aux) {
    if (!aux->isTuple()) {
      for (int i = aux->getSize(); i--; ) {
        if (aux->table[i].key) {
          oz_gCollectTerm(aux->table[i].key,aux->table[i].key);
        }
      }
    }
    oz_gCollectTerm(aux->list,aux->list);
    aux = aux->next;
  }
}

void ArityTable::gCollect(void) {
  for (int i = size; i--; ) {
    if (table[i] != NULL)
      (table[i])->gCollect();
  }
}

void PrTabEntry::gCollectPrTabEntries()
{
  PrTabEntry *aux = allPrTabEntries;
  while(aux) {
    oz_gCollectTerm(aux->info,aux->info);
    oz_gCollectTerm(aux->file,aux->file);
    oz_gCollectTerm(aux->printname,aux->printname);
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

void AbstractionEntry::gCollectAbstractionEntry(void) {
  if (this==NULL || collected) return;

  collected = OK;
  abstr = gCollectAbstraction(abstr);
}

#endif

/*
 * CONST collection
 *
 */

ObjectClass * ObjectClass::_cacClass() {
  return this ? ((ObjectClass *) _cacConstTerm()) : this;
}


#ifdef G_COLLECT

// failure interface for local tertiarys
inline void maybeGCForFailure(Tertiary *t) {
  if(t->getInfo()!=NULL)
    (*gCollectEntityInfo)(t);
}

inline
void ConstTermWithHome::gCollectConstTermWithHome(void) {
  if (hasGName()) {
    dogcGName(getGName1());
  } else {
    setBoard(getSubBoardInternal()->gCollectBoard());
  }
}

inline
void Tertiary::gCollectTertiary(void) {
  if (isLocal()) {
    setBoardLocal(getBoardLocal()->gCollectBoard());
  }
}

#else

#define maybeGCForFailure(t)

inline
void ConstTermWithHome::sCloneConstTermWithHome(void) {
  Assert(!hasGName());
  setBoard(getSubBoardInternal()->sCloneBoard());
}


inline
void Tertiary::sCloneTertiary(void) {
  Assert(isLocal());
  setBoardLocal(getBoardLocal()->sCloneBoard());
}


#endif



void ConstTerm::_cacConstRecurse(void) {
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;

      switch(o->getTertType()) {
      case Te_Local:
        maybeGCForFailure(o);
        break;
#ifdef G_COLLECT
      case Te_Proxy:   // PER-LOOK is this possible?
        (*gCollectProxyRecurse)(o);
        (*gCollectEntityInfo)(o);
        break;
      case Te_Manager:
        (*gCollectManagerRecurse)(o);
        (*gCollectEntityInfo)(o);
        break;
#endif
      default:         Assert(0);
      }

      o->setClass(o->getClass()->_cacClass());
      if (o->getFreeRecord())
        o->setFreeRecord(o->getFreeRecord()->_cacSRecord());
      RecOrCell state = o->getState();
      if (stateIsCell(state)) {
        if (o->isLocal() && getCell(state)->isLocal()) {
          TaggedRef newstate = ((CellLocal*) getCell(state))->getValue();
          o->setState(tagged2SRecord(oz_deref(newstate))->_cacSRecord());
        } else if (getCell(state)) {
          o->setState((Tertiary*) getCell(state)->_cacConstTerm());
        }
      } else {
        o->setState(getRecord(state)->_cacSRecord());
      }
      if (o->getLock())
        o->lock = (OzLock *) o->getLock()->_cacConstTerm();
      break;
    }

  case Co_Class:
    {
      ObjectClass *cl = (ObjectClass *) this;
      cl->fastMethods    = (OzDictionary*) cl->fastMethods->_cacConstTerm();
      cl->defaultMethods = (OzDictionary*) cl->defaultMethods->_cacConstTerm();
      cl->features       = cl->features->_cacSRecord();
      if (cl->unfreeFeatures)
        cl->unfreeFeatures = cl->unfreeFeatures->_cacSRecord();
      break;
    }

  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
#ifdef G_COLLECT
      gCollectCode(a->getPred()->getCodeBlock());
#endif
      OZ_cacBlock(a->getGRef(),a->getGRef(),
                  a->getPred()->getGSize());
      break;
    }

  case Co_Cell:
    {
      Tertiary *t=(Tertiary*)this;
      if (t->isLocal()) {
        CellLocal *cl=(CellLocal*)t;
        oz_cacTerm(cl->val,cl->val);
        maybeGCForFailure(t);
      }
#ifdef G_COLLECT
      else {
        (*gCollectDistCellRecurse)(t);
      }
#endif
      break;
    }

  case Co_Port:
    {
      Port *p = (Port*) this;
      if (p->isLocal()) {
        PortWithStream *pws = (PortWithStream *) this;
        oz_cacTerm(pws->strm,pws->strm);
        maybeGCForFailure(p);
        break;
      }
#ifdef G_COLLECT
      else {
        (*gCollectDistPortRecurse)(p);
      }
#endif
      break;
    }

  case Co_Chunk:
    {
      SChunk *c = (SChunk *) this;
      oz_cacTerm(c->value,c->value);
      break;
    }

  case Co_Array:
    {
      OzArray *a = (OzArray*) this;
      int aw = a->getWidth();
      if (aw > 0) {
        TaggedRef *newargs = (TaggedRef*) heapMalloc(sizeof(TaggedRef) * aw);
        OZ_cacBlock(a->getArgs(), newargs, aw);
        a->args=newargs;
      }
      break;
    }

  case Co_Dictionary:
    {
      OzDictionary *dict = (OzDictionary *) this;
      dict->table = dict->table->_cac();
      break;
    }

  case Co_Lock:
    {
      Tertiary *t=(Tertiary*)this;
      if (t->isLocal()) {
        LockLocal *ll = (LockLocal *) this;
#ifdef G_COLLECT
        gCollectPendThreadEmul(&(ll->pending));
#endif
        ll->setLocker(SuspToThread(ll->getLocker()->_cacSuspendable()));
        maybeGCForFailure(t);
        break;
      }
#ifdef G_COLLECT
      else {
        (*gCollectDistLockRecurse)(t);
      }
#endif
      break;
    }

  default:
    Assert(0);
  }
}

#ifdef S_CLONE

#define CheckLocalWithHome                                \
{                                                         \
   ConstTermWithHome * ctwh = (ConstTermWithHome *) this; \
   if (ctwh->hasGName()) return this;                     \
   Board * bb = ctwh->getSubBoardInternal();              \
   Assert(bb->cacIsAlive());                              \
   if (!NEEDSCOPYING(bb)) return this;                    \
}

#define CheckLocalTertiary             \
{                                      \
   Tertiary * t = (Tertiary *) this;   \
   if (!t->isLocal()) return this;     \
   Board * bb = t->getBoardLocal();    \
   Assert(bb->cacIsAlive());           \
   if (!NEEDSCOPYING(bb)) return this; \
}

#else

#define CheckLocalWithHome

#define CheckLocalTertiary

#endif

ConstTerm *ConstTerm::_cacConstTerm() {
  Assert(this);

  if (cacIsMarked())
    return cacGetFwd();

  ConstTerm * ret;

  switch (getType()) {

    /*
     * Unsituated types
     *
     */

  case Co_BigInt:
#ifdef G_COLLECT
    ret = ((BigInt *) this)->gCollect();
    storeFwdField(this, ret, OK);
    return ret;
#else
    return this;
#endif

  case Co_Foreign_Pointer:
#ifdef G_COLLECT
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(ForeignPointer));
    storeFwdField(this, ret, OK);
    return ret;
#else
    return this;
#endif

  case Co_Resource:
#ifdef G_COLLECT
    ret = (*gCollectDistResource)(this);
    storeFwdField(this, ret, OK);
    return ret;
#else
    return this;
#endif

  case Co_Builtin:
    return this;

    /*
     * ConstTermWithHome
     *
     */

  case Co_Abstraction:
    CheckLocalWithHome;
    ret = (ConstTerm *) oz_hrealloc(this,
                                    ((Abstraction *) this)->getAllocSize());
    goto const_withhome;

  case Co_Chunk:
    CheckLocalWithHome;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(SChunk));
    goto const_withhome;

  case Co_Array:
    CheckLocalWithHome;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(OzArray));
    goto const_withhome;

  case Co_Dictionary:
    CheckLocalWithHome;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(OzDictionary));
    goto const_withhome;

  case Co_Class:
    CheckLocalWithHome;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(ObjectClass));
    goto const_withhome;

    /*
     * Tertiary
     *
     */

  case Co_Object:
    CheckLocalTertiary;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(Object));
#ifdef G_COLLECT
    dogcGName(((Object *) this)->getGName1());
#endif
    goto const_tertiary;

  case Co_Cell:
    CheckLocalTertiary;
#ifdef G_COLLECT
    if (((Tertiary *)this)->getTertType() == Te_Frame) {
      ret = (ConstTerm *) _cacReallocStatic(this,sizeof(CellFrameEmul));
    } else {
      ret = (ConstTerm *) _cacReallocStatic(this,sizeof(CellManagerEmul));
    }
#else
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(CellManagerEmul));
#endif
    goto const_tertiary;

  case Co_Port:
    CheckLocalTertiary;
#ifdef G_COLLECT
    if (((Tertiary *)this)->getTertType() == Te_Proxy) {
      ret = (ConstTerm *) _cacReallocStatic(this, SIZEOFPORTPROXY);
    } else {
      ret = (ConstTerm *) _cacReallocStatic(this,sizeof(PortLocal));
    }
#else
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(PortLocal));
#endif
    goto const_tertiary;

  case Co_Space:
    CheckLocalTertiary;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(Space));
    {
      Space *s = (Space *) ret;
      if (!s->isProxy()) {
        if (!s->isMarkedFailed() && !s->isMarkedMerged()) {
          if (s->solve->cacIsAlive()) {
            s->solve = s->solve->_cacBoard();
          } else {
            s->solve = (Board *) 0;
            Assert(s->isMarkedFailed());
          }
        }
      }
    }
    goto const_tertiary_nopush;

  case Co_Lock:
    CheckLocalTertiary;
    ret = (ConstTerm *) _cacReallocStatic(this,sizeof(LockLocal));
    goto const_tertiary;

  }

 const_tertiary:
  // Reserve space on stack
  cacStack.push(ret,PTR_CONSTTERM);
 const_tertiary_nopush:
  ((Tertiary *) ret)->_cacTertiary();
  storeFwdField(this, ret, OK);
  return ret;

 const_withhome:
  ((ConstTermWithHome *) ret)->_cacConstTermWithHome();
  cacStack.push(ret,PTR_CONSTTERM);
  storeFwdField(this, ret, OK);
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

#ifdef G_COLLECT

inline
OzDebug *OzDebug::gCollectOzDebug() {
  OzDebug *ret = (OzDebug*) _cacReallocStatic(this,sizeof(OzDebug));

  ret->Y   = gCollectRefsArray(ret->Y);
  ret->CAP = gCollectAbstraction(ret->CAP);

  if (ret->data)
    oz_gCollectTerm(ret->data,ret->data);

  if (ret->arity > 0) {
    ret->arguments = (TaggedRef *)
      heapMalloc(ret->arity * sizeof(TaggedRef));

    OZ_gCollectBlock(arguments, ret->arguments, arity);
  }

  return ret;
}

#endif


inline
TaskStack * TaskStack::_cac(void) {

  TaskStack *newstack = new TaskStack(suggestNewSize());
  TaskStack *oldstack = this;

  Frame *oldtop = oldstack->getTop();
  int offset    = oldstack->getUsed();
  Frame *newtop = newstack->array + offset;

  while (1) {
    GetFrame(oldtop,PC,Y,CAP);

#ifdef G_COLLECT
    gCollectCode(PC);
#endif

    if (PC == C_EMPTY_STACK) {
      *(--newtop) = PC;
      *(--newtop) = Y;
      *(--newtop) = CAP;
      Assert(newstack->array == newtop);
      newstack->setTop(newstack->array+offset);
      return newstack;
    } else if (PC == C_CATCH_Ptr) {
    } else if (PC == C_XCONT_Ptr) {
      // mm2: opt: only the top task can/should be xcont!!
      ProgramCounter pc   = (ProgramCounter) *(oldtop-1);
#ifdef G_COLLECT
      gCollectCode(pc);
#endif
      (void) CodeArea::livenessX(pc,Y,getRefsArraySize(Y));
      Y = _cacRefsArray(Y); // X
    } else if (PC == C_LOCK_Ptr) {
      Y = (RefsArray) ((OzLock *) Y)->_cacConstTerm();
    } else if (PC == C_SET_SELF_Ptr) {
      Y = (RefsArray) (Y ? (((Object*)Y)->_cacConstTerm()) : 0);
    } else if (PC == C_SET_ABSTR_Ptr) {
      ;
    } else if (PC == C_DEBUG_CONT_Ptr) {
#ifdef G_COLLECT
      Y = (RefsArray) ((OzDebug *) Y)->gCollectOzDebug();
#else
      Assert(0);
#endif
    } else if (PC == C_CALL_CONT_Ptr) {
      /* tt might be a variable, so use this ugly construction */
      *(newtop-2) = Y; /* UGLYYYYYYYYYYYY !!!!!!!! */
      TaggedRef *tt = (TaggedRef*) (newtop-2);
      oz_cacTerm(*tt,*tt);
      Y = (RefsArray) ToPointer(*tt);
      CAP = (Abstraction *) _cacRefsArray((RefsArray)CAP);
    } else if (PC == C_CFUNC_CONT_Ptr) {
      CAP = (Abstraction *) _cacRefsArray((RefsArray)CAP);
    } else { // usual continuation
      Y   = _cacRefsArray(Y);
      CAP = _cacAbstraction(CAP);
    }

    *(--newtop) = PC;
    *(--newtop) = Y;
    *(--newtop) = CAP;
  } // while not task stack is empty
}

inline
void Thread::_cacRecurse(Thread * fr) {
  taskStack = fr->taskStack->_cac();
  abstr     = fr->abstr;
  id        = fr->id;
}

inline
void Propagator::_cacRecurse(Propagator * fr) {
  _p = fr->_p->cac();
  _p->_cac();

}

inline
Suspendable * Suspendable::_cacSuspendableInline(void) {
  Assert(this);

  if (isCacMarked())
    return cacGetFwd();

  if (isDead())
    return (Suspendable *) NULL;

  Suspendable * to;

  if (getBoardInternal()->cacIsAlive()) {
#ifdef S_CLONE
    Assert(!isRunnable());
#endif

    if (isThread()) {
      to = (Suspendable *) CAC_MALLOC(sizeof(Thread));

      ((Thread *) to)->_cacRecurse(SuspToThread(this));

    } else {
      to = (Suspendable *) CAC_MALLOC(sizeof(Propagator));

      ((Propagator *) to)->_cacRecurse(SuspToPropagator(this));

    }

    to->setBoardInternal(getBoardInternal()->_cacBoard());

  } else if (isThread()) {

    to = (Suspendable *) _cacReallocStatic(this, sizeof(Thread));

    Board * nb = getBoardInternal()->cacGetNotificationBoard()->_cacBoard();

    ((Thread *) to)->setTaskStack(new TaskStack(ozconf.stackMinSize));

    nb->incSuspCount();

    ((Thread *) to)->pushCall(BI_skip,0,0);

    to->setBoardInternal(nb);
  } else {
    return NULL;
  }

  to->flags = flags;
  storeFwdField(this, to, OK);

  return to;
}

inline
Propagator * Propagator::_cacLocalInline(Board * bb) {
  Assert(this);

  if (isCacMarked())
    return SuspToPropagator(cacGetFwd());

  Assert(isPropagator());

  if (isDead())
    return (Propagator *) NULL;

  Propagator * to;

  Assert(getBoardInternal()->cacIsAlive());
#ifdef S_CLONE
  Assert(!isRunnable());
#endif

  Assert(getBoardInternal()->derefBoard()->cacIsMarked() &&
         getBoardInternal()->derefBoard()->cacGetFwd() == bb);

  to = (Propagator *) CAC_MALLOC(sizeof(Propagator));

  to->_cacRecurse(SuspToPropagator(this));

  to->setBoardInternal(bb);

  to->flags = flags;

  storeFwdField(this, to, OK);

  Assert(to->isPropagator());

  return to;
}

Suspendable * Suspendable::_cacSuspendable(void) {
  return (this == NULL) ? (Suspendable *) NULL : _cacSuspendableInline();
}

/*
 * This routine MUST maintain the order, since it is also used
 * for ordered susplists and suspendable queues
 *
 */

inline
SuspList * SuspList::_cacRecurse(SuspList ** last) {
  SuspList * sl = this;
  SuspList * pl = SuspList::_gc_sentinel;

  while (sl) {
    Suspendable * to = sl->getSuspendable()->_cacSuspendableInline();

    if (to) {
      SuspList * nl = new SuspList(to);
      pl->setNext(nl);
      pl = nl;
    }

    sl = sl->getNext();

  };

  pl->setNext(NULL);

  if (last)
    *last = pl;

  return SuspList::_gc_sentinel->getNext();
}

inline
SuspList * SuspList::_cacLocalRecurse(Board * bb) {
  SuspList * ret;
  SuspList ** p = &ret;

  for (SuspList * sl = this; sl; sl=sl->getNext()) {
    Suspendable * to =
      SuspToPropagator(sl->getSuspendable())->_cacLocalInline(bb);

    if (to) {
      SuspList * n = new SuspList(to);
      *p = n;
      p  = &(n->_next);
    }

  }

  *p = NULL;

  return ret;
}

void SuspQueue::_cac(void) {
  if (isEmpty())
    return;

  SuspList * head = last->getNext();

#ifdef S_CLONE
  cpTrail.save((int32 *) last->getNextRef(),OK);
#endif

  last->setNext(NULL);

  head = head->_cacRecurse(&last);

  last->setNext(head);

}

#ifdef G_COLLECT

void ThreadsPool::gCollect(void) {
  _q[ HI_PRIORITY].gCollect();
  _q[MID_PRIORITY].gCollect();
  _q[LOW_PRIORITY].gCollect();
}

#endif

/****************************************************************************
 * Board collection
 ****************************************************************************/

inline
DistBag * DistBag::_cac(void) {
  DistBag *  copy = (DistBag *) 0;
  DistBag ** cur  = &copy;
  DistBag *  old  = this;

  while (old) {
    DistBag * one = new DistBag(old->dist->_cac());
    *cur = one;
    cur  = &(one->next);
    old  = old->next;
  }

  *cur = 0;

  return copy;
}


inline
void Board::_cacRecurse() {
  Assert(!isCommitted() && !isFailed());

  // Do not recurse over root board (be it the global one or
  // the root board for cloning!)
  if (!isRoot() && !getParentInternal()->hasMarkOne())
    parentAndFlags.set(getParentInternal()->_cacBoard(),0);

  lpq._cac();

  oz_cacTerm(script,script);
  oz_cacTerm(rootVar,rootVar);
  oz_cacTerm(status,status);

  cacSuspList(&suspList,OK);
  setDistBag(getDistBag()->_cac());
  cacSuspList((SuspList **) &nonMonoSuspList,OK);

#ifdef CS_PROFILE
#ifdef G_COLLECT
  if((copy_size>0) && copy_start) {
    free(copy_start);
  }
#endif
  orig_start = (int32 *) NULL;
  copy_start = (int32 *) NULL;
  copy_size  = 0;
#endif

}


//*****************************************************************************
//                           collectGarbage
//*****************************************************************************

inline
void SRecord::_cacRecurse() {
  SRecord * to = (SRecord *) GCUNMARK(label);

  oz_cacTerm(to->label,to->label);

  OZ_cacBlock(getRef(), to->getRef(), getWidth());

}


inline
void LTuple::_cacRecurse() {
  LTuple * frm = this;
  LTuple * to  = (LTuple *) GCUNMARK(frm->args[0]);
  TaggedRef aux = oz_deref(to->args[0]);

  //
  if (!oz_isLTuple(aux) || tagged2LTuple(aux) != this) {
    frm->args[0] = to->args[0];
    oz_cacTerm(frm->args[0], to->args[0]);
    _cacStoreFwdMark((int32 *)frm->args, to->args, OK);
  } else {
    to->args[0] = makeTaggedLTuple((LTuple *) to);
  }

  while (1) {
    // Store forward, order is important, since collection might already
    // have done a storeFwd, which means that this one will be overwritten
    TaggedRef t = oz_deref(frm->args[1]);

    if (!oz_isCons(t)) {
      oz_cacTerm(frm->args[1], to->args[1]);
      return;
    }

    frm = tagged2LTuple(t);

    if (GCISMARKED(frm->args[0])) {
      to->args[1] = makeTaggedLTuple((LTuple *) GCUNMARK(frm->args[0]));
      return;
    }

    LTuple * next = (LTuple *) CAC_MALLOC(sizeof(LTuple));

    to->args[1] = makeTaggedLTuple(next);
    to = next;

    oz_cacTerm(frm->args[0], to->args[0]);

    _cacStoreFwdMark((int32 *)frm->args, to->args, OK);

  }

  Assert(0);
}


void CacStack::_cacRecurse(void) {

  while (!isEmpty()) {
    TaggedRef tp  = (TaggedRef) pop();
    void * ptr    = tagValueOf(tp);
    TypeOfPtr how = (TypeOfPtr) tagTypeOf(tp);

    switch(how) {
    case PTR_LTUPLE:
      ((LTuple *) ptr)->_cacRecurse();
      break;
    case PTR_SRECORD:
      ((SRecord *) ptr)->_cacRecurse();
      break;
    case PTR_BOARD:
      ((Board *) ptr)->_cacRecurse();
      break;
    case PTR_CVAR:
      ((OzVariable *) ptr)->_cacVarRecurse();
      break;
    case PTR_CONSTTERM:
      ((ConstTerm *) ptr)->_cacConstRecurse();
      break;
    case PTR_EXTENSION:
      ((OZ_Extension *) ptr)->_cacRecurseV();
      break;
    case PTR_SUSPLIST:
      *((SuspList **) ptr) = (*(SuspList **) ptr)->_cacRecurse(NULL);
      break;
    case PTR_UNUSED2:
      Assert(0);
    default:
      {
        Assert(how & PTR_LOCAL_SUSPLIST);

        SuspList ** sl = (SuspList **) ptr;
        Board    *  bb = (Board *) pop();

        for (int i = how - PTR_LOCAL_SUSPLIST; i--; )
          sl[i] = sl[i]->_cacLocalRecurse(bb);

      }
    }
  }
}


#ifdef G_COLLECT

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

void AM::doGCollect() {
  Assert(oz_onToplevel());

  /* do gc */
  gCollect(ozconf.gcVerbosity);

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

//*****************************************************************************
//       GC Code Area
//*****************************************************************************

static int codeGCgeneration = CODE_GC_CYLES;

void CodeArea::gCollectCodeBlock()
{
  if (referenced == NO) {
    referenced = OK;
    gclist->collectGClist();
  }
}

void gCollectCode(CodeArea *block) {
  if (codeGCgeneration!=0)
    return;

  block->gCollectCodeBlock();
}


void gCollectCode(ProgramCounter PC) {
  gCollectCode(CodeArea::findBlock(PC));
}

void CodeGCList::collectGClist()
{
  CodeGCList *aux = this;
  while(aux) {
    for (int i=aux->nextFree; i--; ) {
      switch(aux->block[i].tag) {
      case C_TAGGED:
        oz_gCollectTerm(*(TaggedRef*)aux->block[i].pc,
                        *(TaggedRef*)aux->block[i].pc);
        break;
      case C_ABSTRENTRY:
        ((AbstractionEntry*)*(aux->block[i].pc))->gCollectAbstractionEntry();
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

void CodeArea::gCollectCodeAreaStart()
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
    code->gCollectCodeBlock();
    code = code->nextBlock;
  }
}

void CodeArea::gCollectCollectCodeBlocks()
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

#endif
