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
#include "mozart_cpi.hh"

#ifdef OUTLINE
#define inline
#endif

/*
 * Do some bad style renaming
 *
 */

#ifdef G_COLLECT

#define OZ_cacBlock              OZ_gCollectBlock
#define OZ_cacAllocBlock         OZ_gCollectAllocBlock
#define oz_cacTerm               oz_gCollectTerm

#define _cacRecurse              gCollectRecurse
#define _cac                     gCollect
#define _cacRecurseV             gCollectRecurseV
#define _cacV                    gCollectV

#define _cacMark                 gCollectMark
#define _cacFix                  gCollectFix

#define _cacConstTermInline      gCollectConstTermInline
#define _cacConstRecurse         gCollectConstRecurse

#define _cacBoard                gCollectBoard
#define _cacBoardDo              gCollectBoardDo

#define _cacName                 gCollectName

#define _cacVarRecurse           gCollectVarRecurse
#define _cacVarInline            gCollectVarInline

#define _cacSRecord              gCollectSRecord

#define _cacSuspendableInline    gCollectSuspendableInline
#define _cacSuspendable          gCollectSuspendable

#define _cacLocalInline          gCollectLocalInline
#define _cacLocalRecurse         gCollectLocalRecurse

#define _cacExtension            gCollectExtension
#define _cacExtensionRecurse     gCollectExtensionRecurse

#define _cacPendThreadEmul       gCollectPendThreadEmul

#define _cacRefsArray            gCollectRefsArray

#define _cacSuspList             gCollectSuspList
#define _cacLocalSuspList        gCollectLocalSuspList

#endif


#ifdef S_CLONE

#define OZ_cacBlock              OZ_sCloneBlock
#define OZ_cacAllocBlock         OZ_sCloneAllocBlock
#define oz_cacTerm               oz_sCloneTerm

#define _cacRecurse              sCloneRecurse
#define _cac                     sClone
#define _cacRecurseV             sCloneRecurseV
#define _cacV                    sCloneV

#define _cacMark                 sCloneMark
#define _cacFix                  sCloneFix

#define _cacConstTermInline      sCloneConstTermInline
#define _cacConstRecurse         sCloneConstRecurse

#define _cacBoard                sCloneBoard
#define _cacBoardDo              sCloneBoardDo

#define _cacName                 sCloneName

#define _cacVarRecurse           sCloneVarRecurse
#define _cacVarInline            sCloneVarInline

#define _cacSRecord              sCloneSRecord

#define _cacSuspendableInline    sCloneSuspendableInline
#define _cacSuspendable          sCloneSuspendable

#define _cacLocalInline          sCloneLocalInline
#define _cacLocalRecurse         sCloneLocalRecurse

#define _cacExtension            sCloneExtension
#define _cacExtensionRecurse     sCloneExtensionRecurse

#define _cacPendThreadEmul       sClonePendThreadEmul

#define _cacRefsArray            sCloneRefsArray

#define _cacSuspList             sCloneSuspList
#define _cacLocalSuspList        sCloneLocalSuspList

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


#ifdef S_CLONE

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
 * Allocate and copy memory blocks for which the size is known at compile
 * time.
 *
 */

#define cacReallocStatic(Type,f,t,n) \
{                                                                            \
  int _n = (n);                                                              \
  Assert(_n % sizeof(int) == 0);                                             \
  int32 * _f = (int32 *) (f);                                                \
  int32 * _t = (int32 *) CAC_MALLOC(_n);                                     \
  switch (_n) {                                                              \
  case 24:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];_t[3]=_f[3];_t[4]=_f[4];_t[5]=_f[5]; \
    break;                                                                   \
  case 20:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];_t[3]=_f[3];_t[4]=_f[4];             \
    break;                                                                   \
  case 16:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];_t[3]=_f[3];                         \
    break;                                                                   \
  case 12:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];                                     \
    break;                                                                   \
  case 8:                                                                    \
    _t[0]=_f[0];_t[1]=_f[1];                                                 \
    break;                                                                   \
  case 4:                                                                    \
    _t[0]=_f[0];                                                             \
    break;                                                                   \
  default:                                                                   \
    memcpy(_t, _f, _n);                                                      \
  }                                                                          \
  t = (Type *) _t;                                                           \
}


#ifdef S_CLONE

#define CPTRAIL(p) cpTrail.save(((int *) p))

#else

#define CPTRAIL(p)

#endif

/*
 * Marking of collected data structures
 *
 *
 * Write a marked forward pointer (pointing into the to-space)
 * into a structure in the from-space.
 *
 */
#define STOREFWDNOMARK(fromPtr, newValue)  \
  CPTRAIL(fromPtr);                        \
  *((int32 *) fromPtr) = ToInt32(newValue);

#define STOREFWDMARK(fromPtr, newValue)   \
  CPTRAIL(fromPtr);                       \
  *((int32 *) fromPtr) = makeTaggedGcMark(newValue);

#define STOREFWDFIELD(d,t) \
  CPTRAIL(d->cacGetMarkField()); \
  d->cacMark(t);



/*
 *   Collection of terms
 *
 */


/*
 * RefsArrays
 *
 */

// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc tag set --> has already been copied

#define RAGCTag (1<<31)

#define RefsArrayIsMarked(r) (r[-1]&RAGCTag)

#define RefsArrayMark(r,ptr) \
  STOREFWDNOMARK((int32*)&r[-1],ToPointer(ToInt32(ptr)|RAGCTag));

#define RefsArrayUnmark(r) \
 ((RefsArray) ToPointer(r[-1]&(~(RAGCTag)|mallocBase)))

inline
RefsArray _cacRefsArray(RefsArray r) {
  if (r == NULL)
    return r;

  GCDBG_NOTINTOSPACE(r);

  if (RefsArrayIsMarked(r)) {
    return RefsArrayUnmark(r);
  }

  int sz = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(sz,NO);

  RefsArrayMark(r,aux);

  OZ_cacBlock(r, aux, sz);

  return aux;
}


/*
 * Boards:
 *
 */


// Test whether a board must be copied
#ifdef G_COLLECT

#define NEEDSCOPYING(bb) (OK)

#else

#define NEEDSCOPYING(bb) (!(bb)->hasMarkOne())

#endif


inline
Board * Board::_cacBoard(void) {
  GCDBG_INFROMSPACE(this);

  // Do not clone a space above or collect a space above root ;-(
  Assert(this && !hasMarkOne());

  return cacIsMarked() ? cacGetFwd() : _cacBoardDo();
}

Board * Board::_cacBoardDo(void) {
  Board * bb = derefBoard();

  Assert(bb->cacIsAlive());

  if (bb->cacIsMarked())
    return bb->cacGetFwd();

  Board * ret;
  cacReallocStatic(Board,bb,ret,sizeof(Board));

  cacStack.push(ret,PTR_BOARD);

  STOREFWDFIELD(bb,ret);

  return ret;
}

/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

inline
Name *Name::_cacName(void) {

  if (cacIsMarked())
    return cacGetFwd();

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

    Name * aux;
    cacReallocStatic(Name,this,aux,sizeof(Name));

    STOREFWDFIELD(this, aux);

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


/*
 * Dynamic tables
 *
 */

inline
DynamicTable * DynamicTable::_cac(void) {
  Assert(isPwrTwo(size));

  // Copy the table:
  DynamicTable * to = (DynamicTable *) heapMalloc((size-1)*sizeof(HashElement)
                                                  + sizeof(DynamicTable));
  to->numelem = numelem;
  to->size    = size;

  OZ_cacBlock((TaggedRef *) table, (TaggedRef *) to->table, 2 * size);

  return to;
}

/*
 * Variables
 *
 */

inline
void OzVariable::_cacMark(TaggedRef * fwd) {
  Assert(!cacIsMarked());
  CPTRAIL((int32 *) &suspList);
  suspList = (SuspList *) MarkPointer(fwd,1);
}

inline
void OzFDVariable::_cac(Board * bb) {
  ((OZ_FiniteDomainImpl *) &finiteDomain)->copyExtension();

  cacStack.pushLocalSuspList(bb, &(fdSuspList[0]), fd_prop_any);
}

inline
void OzFSVariable::_cac(Board * bb) {

#ifdef BIGFSET
  _fset.copyExtension();
#endif

  cacStack.pushLocalSuspList(bb, &(fsSuspList[0]), fs_prop_any);
}

inline
void OzCtVariable::_cac(Board * bb) {
  // suspension lists
  int noOfSuspLists = getNoOfSuspLists();

  // copy
  SuspList ** new_susp_lists = (SuspList **)
    heapMalloc(sizeof(SuspList *) * noOfSuspLists);
  for (int i = noOfSuspLists; i--; )
    new_susp_lists[i] = _susp_lists[i];
  _susp_lists = new_susp_lists;
  // collect
  cacStack.pushLocalSuspList(bb, _susp_lists, noOfSuspLists);
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
    cacReallocStatic(OzVariable,this,to,sizeof(SimpleVar));
    break;
  case OZ_VAR_FUTURE:
    cacReallocStatic(OzVariable,this,to,sizeof(Future));
    cacStack.push(to, PTR_CVAR);
    break;
  case OZ_VAR_BOOL:
    cacReallocStatic(OzVariable,this,to,sizeof(OzBoolVariable));
    break;
  case OZ_VAR_OF:
    cacReallocStatic(OzVariable,this,to,sizeof(OzOFVariable));
    cacStack.push(to, PTR_CVAR);
    break;
  case OZ_VAR_FD:
    cacReallocStatic(OzVariable,this,to,sizeof(OzFDVariable));
    ((OzFDVariable *) to)->_cac(bb);
    break;
  case OZ_VAR_FS:
    cacReallocStatic(OzVariable,this,to,sizeof(OzFSVariable));
    ((OzFSVariable *) to)->_cac(bb);
    break;
  case OZ_VAR_CT:
    cacReallocStatic(OzVariable,this,to,sizeof(OzCtVariable));
    ((OzCtVariable*) to)->_cac(bb);
    cacStack.push(to, PTR_CVAR);
    break;
  }

  to->setHome(bb);
  cacStack.push(&(to->suspList), PTR_SUSPLIST);

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
  oz_cacTerm(function,function);
}

inline
void OzVariable::_cacVarRecurse(void) {

  switch (getType()) {
  case OZ_VAR_FUTURE:
    ((Future *)      this)->_cacRecurse();
    break;
  case OZ_VAR_OF:
    ((OzOFVariable*) this)->_cacRecurse();
    break;
  case OZ_VAR_CT:
    ((OzCtVariable*) this)->_cacRecurse();
    break;
  case OZ_VAR_EXT:
    ((ExtVar *)      this)->_cacRecurseV();
    break;
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
  ((OZ_FSetValue *) retval)->copyExtension();
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


inline
LTuple * LTuple::_cac(void) {
  // Does basically nothing, the real stuff is in Recurse

  GCDBG_INFROMSPACE(this);

  if (cacIsMarked())
    return cacGetFwd();

  LTuple * to = (LTuple *) CAC_MALLOC(sizeof(LTuple));

  // Save the content
  to->args[0] = args[0];

  // Do not store foreward! Recurse takes care of this!
  cacMark(to);

  cacStack.push(this, PTR_LTUPLE);

  return to;
}

inline
SRecord *SRecord::_cacSRecord(void) {
  Assert(this);

  if (cacIsMarked())
    return cacGetFwd();

  SRecord * to =
    (SRecord*) CAC_MALLOC((getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord));

  to->recordArity = recordArity;

  STOREFWDFIELD(this, to);

  cacStack.push(this, PTR_SRECORD);

  return to;
}


/*
 * Extension
 *
 */

inline
TaggedRef _cacExtension(TaggedRef term) {
  OZ_Extension *ex = tagged2Extension(term);

  Assert(ex);

  void ** spa = ex->__getSpaceRefInternal();


  // hack alert: write forward into something
  if ((*((int32*)spa))&1) {
    return makeTaggedExtension((OZ_Extension *)ToPointer((*(int32*)spa)&~1));
  }

  Board *bb=(Board*)(*spa);

  if (bb) {
    Assert(bb->cacIsAlive());
    if (!NEEDSCOPYING(bb))
      return term;
  }

  OZ_Extension *ret = ex->_cacV();

  if (bb)
    ret->__setSpaceInternal(bb->_cacBoard());

  cacStack.push(ret,PTR_EXTENSION);

  int32 *fromPtr = (int32*)spa;

  CPTRAIL(fromPtr);

  *fromPtr = ToInt32(ret)|1;

  return makeTaggedExtension(ret);
}

// ===================================================================
// Weak Dictionaries

#ifdef G_COLLECT

// this stack is used to revive the pairs scheduled for
// finalization.  we should not copy them while gc-ing the
// weak dictionaries because we depend on the absence of
// gc marks to tell us that entries need to be finalized.
// thus we must postpone copying the entries to be finalized
// until after all weak dictionaries have been processed.
// it would not be incorrect to copy while processing the
// weak dictionaries, but an unreachable chain of things
// would require several gc in order to be fully finalized.

class WeakReviveStack : public Stack {
public:
  WeakReviveStack() : Stack(64,Stack_WithMalloc) {}
  ~WeakReviveStack() {}
  // we push the pair whose 2 elements are still uncollected
  // actually, it may be that the 1st element (i.e. the key)
  // is already collected.
  void push(OZ_Term pair) {
    Assert(oz_isSRecord(pair));
    Stack::push((StackEntry) pair);
  }
  SRecord* pop() { return tagged2SRecord((OZ_Term) Stack::pop()); }
  void recurse(void);
};

static WeakReviveStack weakReviveStack;

void WeakReviveStack::recurse(void)
{
  SRecord* sr;
  while (!isEmpty()) {
    sr = pop();
    // collect the args of this 2-ary tuple
    OZ_cacBlock(sr->getRef(),sr->getRef(),2);
  }
}

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
inline int isNowMarked(OZ_Term t) {
 redo:
  switch (tagTypeOf(t)) {
  case TAG_REF:
  case TAG_REF2:
  case TAG_REF3:
  case TAG_REF4:
    {
      TaggedRef * ptr;
      do {
        ptr = tagged2Ref(t);
        t   = *ptr;
      } while (oz_isRef(t));
      goto redo;
    }
  case TAG_LITERAL:
    {
      Literal * lit = tagged2Literal(t);
      if (lit->isAtom())
        return 1;
      else
        return ((Name*)lit)->cacIsMarked();
    }
  case TAG_EXT:
    return ((*(int32*) tagged2Extension(t)->__getSpaceRefInternal())&1);
  case TAG_CONST:
    return (tagged2Const(t)->cacIsMarked());
  case TAG_CVAR:
    return tagged2CVar(t)->cacIsMarked();

  case TAG_GCMARK:
    return OK;

  case TAG_SRECORD:
  case TAG_LTUPLE:
  case TAG_FSETVALUE:
  case TAG_FLOAT:
  case TAG_SMALLINT :
    return NO;

  default:
    Assert(0);
    return 0;
  }
}

void WeakDictionary::gCollectRecurseV(void) {
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
        // schedule key and value for later collection
        OZ_Term p = oz_pair2(table->getKey(i),t);
        weakReviveStack.push(p);
        list = oz_cons(p,list);
        count++;
      }
      // set entry to 0 -- it will be skipped when we
      // actually collect the table below
      table->clearValue(i);
    }
  }
  // then schedule the stream for update
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

#endif



/*
 * After collection has finished, update variable references
 *
 */
void VarFix::_cacFix(void) {

  if (isEmpty())
    return;

  do {
    StackEntry e;
    pop1(e);
    TaggedRef * to = (TaggedRef *) e;

    Assert(oz_isRef(*to));

    TaggedRef * aux_ptr = tagged2Ref(*to);
    TaggedRef   aux     = *aux_ptr;

    TaggedRef * to_ptr  =
      (tagTypeOf(aux) == TAG_UVAR) ?
      newTaggedUVar(tagged2VarHome(aux)->derefBoard()->cacGetFwd()) :
      (TaggedRef *) tagged2GcUnmarked(aux);

    Assert(tagTypeOf(aux) == TAG_UVAR || tagTypeOf(aux) == TAG_GCMARK);

    *to = makeTaggedRef(to_ptr);
    STOREFWDMARK(aux_ptr, to_ptr);

  } while (!isEmpty());

}


/*
 * CONST collection
 *
 */

#ifdef G_COLLECT

// failure interface for local tertiarys
#define maybeGCForFailure(t) \
  if (t->getInfo() != NULL) (*gCollectEntityInfo)(t);

#else

#define maybeGCForFailure(t)

#endif



inline
void ConstTerm::_cacConstRecurse(void) {
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;

#ifdef G_COLLECT
      GName * gn = o->getGName1();
      if (gn)
        gCollectGName(gn);
#endif

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

      OZ_cacBlock(&(o->cl1), &(o->cl1), 4);
      break;
    }

  case Co_Space:
    {
      Space *s = (Space *) this;
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
      break;
    }

  case Co_Class:
    {
      ObjectClass *cl = (ObjectClass *) this;
      OZ_cacBlock(&(cl->features), &(cl->features), 4);
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

#ifdef G_COLLECT

inline
ConstTerm *ConstTerm::gCollectConstTermInline(void) {
  Assert(this);

  if (cacIsMarked())
    return cacGetFwd();

  int sz;

  switch (getType()) {

    /*
     * Unsituated types
     *
     */

  case Co_BigInt: {
      ConstTerm * ret = ((BigInt *) this)->gCollect();
      STOREFWDFIELD(this, ret);
      return ret;
    }

  case Co_Foreign_Pointer: {
      ConstTerm * ret;
      cacReallocStatic(ConstTerm,this,ret,sizeof(ForeignPointer));
      STOREFWDFIELD(this, ret);
      return ret;
    }

  case Co_Resource: {
      ConstTerm * ret = (*gCollectDistResource)(this);
      STOREFWDFIELD(this, ret);
      return ret;
    }

  case Co_Builtin:
    return this;

    /*
     * ConstTermWithHome
     *
     */

  case Co_Abstraction:
    sz = ((Abstraction *) this)->getAllocSize();
    goto const_withhome;

  case Co_Chunk:
    sz = sizeof(SChunk);
    goto const_withhome;

  case Co_Array:
    sz = sizeof(OzArray);
    goto const_withhome;

  case Co_Dictionary:
    sz = sizeof(OzDictionary);
    goto const_withhome;

  case Co_Class:
    sz = sizeof(ObjectClass);
    goto const_withhome;

    /*
     * Tertiary
     *
     */

  case Co_Object:
    sz = sizeof(Object);
    goto const_tertiary;

  case Co_Cell:
    sz = (((Tertiary *) this)->getTertType() == Te_Frame) ?
      sizeof(CellFrameEmul) : sizeof(CellManagerEmul);
    goto const_tertiary;

  case Co_Port:
    sz = (((Tertiary *)this)->getTertType() == Te_Proxy) ?
      SIZEOFPORTPROXY : sizeof(PortLocal);
    goto const_tertiary;

  case Co_Space:
    sz = sizeof(Space);
    goto const_tertiary;

  case Co_Lock:
    sz = sizeof(LockLocal);
    goto const_tertiary;

  default:
    Assert(0);
  }

 const_tertiary: {
    Tertiary * t_t = (Tertiary *) oz_hrealloc(this, sz);
    if (t_t->isLocal())
      t_t->setBoardLocal(t_t->getBoardLocal()->gCollectBoard());
    cacStack.push(t_t, PTR_CONSTTERM);
    STOREFWDFIELD(this, t_t);
    return t_t;
  }

 const_withhome: {
   ConstTermWithHome * ctwh_t = (ConstTermWithHome *) oz_hrealloc(this, sz);
   STOREFWDFIELD(this, ctwh_t);
   if (ctwh_t->hasGName())
     gCollectGName(ctwh_t->getGName1());
   else
     ctwh_t->setBoard(ctwh_t->getSubBoardInternal()->gCollectBoard());
   cacStack.push(ctwh_t, PTR_CONSTTERM);
   return ctwh_t;
 }

}

#else

inline
ConstTerm *ConstTerm::sCloneConstTermInline(void) {
  Assert(this);

  if (cacIsMarked())
    return cacGetFwd();

  int sz;

  switch (getType()) {

    /*
     * Unsituated types
     *
     */

  case Co_BigInt:
  case Co_Foreign_Pointer:
  case Co_Resource:
  case Co_Builtin:
    return this;

    /*
     * ConstTermWithHome
     *
     */

  case Co_Abstraction:
    sz = ((Abstraction *) this)->getAllocSize();
    goto const_withhome;

  case Co_Chunk:
    sz = sizeof(SChunk);
    goto const_withhome;

  case Co_Array:
    sz = sizeof(OzArray);
    goto const_withhome;

  case Co_Dictionary:
    sz = sizeof(OzDictionary);
    goto const_withhome;

  case Co_Class:
    sz = sizeof(ObjectClass);
    goto const_withhome;

    /*
     * Tertiary
     *
     */

  case Co_Object:
    sz = sizeof(Object);
    goto const_tertiary;

  case Co_Cell:
    sz = sizeof(CellManagerEmul);
    goto const_tertiary;

  case Co_Port:
    sz = sizeof(PortLocal);
    goto const_tertiary;

  case Co_Space:
    sz = sizeof(Space);
    goto const_tertiary;

  case Co_Lock:
    sz = sizeof(LockLocal);
    goto const_tertiary;

  default:
    Assert(0);
  }

 const_tertiary: {
    Tertiary * t_f = (Tertiary *) this;
    if (!t_f->isLocal())
      return this;
    Board * bb = t_f->getBoardLocal();
    Assert(bb->cacIsAlive());
    if (!NEEDSCOPYING(bb))
      return this;
    Tertiary * t_t = (Tertiary *) oz_hrealloc(this, sz);
    t_t->setBoardLocal(bb->sCloneBoard());
    cacStack.push(t_t, PTR_CONSTTERM);
    STOREFWDFIELD(this, t_t);
    return t_t;
  }

 const_withhome: {
   ConstTermWithHome * ctwh_f = (ConstTermWithHome *) this;
   if (ctwh_f->hasGName())
     return this;
   Board * bb = ctwh_f->getSubBoardInternal();
   Assert(bb->cacIsAlive());
   if (!NEEDSCOPYING(bb))
     return this;
   ConstTermWithHome * ctwh_t = (ConstTermWithHome *) oz_hrealloc(this, sz);
   ctwh_t->setBoard(bb->sCloneBoard());
   cacStack.push(ctwh_t, PTR_CONSTTERM);
   STOREFWDFIELD(this, ctwh_t);
   return ctwh_t;
 }

}

#endif


#ifdef G_COLLECT

inline
OzDebug *OzDebug::gCollectOzDebug(void) {
  OzDebug *ret;
  cacReallocStatic(OzDebug,this,ret,sizeof(OzDebug));

  ret->Y   = gCollectRefsArray(ret->Y);

  OZ_gCollectBlock(&(ret->CAP),&(ret->CAP),2);

  if (ret->arity > 0) {
    ret->arguments = (TaggedRef *)
      heapMalloc(ret->arity * sizeof(TaggedRef));

    OZ_gCollectBlock(arguments, ret->arguments, arity);
  }

  return ret;
}

#endif

TaskStack * TaskStack::_cac(void) {

  TaskStack *newstack = new TaskStack(this);

  Assert(newstack->getUsed() == getUsed());

  Frame * newtop = newstack->tos;

  while (1) {
    ProgramCounter PC    = (ProgramCounter) *(--newtop);
    RefsArray      * Y   = (RefsArray *)      --newtop;
    TaggedRef      * CAP = (TaggedRef *)      --newtop;

#ifdef G_COLLECT
    gCollectCode(PC);
#endif

    if (PC == C_EMPTY_STACK) {
      return newstack;
    } else if (PC == C_CATCH_Ptr) {
    } else if (PC == C_XCONT_Ptr) {
      // mm2: opt: only the top task can/should be xcont!!
      ProgramCounter pc   = (ProgramCounter) *(newtop-1);
#ifdef G_COLLECT
      gCollectCode(pc);
#endif
      (void) CodeArea::livenessX(pc,*Y,getRefsArraySize(*Y));
      *Y = _cacRefsArray(*Y);
    } else if (PC == C_LOCK_Ptr) {
      oz_cacTerm(*CAP, *CAP);
    } else if (PC == C_SET_SELF_Ptr) {
      oz_cacTerm(*CAP, *CAP);
    } else if (PC == C_SET_ABSTR_Ptr) {
      ;
    } else if (PC == C_DEBUG_CONT_Ptr) {
#ifdef G_COLLECT
      *Y = (RefsArray) ((OzDebug *) *Y)->gCollectOzDebug();
#else
      Assert(0);
#endif
    } else if (PC == C_CALL_CONT_Ptr) {
      oz_cacTerm(*((TaggedRef *) Y), *((TaggedRef *) Y));
      *CAP = makeTaggedMiscp((void *)
                             _cacRefsArray((RefsArray)
                                           tagValueOf(*CAP)));
    } else { // usual continuation
      *Y   = _cacRefsArray(*Y);
      oz_cacTerm(*CAP, *CAP);
    }

  }

}

inline
void Thread::_cacRecurse(Thread * fr) {
  taskStack = fr->taskStack->_cac();
  abstr     = fr->abstr;
  id        = fr->id;
}

inline
void Propagator::_cacRecurse(Propagator * fr) {
  _p = (OZ_Propagator *) oz_hrealloc(fr->_p, fr->_p->sizeOf());
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


  } else {
    return NULL;
  }

  to->flags = flags;
  STOREFWDFIELD(this, to);

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

  STOREFWDFIELD(this, to);

  Assert(to->isPropagator());

  return to;
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

  CPTRAIL((int32 *) last->getNextRef());

  last->setNext(NULL);

  head = head->_cacRecurse(&last);

  last->setNext(head);

}

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

  OZ_cacBlock(&script,&script,3);

  cacStack.push(&suspList, PTR_SUSPLIST);
  setDistBag(getDistBag()->_cac());
  cacStack.push((SuspList **) &nonMonoSuspList, PTR_SUSPLIST);

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
  SRecord * to = cacGetFwd();

  OZ_cacBlock(&label, &(to->label), to->getWidth()+1);

}


inline
void LTuple::_cacRecurse() {
  LTuple * frm = this;
  LTuple * to  = frm->cacGetFwd();
  TaggedRef aux = oz_deref(to->args[0]);

  //
  if (!oz_isLTuple(aux) || tagged2LTuple(aux) != this) {
    frm->args[0] = to->args[0];
    oz_cacTerm(frm->args[0], to->args[0]);
    STOREFWDFIELD(frm, to);
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

    if (frm->cacIsMarked()) {
      to->args[1] = makeTaggedLTuple(frm->cacGetFwd());
      return;
    }

    LTuple * next = (LTuple *) CAC_MALLOC(sizeof(LTuple));

    to->args[1] = makeTaggedLTuple(next);
    to = next;

    oz_cacTerm(frm->args[0], to->args[0]);

    STOREFWDFIELD(frm, to);

  }

  Assert(0);
}


void CacStack::_cacRecurse(void) {

  while (!isEmpty()) {
    StackEntry tp;
    pop1(tp);
    void * ptr    = tagValueOf((TaggedRef) tp);
    TypeOfPtr how = (TypeOfPtr) tagTypeOf((TaggedRef) tp);

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
        StackEntry e;
        pop1(e);
        Board    *  bb = (Board *) e;

        for (int i = how - PTR_LOCAL_SUSPLIST; i--; )
          sl[i] = sl[i]->_cacLocalRecurse(bb);

      }
    }
  }
}


/*
 * Entry points into garbage collection
 *
 */


void OZ_cacBlock(OZ_Term * frm, OZ_Term * to, int sz) {

  register TaggedRef * f = frm - 1;
  register TaggedRef * t = to - 1;

  TaggedRef aux, *aux_ptr;

  while (sz > 0) {
    sz--; f++; t++;
    aux = *f;

    switch (tagTypeOf(aux)) {
    case TAG_REF:
      if (aux)
        goto DO_DEREF;
      *t = makeTaggedNULL();
      continue;
    case TAG_REF2:       goto DO_DEREF;
    case TAG_REF3:       goto DO_DEREF;
    case TAG_REF4:       goto DO_DEREF;
    case TAG_GCMARK:     goto DO_GCMARK;
    case TAG_SMALLINT:   goto DO_SMALLINT;
    case TAG_FSETVALUE:  goto DO_FSETVALUE;
    case TAG_LITERAL:    goto DO_LITERAL;
    case TAG_EXT:        goto DO_EXT;
    case TAG_LTUPLE:     goto DO_LTUPLE;
    case TAG_SRECORD:    goto DO_SRECORD;
    case TAG_FLOAT:      goto DO_FLOAT;
    case TAG_CONST:      goto DO_CONST;
    case TAG_UNUSED:     goto DO_UNUSED;
    case TAG_UVAR:       goto DO_D_UVAR;
    case TAG_CVAR:       goto DO_D_CVAR;
    }

  DO_DEREF:
    aux_ptr = tagged2Ref(aux);
    aux     = *aux_ptr;

    switch (tagTypeOf(aux)) {
    case TAG_REF:        goto DO_DEREF;
    case TAG_REF2:       goto DO_DEREF;
    case TAG_REF3:       goto DO_DEREF;
    case TAG_REF4:       goto DO_DEREF;
    case TAG_GCMARK:     goto DO_GCMARK;
    case TAG_SMALLINT:   goto DO_SMALLINT;
    case TAG_FSETVALUE:  goto DO_FSETVALUE;
    case TAG_LITERAL:    goto DO_LITERAL;
    case TAG_EXT:        goto DO_EXT;
    case TAG_LTUPLE:     goto DO_LTUPLE;
    case TAG_SRECORD:    goto DO_SRECORD;
    case TAG_FLOAT:      goto DO_FLOAT;
    case TAG_CONST:      goto DO_CONST;
    case TAG_UNUSED:     goto DO_UNUSED;
    case TAG_UVAR:       goto DO_I_UVAR;
    case TAG_CVAR:       goto DO_I_CVAR;
    }

  DO_GCMARK:
    *t = makeTaggedRef((TaggedRef*) tagged2GcUnmarked(aux));
    continue;

  DO_FSETVALUE:
#ifdef G_COLLECT
    *t = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(aux))->gCollect());
    continue;
#endif

  DO_FLOAT:
#ifdef G_COLLECT
    *t = makeTaggedFloat(tagged2Float(aux)->gCollect());
    continue;
#endif

  DO_SMALLINT:
    *t = aux;
    continue;

  DO_LITERAL:
     *t = makeTaggedLiteral(tagged2Literal(aux)->_cac());
     continue;

  DO_EXT:
     *t = _cacExtension(aux);
     continue;

  DO_LTUPLE:
     *t = makeTaggedLTuple(tagged2LTuple(aux)->_cac());
     continue;

  DO_SRECORD:
     *t = makeTaggedSRecord(tagged2SRecord(aux)->_cacSRecord());
     continue;

  DO_CONST:
     *t = makeTaggedConst(tagged2Const(aux)->_cacConstTermInline());
     continue;

  DO_UNUSED:
     Assert(0);
     continue;

  DO_I_UVAR:
     {
       Board * bb = tagged2VarHome(aux);

       if (NEEDSCOPYING(bb)) {
         bb = bb->_cacBoard();
         Assert(bb);
         vf.defer(aux_ptr, t);
       } else {
         *t = makeTaggedRef(aux_ptr);
       }
     }
     continue;

  DO_D_UVAR:
     {
       Board * bb = tagged2VarHome(aux);

       if (NEEDSCOPYING(bb)) {
         bb = bb->_cacBoard();
         Assert(bb);
         *t = makeTaggedUVar(bb);
         STOREFWDMARK(f, t);
       } else {
         *f = makeTaggedRef(t);
         *t = aux;
         STOREFWDMARK(f, t);
       }
     }
     continue;

     OzVariable * cv;
     TaggedRef  * var_ptr;

  DO_I_CVAR:
     cv = tagged2CVar(aux);

     if (cv->cacIsMarked()) {
       Assert(oz_isCVar(*(cv->cacGetFwd())));
       *t = makeTaggedRef(cv->cacGetFwd());
       continue;
     }

     if (!NEEDSCOPYING(cv->getBoardInternal())) {
       *t = makeTaggedRef(aux_ptr);
       continue;
     }

     var_ptr = (TaggedRef *) heapMalloc(sizeof(TaggedRef));
     *t = makeTaggedRef(var_ptr);

  DO_CVAR:
     *var_ptr = makeTaggedCVar(cv->_cacVarInline());
     cv->_cacMark(var_ptr);
     continue;

  DO_D_CVAR:
     cv = tagged2CVar(aux);

     if (cv->cacIsMarked()) {
       Assert(oz_isCVar(*(cv->cacGetFwd())));
       *t = makeTaggedRef(cv->cacGetFwd());
       continue;
     }

     if (!NEEDSCOPYING(cv->getBoardInternal())) {
       // We cannot copy the variable, but we have already copied
       // their taggedref, so we change the original variable to a ref
       // of the copy.
       // After pushing on the update stack the
       // the original variable is replaced by a reference!
       *f = makeTaggedRef(t);
       *t = aux;
       STOREFWDMARK(f, t);
       continue;
     }

     var_ptr = t;
     goto DO_CVAR;

  }

}

Suspendable * Suspendable::_cacSuspendable(void) {
  return (this == NULL) ? (Suspendable *) NULL : _cacSuspendableInline();
}

OZ_Term * OZ_cacAllocBlock(int n, OZ_Term * frm) {
  if (n==0)
    return (OZ_Term *) NULL;

  OZ_Term * to = (OZ_Term *) CAC_MALLOC(n * sizeof(OZ_Term));

  OZ_cacBlock(frm, to, n);

  return to;
}
