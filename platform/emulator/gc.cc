/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte (schulte@dfki.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

/****************************************************************************
 ****************************************************************************/


#ifdef LINUX
/* FD_ISSET missing */
#include <sys/time.h>
#endif

#include "gc.hh"

#include "runtime.hh"
#include "perdio.hh"

#include "tcl_tk.hh"

#include "genvar.hh"

#include "fdhook.hh"
#include "fdprofil.hh"

#include "fdomn.hh"

#include "dictionary.hh"

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
 *               Debug
 *
 */




/*
 * CHECKSPACE -- check if object is really copied from heap
 *   has as set of macros:
 *    INITCHECKSPACE - save pointer to from-space & print from-space
 *    NOTINTOSPACE   - assert not in to-space
 *    INTOSPACE      - assert in to-space
 * NOTE: this works only for chunk
 */

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

#ifdef DEBUG_GC

#define INFROMSPACE(P)  Assert(inFromSpace(P))
#define NOTINTOSPACE(P) Assert(notInToSpace(P))
#define INTOSPACE(P)    Assert(inToSpace(P))

#else

#define INFROMSPACE(P)
#define NOTINTOSPACE(P)
#define INTOSPACE(P)

#endif


/**************************************************
 *  Invalidating of inline caches
 **************************************************/

const int inlineCacheListBlockSize = 100;

class InlineCacheList {
  InlineCacheList *next;
  int nextFree;
  InlineCache *block[inlineCacheListBlockSize];

public:
  InlineCacheList(InlineCacheList *nxt) { nextFree=0; next=nxt; }

  InlineCacheList *add(InlineCache *ptr)
  {
    if (nextFree < inlineCacheListBlockSize) {
      block[nextFree] = ptr;
      nextFree++;
      return this;
    } else {
      InlineCacheList *aux = new InlineCacheList(this);
      return aux->add(ptr);
    }
  }

  void cacheListGC()
  {
    InlineCacheList *aux = this;
    while(aux) {
      for (int i=aux->nextFree; i--; ) {
        aux->block[i]->invalidate();
      }
      aux = aux->next;
    }
  }
};


static InlineCacheList *cacheList = new InlineCacheList(NULL);

void protectInlineCache(InlineCache *cache)
{
  cacheList = cacheList->add(cache);
}


/**************************************************
 *  Dumping of threads
 **************************************************/

class ThreadList {
public:
  static ThreadList *allthreads;
  ThreadList *next;
  Thread *elem;
  ThreadList(Thread *el, ThreadList *nxt): next(nxt), elem(el) {};

  static void add(Thread *t)
  {
    allthreads = new ThreadList(t,allthreads);
  }

  static void dispose()
  {
    ThreadList *aux = allthreads;
    allthreads = NULL;
    while(aux) {
      ThreadList *aux1 = aux;
      aux = aux->next;
      delete aux1;
    }
  }

  static void print()
  {
    for (ThreadList *aux = allthreads; aux; aux=aux->next) {
      char *s;
      if (aux->elem->isDeadThread())
        s = "terminated";
      else if (aux->elem->isRunnable())
        s = "ready";
      else
        s = "blocked";
      message("Thread: id = %d, parent = %d, state: %s\n",
              aux->elem->getID() & THREAD_ID_MASK,
              (aux->elem->getID() >> THREAD_ID_SIZE) & THREAD_ID_MASK,
              s);
      //message("---------------------------------------------\n");
      aux->elem->printTaskStack(ozconf.errorThreadDepth);
    }
  }

  static OZ_Term list()
  {
    OZ_Term out = OZ_nil();

    for (ThreadList *aux = allthreads; aux; aux=aux->next) {
      out = OZ_cons(makeTaggedConst(aux->elem),
                    out);
    }
    return out;
  }

};

ThreadList *ThreadList::allthreads = NULL;

OZ_C_proc_begin(BIlistThreads, 1)
{
  return (OZ_unify(OZ_getCArg(0), ThreadList::list()));
}
OZ_C_proc_end

OZ_C_proc_begin(BIdumpThreads, 0)
{
  ThreadList::print();
  return PROCEED;
}
OZ_C_proc_end


/*
 * Allocate and copy memory blocks.
 *
 */

inline
void * gcReallocStatic(void * p, size_t sz) {
  // Use for blocks where size is known statically at compile time
  DebugCheck(sz%sizeof(int) != 0,
             error("gcReallocStatic: can only handle word sized blocks"););

  int32 * frm = (int32 *) p;
  int32 * to  = (int32 *) heapMalloc(sz);

  switch(sz) {
  case 64: to[15] = frm[15];
  case 60: to[14] = frm[14];
  case 56: to[13] = frm[13];
  case 52: to[12] = frm[12];
  case 48: to[11] = frm[11];
  case 44: to[10] = frm[10];
  case 40: to[9]  = frm[9];
  case 36: to[8]  = frm[8];
  case 32: to[7]  = frm[7];
  case 28: to[6]  = frm[6];
  case 24: to[5]  = frm[5];
  case 20: to[4]  = frm[4];
  case 16: to[3]  = frm[3];
  case 12: to[2]  = frm[2];
  case  8: to[1]  = frm[1];
  case  4: to[0]  = frm[0];
    break;
#ifdef DEBUG_CHECK
  default:
    if (sz > 64)
      { Assert(0); };
#endif
  }
  return to;
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
  PTR_NAME,
  PTR_BOARD,
  PTR_ACTOR,
  PTR_THREAD,
  PTR_CONT,
  PTR_PROPAGATOR,
  PTR_DYNTAB,
  PTR_CONSTTERM
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
void storeFwd (int32* fromPtr, void *newValue, Bool domark=OK) {
  if (!isInGc)
    cpTrail.save(fromPtr);

  Assert(inFromSpace(fromPtr));

  *fromPtr = domark ? GCMARK(newValue) : ToInt32(newValue);
}

inline
void storeFwd(void* fromPtr, void *newValue) {
  storeFwd((int32*) fromPtr, newValue);
}


#define storeFwdField(t) \
  storeFwd((int32*) this->gcGetMarkField(), t, NO); this->gcMark(t);

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


inline
Bool needsNoCollection(TaggedRef t)
{
  Assert(t != makeTaggedNULL());

  TypeOfTerm tag = tagTypeOf(t);
  return isSmallInt(tag) ||
         isLiteral(tag) && !needsCollection(tagged2Literal(t));
}


Bool gcProtect(TaggedRef *ref)
{
  extRefs = extRefs->protect(ref);
  return OK;
}

/* protect a ref, that will never change its initial value
 *  --> no need to remember it, if it's a small int or atom
 */
Bool gcStaticProtect(TaggedRef *ref)
{
  if (needsNoCollection(*ref))
    return OK;

  return gcProtect(ref);
}

Bool gcUnprotect(TaggedRef *ref)
{
  ExtRefNode *aux = extRefs->find(ref);

  if (aux == NULL)
    return NO;

  aux->remove();
  return OK;
}


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
    // Do not use makeTaggedRef, because this expects references
    // to to-space!
    *ref = ToInt32(var);
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
 * Copying: groundness check needs to find whether situated entity
 * (i.e., variables, cells, procedures, ...) have been copied
 *
 */
static Bool isGround;

/*
 * Copying: before copying all spaces but the space to be copied get marked.
 */
void Board::setGlobalMarks(void) {
  Assert(!isInGc);
  Assert(!_isRoot());

  Board * b = this;

  do {
    b = b->getParent(); b->setGlobalMark();
  } while (!b->_isRoot());

}

/*
 * Copying: purge marks after copying
 */
void Board::unsetGlobalMarks(void) {
  Assert(!isInGc);
  Assert(!_isRoot());

  Board * b = this;

  do {
    b = b->getParent(); b->unsetGlobalMark();
  } while (!b->_isRoot());

}


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

  NOTINTOSPACE(r);

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

//
//  ... Continuation;
inline
void Continuation::gcRecurse () {
  yRegs = gcRefsArray(yRegs);
  gRegs = gcRefsArray(gRegs);
  xRegs = gcRefsArray(xRegs);
}

inline
Bool Continuation::gcIsMarked(void) {
  return GCISMARKED((TaggedRef) pc);
}

inline
void Continuation::gcMark(Continuation * fwd) {
  pc = (ProgramCounter) GCMARK(fwd);
}

inline
void ** Continuation::gcGetMarkField(void) {
  return (void **) &pc;
}

inline
Continuation * Continuation::gcGetFwd(void) {
  Assert(gcIsMarked());
  return (Continuation *) GCUNMARK((int32) pc);
}

Continuation *Continuation::gc() {
  if (gcIsMarked())
    return gcGetFwd();

  Continuation *ret =
    (Continuation *) gcReallocStatic(this, sizeof(Continuation));

  gcStack.push (ret, PTR_CONT);

  storeFwdField(ret);

  return (ret);
}

inline
Bool Board::gcIsMarked(void) {
  return body.gcIsMarked();
}

inline
void Board::gcMark(Board * fwd) {
  body.gcMark((Continuation *) fwd);
}

inline
void ** Board::gcGetMarkField(void) {
  return body.gcGetMarkField();
}

inline
Board * Board::gcGetFwd(void) {
  return (Board *) body.gcGetFwd();
}

Board * Board::gcBoard() {
  INFROMSPACE(this);

  if (!this) return 0;

  Board * bb = derefBoard();

  Assert(bb);

  if (bb->gcIsMarked())
    return bb->gcGetFwd();

  if (!bb->gcIsAlive())
    return 0;

  Assert(isInGc || bb->isInTree());

  Assert(!isInGc || !inToSpace(bb));

  Board *ret = (Board *) gcReallocStatic(bb, sizeof(Board));

  gcStack.push(ret,PTR_BOARD);

  storeFwdField(ret);

  return ret;
}



/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

void dogcGName(GName *gn) {
  if (gn && isInGc)
    gcGName(gn);
}


Name *Name::gcName() {
  CHECKCOLLECTED(homeOrGName, Name *);
  GName *gn = NULL;

  if (hasGName()) {
    gn = getGName();
  }

  if (isInGc && isOnHeap() ||
      !isInGc && !(GETBOARD(this))->isMarkedGlobal()) {

    isGround = NO;
    Name *aux = (Name*) gcReallocStatic(this,sizeof(Name));

    gcStack.push(aux, PTR_NAME);
    storeFwd(&homeOrGName, aux);

    dogcGName(gn);
    return aux;
  } else {
    dogcGName(gn);
    return this;
  }
}

inline
Literal *Literal::gc() {
  Assert(needsCollection(this));

  Assert(isName());
  return ((Name*) this)->gcName();
}

inline
void Name::gcRecurse() {
  if (hasGName())
    return;

  homeOrGName = ToInt32(((Board*)ToPointer(homeOrGName))->gcBoard());
}

Object *Object::gcObject() {
  return (Object *) gcConstTerm();
}

/*
 *  We reverse the order of the list, but this should be no problem.
 *
 * kost@ : ... in any case, this is complaint with the
 * 'The Definition of Kernel Oz';
 *
 */
inline
SuspList * SuspList::gc() {
  SuspList *ret = NULL;

  for (SuspList* help = this; help != NULL; help = help->next) {
    Thread *aux = (help->getElem ())->gcThread ();
    if (!aux) {
      continue;
    }
    ret = new SuspList(aux, ret);

  }

  return (ret);
}




inline
Bool SVariable::gcIsMarked(void) {
  return IsMarkedPointer(suspList);
}

inline
void SVariable::gcMark(SVariable * fwd) {
  suspList = (SuspList *) MarkPointer(fwd);
}

inline
void ** SVariable::gcGetMarkField(void) {
  Assert(!gcIsMarked());
  return (void **) &suspList;
}

inline
SVariable * SVariable::gcGetFwd(void) {
  return (SVariable *) UnMarkPointer(suspList);
}

inline
SVariable * SVariable::gc() {
  Assert(!gcIsMarked())

  Board * bb = home->gcBoard();

  Assert(bb);

  isGround = NO;

  SVariable * to = (SVariable *) heapMalloc(sizeof(SVariable));

  Assert(!isInGc || to->home != bb);

  to->suspList = suspList->gc();
  to->home     = bb;

  storeFwdField(to);


  return to;
}


inline
void OZ_FiniteDomainImpl::gc(void)
{

  copyExtension();
}

inline
void GenFDVariable::gc(GenFDVariable * frm) {
  finiteDomain = frm->finiteDomain;
  ((OZ_FiniteDomainImpl *) &finiteDomain)->gc();

  int i;
  for (i = fd_prop_any; i--; )
    fdSuspList[i] = frm->fdSuspList[i]->gc();
}

inline
void GenBoolVariable::gc(GenBoolVariable * frm) {
  store_patch = frm->store_patch;
}

inline
void GenFSetVariable::gc() {
  int i;
  for (i = fs_prop_any; i--; )
    fsSuspList[i] = fsSuspList[i]->gc();
}

inline
FSetValue * FSetValue::gc(void)
{
  return (FSetValue *) OZ_hrealloc(this, sizeof(FSetValue));
}


void GenLazyVariable::gc(void) {
  if (function!=0) {
    OZ_collectHeapTerm(function,function);
    OZ_collectHeapTerm(result,result);
  }
}

inline
void GenMetaVariable::gc(void) {
  OZ_collectHeapTerm(data,data);
}

inline
void AVar::gc(void) {
  OZ_collectHeapTerm(value,value);
}

inline
DynamicTable* DynamicTable::gc(void) {
  Assert(isPwrTwo(size));

  // Copy the table:

  size_t len = (size-1)*sizeof(HashElement) + sizeof(DynamicTable);

  DynamicTable* ret = (DynamicTable*) heapMalloc(len);

  ret->numelem = numelem;
  ret->size    = size;

  // Leave a mark where gcRecurse finds the already allocated heap mem
  numelem = (dt_index) ToInt32(ret);

  // Take care of all TaggedRefs in the table:
  gcStack.push(this, PTR_DYNTAB);
  // (no storeFwd needed since only one place points to the dynamictable)

  return ret;
}

inline
void DynamicTable::gcRecurse() {
  DynamicTable * to = (DynamicTable *) ToPointer(numelem);

  OZ_collectHeapBlock((TaggedRef *) table, (TaggedRef *) to->table, size * 2);

}



void GenOFSVariable::gc(void) {
  OZ_collectHeapTerm(label,label);
  // Update the pointer in the copied block:
  dynamictable=dynamictable->gc();
}

inline
Bool GenCVariable::gcIsMarked(void) {
  return IsMarkedPointer(suspList);
}

inline
void ** GenCVariable::gcGetMarkField(void) {
  Assert(!gcIsMarked());
  return (void **) &suspList;
}

inline
void GenCVariable::gcMark(GenCVariable * fwd) {
  suspList = (SuspList *) MarkPointer(fwd);
}

inline
GenCVariable * GenCVariable::gcGetFwd(void) {
  return (GenCVariable *) UnMarkPointer(suspList);
}

inline
GenCVariable * GenCVariable::gc(void) {
  INFROMSPACE(this);

  Assert(!gcIsMarked())

  Board * bb = home->gcBoard();

  Assert(bb);

  SuspList * sl = suspList;

  isGround = NO;

  GenCVariable * to;

  switch (getType()){
  case FDVariable:
    to = (GenCVariable *) heapMalloc(sizeof(GenFDVariable));
    to->u = this->u;
    storeFwdField(to);
    ((GenFDVariable *) to)->gc((GenFDVariable *) this);
    break;
  case BoolVariable:
    to = (GenCVariable *) heapMalloc(sizeof(GenBoolVariable));
    to->u = this->u;
    storeFwdField(to);
    ((GenBoolVariable *) to)->gc((GenBoolVariable *) this);
    break;
  case OFSVariable:
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(GenOFSVariable));
    storeFwdField(to);
    ((GenOFSVariable *) to)->gc();
    break;
  case MetaVariable:
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(GenMetaVariable));
    storeFwdField(to);
    ((GenMetaVariable *) to)->gc();
    break;
  case AVAR:
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(AVar));
    storeFwdField(to);
    ((AVar *) to)->gc();
    break;
  case PerdioVariable:
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(PerdioVar));
    storeFwdField(to);
    ((PerdioVar *) to)->gc();
    break;
  case FSetVariable:
    // TMUELLER: must be reset to `gcReallocStatic'
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(GenFSetVariable));
    storeFwdField(to);
    ((GenFSetVariable *) to)->gc();
    break;
  case LazyVariable:
    to = (GenCVariable *) OZ_hrealloc(this, sizeof(GenLazyVariable));
    storeFwdField(to);
    ((GenLazyVariable*) to)->gc();
    break;
  default:
    Assert(0);
  }

  Assert(!isInGc || this->home != bb);

  to->suspList = sl->gc();
  to->home     = bb;

  return to;
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


BigInt * BigInt::gc() {
  Assert(isInGc);

  CHECKCOLLECTED(*(int *)&value, BigInt *);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  storeFwd((int *)&value, ret);
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
      if (!isInGc && IsRef(auxTerm)) {
        do {
          if (auxTerm == makeTaggedNULL ())
            error ("NULL in script");
          if (GCISMARKED(auxTerm)) {
            auxTerm = ToInt32(GCUNMARK(auxTerm));
            continue;
          }
          if (isRef (auxTerm)) {
            auxTermPtr = tagged2Ref (auxTerm);
            auxTerm = *auxTermPtr;
            continue;
          }
          if (isAnyVar (auxTerm))
            break;   // ok;
          error ("non-variable is found at left side in Script");
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


inline
ChachedOORegs gcChachedOORegs(ChachedOORegs regs) {
  Object *o = getObject(regs)->gcObject();
  return setObject(regs,o);
}

RunnableThreadBody *RunnableThreadBody::gcRTBody () {
  RunnableThreadBody *ret =
    (RunnableThreadBody *) gcReallocStatic(this, sizeof(RunnableThreadBody));

  taskStack.gc(&ret->taskStack);

  return (ret);
}

OZ_Propagator * OZ_Propagator::gc(void) {
  OZ_Propagator * p = (OZ_Propagator *) OZ_hrealloc(this, sizeOf());

  gcStack.push(p, PTR_PROPAGATOR);

  return p;
}

/* collect LTuple, SRecord */


LTuple * LTuple::gc() {
  // Does basically nothing, the real stuff is in gcRecurse

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


inline
Thread* isCollected(Thread *t,void *p)
{
  CHECKCOLLECTED(ToInt32(p), Thread*);
  return NULL;
}


/*
 *  Preserve runnable threads which home board is dead, because
 * solve counters have to be updated (while, of course, discard
 * non-runnable ones);
 *  If threads is dead, returns (Thread *) NULL;
 */

Thread *Thread::gcThread () {
  if (this == (Thread *) NULL)
    return ((Thread *) NULL);

  Thread *ret = isCollected(this,item.threadBody);
  if (ret) return ret;

  if (isDeadThread()) return ((Thread *) NULL);

  //  Some invariants:
  // nothing can be copied (IN_TC) until stability;

  // first class threads: must only copy thread when local to solve!!!
  if (!isInGc && (GETBOARD(this))->isMarkedGlobal())
    return this;

  Assert(isInGc || !isRunnable());

  //
  //  Note that runnable threads can be also counted
  // in solve actors (for stability check), and, therefore,
  // might not just dissappear!
  if (isSuspended() && !GETBOARD(this)->gcIsAlive()) {
    return ((Thread *) NULL);
  }

  Thread *newThread = (Thread *) gcReallocStatic(this, sizeof(Thread));

  if (isRunnable() || hasStack()) {
    ThreadList::add(newThread);
  }

  gcStack.push(newThread, PTR_THREAD);

  storeFwd(&item.threadBody, newThread);

  return newThread;
}

inline
Abstraction *gcAbstraction(Abstraction *a)
{
  return (Abstraction *) a->gcConstTerm();
}


Thread *Thread::gcDeadThread() {
  Assert(isDeadThread());

  Thread *newThread = (Thread *) gcReallocStatic(this,sizeof(Thread));

  Assert(inToSpace(am.rootBoardGC()));
  newThread->setBoard(am.rootBoardGC());
  //  newThread->state.flags=0;
  Assert(newThread->item.threadBody==NULL);

  storeFwd (&item.threadBody, newThread);
  setSelf(getSelf()->gcObject());
  getAbstr()->gcPrTabEntry();

  return (newThread);
}

inline
void Thread::gcRecurse () {

  if (isLocal()) {
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
      setBoard(notificationBoard->gcBoard());

      GETBOARD(this)->incSuspCount ();

      //
      //  Convert the thread to a 'wakeup' type, and just throw away
      // the body;
      setWakeUpTypeGC ();
      item.threadBody = (RunnableThreadBody *) NULL;
      return;
    } else {
      setBoard(newBoard);
    }
  }

  //
  switch (getThrType ()) {
  case S_RTHREAD:
    item.threadBody = item.threadBody->gcRTBody ();
    break;

  case S_WAKEUP:
    //  should not contain any reference;
    Assert(item.threadBody == (RunnableThreadBody *) NULL);
    break;

  case S_PR_THR:
    item.propagator = item.propagator->gc();
    Assert(item.propagator);
    break;

  default:
    Assert(0);
  }

  setSelf(getSelf()->gcObject());
  getAbstr()->gcPrTabEntry();

  gcTertiary();
}


#ifdef FOREIGN_POINTER
ForeignPointer * ForeignPointer::gc(void) {
  ForeignPointer * ret =
    (ForeignPointer*) gcReallocStatic(this,sizeof(ForeignPointer));
  ret->ptr = ptr;

  storeFwdField(ret);
  return ret;
}
#endif

// ===================================================================
// Finalization

#ifdef FINALIZATION

extern OZ_Term guardian_list;
extern OZ_Term finalize_list;
extern OZ_Term finalize_handler;

void gc_finalize()
{
  static RefsArray args = allocateStaticRefsArray(1);
  // go through the old guardian list
  OZ_Term old_guardian_list = guardian_list;
  guardian_list = finalize_list = oz_nil();
  if (old_guardian_list==0) return;
  while (!isNil(old_guardian_list)) {
    OZ_Term pair = head(old_guardian_list);
    old_guardian_list = tail(old_guardian_list);
    OZ_Term obj = head(pair);
    if (tagged2Const(obj)->gcIsMarked())
      // reachable through live data
      guardian_list = oz_cons(pair,guardian_list);
    else
      // unreachable
      finalize_list = oz_cons(pair,finalize_list);
  }
  // gc both these list normally.
  // since these lists have been freshly consed in the new half space
  // this simply means to go through both and gc the pairs
  // in the head of each cons
  for (OZ_Term l=guardian_list;!isNil(l);l=tail(l)) {
    LTuple *t = tagged2LTuple(l);
    OZ_collectHeapTerm(*t->getRefHead(),*t->getRefHead());
  }
  for (OZ_Term l1=finalize_list;!isNil(l1);l1=tail(l1)) {
    LTuple *t = tagged2LTuple(l1);
    OZ_collectHeapTerm(*t->getRefHead(),*t->getRefHead());
  }
  // if the finalize_list is not empty, we must create a new
  // thread (at top level) to effect the finalization phase
  if (!isNil(finalize_list)) {
    Thread* thr = am.mkRunnableThread(DEFAULT_PRIORITY,ozx_rootBoard());
    args[0] = finalize_list;
    thr->pushCall(finalize_handler,args,1);
    am.scheduleThread(thr);
    finalize_list = oz_nil();
  }
}
#endif



inline
void gcTagged(TaggedRef & frm, TaggedRef & to) {
  Assert(!isInGc || !fromSpace->inChunkChain(&to));

  TaggedRef aux       = frm;
  TaggedRef * aux_ptr = NULL;

update:

  switch (tagTypeOf(aux)) {

  case REFTAG1:
    /* initalized but unused cell in register array */
    if (aux == makeTaggedNULL()) {
      to = aux;
      return;
    }

  case REFTAG2:
  case REFTAG3:
  case REFTAG4:

    do {
      aux_ptr = tagged2Ref(aux);
      aux     = *aux_ptr;
    } while (IsRef(aux));

    goto update;

  case GCTAG:
    to = makeTaggedRef((TaggedRef*) GCUNMARK(aux));
    // mm2: this can lead to not shortened ref chains together with
    // the CONS forwarding: if a CONS cell is collected then every
    // reference to the first element becomes a ref. May try this:
    // if (!isVar(*to)) to=deref(to);
    return;

  case SMALLINT:
    to = aux;
    return;

  case FSETVALUE:
    if (isInGc) {
      to = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(aux))->gc());
    } else {
      to = aux;
    }
    return;

  case LITERAL:
    {
      Literal * l = tagged2Literal(aux);

      if (needsCollection(l)) {
        to = makeTaggedLiteral(l->gc());
      } else {
        to = aux;
      }
      return;
    }

  case LTUPLE:
    to = makeTaggedLTuple(tagged2LTuple(aux)->gc());
    return;

  case SRECORD:
    to = makeTaggedSRecord(tagged2SRecord(aux)->gcSRecord());
    return;

  case BIGINT:
    if (isInGc) {
      to = makeTaggedBigInt(tagged2BigInt(aux)->gc());
    } else {
      to = aux;
    }
    return;

  case OZFLOAT:
    if (isInGc) {
      to = makeTaggedFloat(tagged2Float(aux)->gc());
    } else {
      to = aux;
    }
    return;

  case OZCONST:
    to = makeTaggedConst(tagged2Const(aux)->gcConstTerm());
    return;

  case SVAR:
    {
      SVariable * sv = tagged2SVar(aux);

      if (sv->gcIsMarked()) {
        if (aux_ptr) {
          // This means the variable is not direct!
          varFix.defer(aux_ptr, &to);
        } else {
          to = makeTaggedSVar(sv->gcGetFwd());
          storeFwd(&frm, &to);
        }
        return;
      }

      if (isInGc || !(GETBOARD(sv))->isMarkedGlobal()) {
        SVariable * sv_gc = sv->gc();

        if (aux_ptr) {
          // This means the variable is not direct!
          varFix.defer(aux_ptr, &to);
        } else {
          to = makeTaggedSVar(sv_gc);
          storeFwd(&frm, &to);
        }

        return;
      }

      break;

    }

  case CVAR:
    {
      GenCVariable * cv = tagged2CVar(aux);

      if (cv->gcIsMarked()) {
        if (aux_ptr) {
          // This means the variable is not direct!
          varFix.defer(aux_ptr, &to);
        } else {
          to = makeTaggedSVar(cv->gcGetFwd());
          storeFwd(&frm, &to);
        }
        return;
      }

      if (isInGc || !(GETBOARD(cv))->isMarkedGlobal()) {
        GenCVariable * cv_gc = cv->gc();

        if (aux_ptr) {
          // This means the variable is not direct!
          varFix.defer(aux_ptr, &to);
        } else {
          Assert(isDirectVar(frm));
          to = makeTaggedCVar(cv_gc);
          storeFwd(&frm, &to);
        }

        return;
      }

      break;
    }

  case UVAR:
    {
      Board * bb = tagged2VarHome(aux)->derefBoard();

      Assert(bb);

      if (isInGc || !bb->isMarkedGlobal()) {

        bb = bb->gcBoard();

        isGround = NO;

        if (aux_ptr) {
          varFix.defer(aux_ptr, &to);
        } else {
          Assert(isDirectVar(frm));
          to = makeTaggedUVar(bb);
          storeFwd(&frm, &to);
        }

        return;
      }

      break;
    }
  }

  Assert(isUVar(aux) || isCVar(aux) || isSVar(aux));

  if (aux_ptr) {
    to = makeTaggedRef(aux_ptr);
  } else {
    Assert(isDirectVar(frm));
    // We cannot copy the variable, but we have already copied
    // their taggedref, so we change the original variable to a ref
    // of the copy.
    // After pushing on the update stack the
    // the original variable is replaced by a reference!
    to  = aux;
    frm = makeTaggedRef(&to);
    storeFwd(&frm, &to);
  }

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
void AM::gc(int msgLevel)
{
#ifdef VERBOSE
  verbReopen ();
#endif

  gcFrameToProxy();

  isCollecting = OK;
  isInGc       = OK;

  ozstat.initGcMsg(msgLevel);

  MemChunks * oldChain = MemChunks::list;

  VariableNamer::cleanup();  /* drop bound variables */
  cacheList->cacheListGC();  /* invalidate inline caches */

  initCheckSpace();
  initMemoryManagement();

  for (int j=NumberOfXRegisters; j--; )
    xRegs[j] = 0;

  Assert(trail.isEmpty());
  Assert(cachedSelf==0);
  Assert(ozstat.currAbstr==NULL);
  Assert(shallowHeapTop==0);
  Assert(_rootBoard);

  _rootBoard = _rootBoard->gcBoard();   // must go first!
  setCurrent(_currentBoard->gcBoard(),NO);

  CodeArea::gc();

  aritytable.gc ();
  ThreadsPool::doGC ();

#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc ();
#endif

  suspendVarList=makeTaggedNULL(); /* no valid data */

  OZ_collectHeapTerm(aVarUnifyHandler,aVarUnifyHandler);
  OZ_collectHeapTerm(aVarBindHandler,aVarBindHandler);

  OZ_collectHeapTerm(defaultExceptionHdl,defaultExceptionHdl);
  OZ_collectHeapTerm(opiCompiler,opiCompiler);
  OZ_collectHeapTerm(debugStreamTail,debugStreamTail);

  gc_tcl_sessions();

  toplevelVars = gcRefsArray(toplevelVars);

  extRefs = extRefs->gc();

  PROFILE_CODE1(FDProfiles.gc());

#ifdef FINALIZATION
  OZ_collectHeapTerm(finalize_handler,finalize_handler);
  gcStack.recurse();
  gc_finalize();
#endif

  gcOwnerTable();
  gcBorrowTable1();
  gcStack.recurse();
  gcBorrowTable2();


  gcStack.recurse();

  gcGNameTable();
  gcStack.recurse();


// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  varFix.fix();

  Assert(gcStack.isEmpty());

  gcBorrowTable3();
  gcSiteTable();

  exitCheckSpace();

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

  while (!isEmpty()) {
    TaggedRef * to = (TaggedRef *) pop();

    Assert(isRef(*to));

    TaggedRef * aux_ptr = tagged2Ref(*to);
    TaggedRef   aux     = *aux_ptr;

    switch (tagTypeOf(aux)) {

    case UVAR:
      {
        Board * bb = tagged2VarHome(aux)->derefBoard();

        bb = bb->gcBoard();

        Assert(bb);

        isGround = NO;

        *to = makeTaggedRef(newTaggedUVar(bb));

        break;
      }

    case SVAR:
      *to = makeTaggedRef(newTaggedSVar(tagged2SVar(aux)->gcGetFwd()));
      break;

    case CVAR:
      *to = makeTaggedRef(newTaggedCVar(tagged2CVar(aux)->gcGetFwd()));
      break;

    case GCTAG:
      *to = makeTaggedRef((TaggedRef *) GCUNMARK(aux));
      break;

    default:
      Assert(NO);
    }

    INFROMSPACE(aux_ptr);

    storeFwd((int32 *) aux_ptr, ToPointer(*to));

  }

}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */

#ifdef CS_PROFILE
static Bool across_redid = NO;
#endif

Board* AM::copyTree(Board* bb, Bool *getIsGround)
{
#ifdef VERBOSE
  if (verbOut == (FILE *) NULL)
    verbReopen ();
#endif

#ifdef CS_PROFILE
  across_redid  = NO;
  across_chunks = NO;
#endif

  PROFILE_CODE1(FDProfiles.add(bb); FDVarsTouched.discard();)

  isCollecting = OK;
  isInGc       = NO;
  isGround     = OK;

  unsigned int starttime = 0;

  if (ozconf.timeDetailed)
    starttime = osUserTime();

#ifdef CS_PROFILE
redo:
  if (across_chunks)
    across_redid = OK;

  if (across_redid)
    error("Redoing cloning twice acress chunk boundarys. Fuck!\n");

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

  if (getIsGround != (Bool *) NULL)
    *getIsGround = isGround;

  PROFILE_CODE1(FDProfiles.setBoard(toCopyBoard);)

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

void PrTabEntry::gcPrTabEntry()
{
  if (this == NULL) return;

  OZ_collectHeapTerm(info,info);
  OZ_collectHeapTerm(names,names);
}

void AbstractionEntry::gcAbstractionEntries()
{
  // there may be NULL entries in the table during gc
  AbstractionEntry *aux = allEntries;
  while(aux) {
    aux->abstr = gcAbstraction(aux->abstr);
    aux->g = (aux->abstr == NULL) ? (RefsArray) NULL : gcRefsArray(aux->g);
    aux = aux->next;
  }
}

void CodeArea::gc() {
  abstractionTab.gcAbstractionTable();
}

void ThreadsPool::doGC () {
  Assert(_currentThread==NULL);
  _rootThread         = _rootThread->gcThread();
  threadBodyFreeList = (RunnableThreadBody *) NULL;

  hiQueue.gc();
  midQueue.gc();
  lowQueue.gc();
}

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

LocalThreadQueue * LocalThreadQueue::gc() {
  if (!this)
    return NULL;

  Assert(isInGc);
  Assert(!isEmpty());

  LocalThreadQueue * new_ltq = new LocalThreadQueue (suggestNewSize());

  // gc local thread queue thread
  new_ltq->ltq_thr = ltq_thr->gcThread();

  // gc and copy entries
#ifdef  LOCAL_THREAD_STACK
  Thread * tmp;

  initReadOutFromBottom();

  while (tmp = readOutFromBottom())
    new_ltq->enqueue(tmp->gcThread());
#else
  for ( ; !isEmpty(); new_ltq->enqueue(dequeue()->gcThread()));
#endif

  return new_ltq;
}

// Note: the order of the list must be maintained
inline
OrderedSuspList * OrderedSuspList::gc() {
  OrderedSuspList * ret = NULL, * help = this, ** p = & ret;

  while (help != NULL) {

    Thread * aux = help->t->gcThread();

    if (aux) {
      *p = new OrderedSuspList(aux, NULL);
      p = & (*p)->n;
    }

    help = help->n;
  }

  return (ret);
}


//*********************************************************************
//                           NODEs
//*********************************************************************


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
  isGround = NO;
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;
      o->gcEntityInfo();

      switch(o->getTertType()) {
      case Te_Local:   o->setBoard(GETBOARD(o)->gcBoard()); break;
      case Te_Proxy:   o->gcProxy(); break;
      case Te_Manager: o->gcManager(); break;
      default:         Assert(0);
      }

      o->setClass(o->getClass()->gcClass());
      o->setFreeRecord(o->getFreeRecord()->gcSRecord());
      RecOrCell state = o->getState();
      if (stateIsCell(state)) {
        o->setState((Tertiary*) getCell(state)->gcConstTerm());
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
      a->gRegs = gcRefsArray(a->gRegs);
      a->gcConstTermWithHome();
      a->getPred()->gcPrTabEntry();
      break;
    }

  case Co_Cell:
    {
      Tertiary *t=(Tertiary*)this;
      t->gcEntityInfo();
      switch(t->getTertType()){
      case Te_Local:{
        CellLocal *cl=(CellLocal*)t;
        cl->setBoard(GETBOARD(cl)->gcBoard());
        OZ_collectHeapTerm(cl->val,cl->val);
        break;}
      case Te_Proxy:{
        t->gcProxy();
        break;}
      case Te_Frame:{
        CellFrame *cf=(CellFrame*)t;
        CellSec *cs=cf->sec;
        cf->sec=(CellSec*)gcReallocStatic(cs,sizeof(CellSec));
        cf->gcCellFrame();
        break;}
      case Te_Manager:{
        CellManager *cm=(CellManager*)t;
        CellFrame *cf=(CellFrame*)t;
        CellSec *cs=cf->sec;
        cf->sec=(CellSec*)gcReallocStatic(cs,sizeof(CellSec));
        cm->gcCellManager();
        break;}
      default:{
        Assert(0);}}
      break;
    }

  case Co_Port:
    {
      Port *p = (Port*) this;
      p->gcEntityInfo();
      switch(p->getTertType()){
      case Te_Local:{
        p->setBoard(GETBOARD(p)->gcBoard()); /* ATTENTION */
        PortWithStream *pws = (PortWithStream *) this;
        OZ_collectHeapTerm(pws->strm,pws->strm);
        break;}
      case Te_Proxy:{
        p->gcProxy();
        break;}
      case Te_Manager:{
        p->gcManager();
        PortWithStream *pws = (PortWithStream *) this;
        OZ_collectHeapTerm(pws->strm,pws->strm);
        break;}
      default:{
        Assert(0);}}
      break;
    }
  case Co_Space:
    {
      Space *s = (Space *) this;
      s->gcEntityInfo();
      if (!s->isProxy()) {
        if (s->solve != (Board *) 1)
        s->solve = s->solve->gcBoard();
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

        TaggedRef *newargs =
          (TaggedRef*) heapMalloc(sizeof(TaggedRef) * aw);

        a->setPtr(newargs);

        OZ_collectHeapBlock(a->getArgs(), newargs, aw);

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
      t->gcEntityInfo();
      switch(t->getTertType()){

      case Te_Local:{
        LockLocal *ll = (LockLocal *) this;
        ll->setBoard(GETBOARD(ll)->gcBoard());  /* maybe getBoardInternal() */
        gcPendThread(&(ll->pending));
        ll->setLocker(ll->getLocker()->gcThread());
        break;}

      case Te_Manager:{
        LockManager* lm=(LockManager*)t;
        LockFrame* lf=(LockFrame*)t;
        LockSec* ls= lf->sec;
        lf->sec=(LockSec*)gcReallocStatic(ls,sizeof(LockSec));
        lm->gcLockManager();
        break;}

      case Te_Frame:{
        LockFrame *lf=(LockFrame*)t;
        LockSec *ls=lf->sec;
        lf->sec=(LockSec*)gcReallocStatic(ls,sizeof(LockSec));
        lf->gcLockFrame();
        break;}

      case Te_Proxy:{
        t->gcProxy();
        break;}

      default:{
        Assert(0);}}
      break;
    }

  case Co_Thread:
      break;

  default:
    Assert(0);
  }
}

#define CheckLocal(CONST)                            \
{                                                    \
   Board *bb=GETBOARD(CONST);                        \
   if (!bb->gcIsAlive()) return NULL;                \
   if (!isInGc && bb->isMarkedGlobal()) return this; \
}

inline void EntityInfo::gcWatchers(){
  Watcher **base=&watchers;
  Watcher *w=*base;
  while(w!=NULL){
    Watcher* newW=(Watcher*) gcReallocStatic(w,sizeof(Watcher));
    *base=newW;
    newW->thread=newW->thread->gcThread();
    OZ_collectHeapTerm(newW->proc,newW->proc);
    base= &(newW->next);
    w=*base;}}

void Tertiary::gcEntityInfo(){
  if(info==NULL) return;
  EntityInfo *newInfo = (EntityInfo *) gcReallocStatic(info,sizeof(EntityInfo));
  info=newInfo;
  info->gcWatchers();}

ConstTerm *ConstTerm::gcConstTerm() {

  if (this == NULL)
    return NULL;

  if (gcIsMarked())
    return gcGetFwd();

  GName *gn = NULL;

  ConstTerm * ret;

  switch (getType()) {
  case Co_HeapChunk:
    return ((HeapChunk *) this)->gc();
  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
      CheckLocal(a);
      gn = a->getGName1();

      ret = (ConstTerm *) gcReallocStatic(this,sizeof(Abstraction));
      break;
    }

  case Co_Object:
    {
      Object *o = (Object *) this;
      CheckLocal(o);
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(Object));
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
        ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellManager));
        break;
      case Te_Frame:{
        CellFrame *cf=(CellFrame *)this;
        if (cf->isAccessBit()) {
          // has only been reached via gcBorrowRoot so far
          DebugCode(cf->resetAccessBit());
          void* forward=cf->getForward();
          ((CellFrame*)forward)->resetAccessBit();
          gcMark((ConstTerm *) forward);
          return (ConstTerm*) forward;
        }
        ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellFrame));
        break;
      }
      default:{
        Assert(0);
        break;}}
      break;
    }

  case Co_Port:
    {
      if(((Tertiary *)this)->getTertType()==Te_Local) {
        CheckLocal((PortLocal *) this);}
      ret = (ConstTerm *) gcReallocStatic(this,sizeof(PortLocal));
      break;
    }
  case Co_Space:
    {
      Space *sp = (Space *) this;
      CheckLocal(sp);
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
        ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockManager));
        break;
      case Te_Frame:{
        LockFrame *lf=(LockFrame *)this;
        if(lf->isAccessBit()){
          DebugCode(lf->resetAccessBit());
          void* forward=lf->getForward();
          ((LockFrame*)forward)->resetAccessBit();
          gcMark((ConstTerm *) forward);
          return (ConstTerm*) forward;}
        ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockFrame));
        break;}
      default:{
        Assert(0);
        break;}}
      break;
    }
  case Co_Thread:
    {
      Thread *old = (Thread *)this;
      Thread *th  = old->gcThread();
      if (!th) th = old->gcDeadThread();
      return th;
    }

  /* builtins are allocate dusing malloc */
  case Co_Builtin:
    return this;

#ifdef FOREIGN_POINTER
  case Co_Foreign_Pointer:
    return ((ForeignPointer*)this)->gc();
#endif

  default:
    Assert(0);
    return 0;
  }

  gcStack.push(ret,PTR_CONSTTERM);
  storeFwdField(ret);
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


ConstTerm* ConstTerm::gcConstTermSpec() {
  if (gcIsMarked())
    return gcGetFwd();

  Tertiary *t=(Tertiary*)this;
  Assert((t->getType()==Co_Cell) || (t->getType()==Co_Lock));
  Assert(t->getTertType()==Te_Frame);
  ConstTerm *ret;
  if(t->getType()==Co_Cell){
    CellFrame *cf=(CellFrame*)t;
    cf->setAccessBit();
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellFrame));
    cf->myStoreForward(ret);}
  else{
    Assert(getType()==Co_Lock);
    LockFrame *lf=(LockFrame*)t;
    lf->setAccessBit();
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockFrame));
    lf->myStoreForward(ret);}

  gcStack.push(ret,PTR_CONSTTERM);
  return ret;
}

HeapChunk * HeapChunk::gc(void) {
  HeapChunk * ret = (HeapChunk *) gcReallocStatic(this, sizeof(HeapChunk));

  ret->chunk_data = copyChunkData();

  storeFwdField(ret);
  return ret;
}

void TaskStack::gc(TaskStack *newstack) {

  TaskStack *oldstack = this;

  newstack->allocate(suggestNewSize());
  Frame *oldtop = oldstack->getTop();
  int offset    = oldstack->getUsed();
  Frame *newtop = newstack->array + offset;

  while (1) {
    GetFrame(oldtop,PC,Y,G);

    if (PC == C_EMPTY_STACK) {
      *(--newtop) = PC;
      *(--newtop) = Y;
      *(--newtop) = G;
      Assert(newstack->array == newtop);
      newstack->setTop(newstack->array+offset);
      return;
    } else if (PC == C_CATCH_Ptr) {
    } else if (PC == C_XCONT_Ptr) {
      ProgramCounter pc   = (ProgramCounter) *(oldtop-1);
      //      if (isInGc)
      //        (void)CodeArea::livenessX(pc,Y,getRefsArraySize(Y));
      Y = gcRefsArray(Y); // X
    } else if (PC == C_LOCK_Ptr) {
      Y = (RefsArray) ((OzLock *) Y)->gcConstTerm();
    } else if (PC == C_LTQ_Ptr) {
      Y = (RefsArray) ((Actor *) Y)->gcActor();
    } else if (PC == C_ACTOR_Ptr) {
      Y = (RefsArray) ((Actor *) Y)->gcActor();
    } else if (PC == C_SET_SELF_Ptr) {
      Y = (RefsArray) ((Object*)Y)->gcConstTerm();
    } else if (PC == C_SET_ABSTR_Ptr) {
      ((PrTabEntry *)Y)->gcPrTabEntry();
    } else if (PC == C_DEBUG_CONT_Ptr) {
      Y = (RefsArray) ((OzDebug *) Y)->gcOzDebug();
    } else if (PC == C_CALL_CONT_Ptr) {
      /* tt might be a variable, so use this ugly construction */
      *(newtop-2) = Y; /* UGLYYYYYYYYYYYY !!!!!!!! */
      TaggedRef *tt = (TaggedRef*) (newtop-2);
      OZ_collectHeapTerm(*tt,*tt);
      Y = (RefsArray) ToPointer(*tt);
      G = gcRefsArray(G);
    } else if (PC == C_CFUNC_CONT_Ptr) {
      G = gcRefsArray(G);
    } else { // usual continuation
      Y = gcRefsArray(Y);
      G = gcRefsArray(G);
    }

    *(--newtop) = PC;
    *(--newtop) = Y;
    *(--newtop) = G;
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

loop:

  if (bb->gcIsMarked() || bb->_isRoot())
    return nb;

  Assert(!bb->isCommitted());

  Actor *aa=bb->getActor();

  if (bb->isFailed() || aa->isCommitted()) {
    /*
     * notification board must be changed
     */
    bb=GETBOARD(aa);
    nb = bb;   // probably not dead;
    goto loop;
  }

  if (aa->gcIsMarked())
    return nb;

  bb = GETBOARD(aa);
  goto loop;
}

/****************************************************************************
 * Board collection
 ****************************************************************************/

/*
 * gcIsAlive(bb):
 *   bb is marked collected, not failed
 *   and all parents are alive
 */

Bool Board::gcIsAlive()
{
  Board *bb = this;
  Actor *aa;

 loop:
  // must be applied to a result of 'getBoard()';
  Assert (!(bb->isCommitted ()));

  if (bb->isFailed ())
    return (NO);

  if (bb->_isRoot () || bb->gcIsMarked())
    return (OK);

  aa=bb->getActor();

  if (aa->isCommitted())
    return (NO);

  if (aa->gcIsMarked())
    return OK;

  bb = GETBOARD(aa);

  goto loop;
}

inline
void Board::gcRecurse() {
  Assert(!isCommitted() && !isFailed());
  body.gcRecurse();
  u.actor=u.actor->gcActor();

  script.Script::gc();
}

inline
void AWActor::gcRecurse() {
  thread = thread->gcThread();
}

inline
void WaitActor::gcRecurse() {
  board = board->gcBoard();
  Assert(board);

  next.gcRecurse ();

  int32 num = ToInt32(children[-1]);
  Board **newChildren=(Board **) heapMalloc((num+1)*sizeof(Board *));

  *newChildren++ = (Board *) num;
  for (int i=num; i--; ) {
    if (children[i]) {
      newChildren[i] = children[i]->gcBoard();
      Assert(newChildren[i]);
    } else {
      newChildren[i] = (Board *) NULL;
    }
  }
  children = newChildren;
  cpb      = cpb->gc();
}

inline
void AskActor::gcRecurse () {
  next.gcRecurse ();
  board = board->gcBoard ();
  Assert(board);
}

inline
void SolveActor::gcRecurse () {
  if (isInGc || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
    Assert(board);
  }
  solveBoard = solveBoard->gcBoard();
  Assert(solveBoard);

  if (isInGc || !isGround())
    OZ_collectHeapTerm(solveVar,solveVar);

  OZ_collectHeapTerm(result,result);
  suspList         = suspList->gc();
  cpb              = cpb->gc();
  localThreadQueue = localThreadQueue->gc();
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
  if (isAskWait()) {
    ((AWActor *)this)->gcRecurse();
    if (isWait()) {
      ((WaitActor *)this)->gcRecurse();
    } else {
      ((AskActor *)this)->gcRecurse();
    }
  } else {
    ((SolveActor *)this)->gcRecurse();
  }
}

CpBag * CpBag::gc(void) {
  CpBag *  copy = (CpBag *) 0;
  CpBag ** cur  = &copy;
  CpBag *  old  = this;

  while (old) {
    WaitActor * wa = old->choice;

    if (wa && wa->isAliveUpToSolve() && GETBOARD(wa)->gcIsAlive()) {
      Assert(isInGc || (GETBOARD(wa))->isInTree());

      CpBag * one = new CpBag((WaitActor *) wa->gcActor());
      *cur = one;
      cur  = &(one->next);
    }
    old = old->next;
  }

  return copy;
}



//*****************************************************************************
//                           collectGarbage
//*****************************************************************************

#define ERROR(Fun, Msg)                                                       \
        error("%s in %s at %s:%d", Msg, Fun, __FILE__, __LINE__);

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

  while (1) {
    // Collect element
    OZ_collectHeapTerm(frm->args[0], to->args[0]);

    // Store forward, order is important, since gcTagged might already
    // have done a storeFwd, which means that this one will be overwritten
    storeFwd(frm->args, to->args);

    TaggedRef t = deref(frm->args[1]);

    if (!isLTuple(t)) {
      OZ_collectHeapTerm(frm->args[1], to->args[1]);
      return;
    }

    frm = tagged2LTuple(t);

    if (GCISMARKED(frm->args[0])) {
      to->args[1] = makeTaggedLTuple((LTuple *) GCUNMARK(frm->args[0]));
      return;
    }

    LTuple * next = (LTuple *) heapMalloc(sizeof(LTuple));

    to->args[1] = makeTaggedLTuple(next);
    to = next;

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

    case PTR_NAME:
      ((Name *) ptr)->gcRecurse ();
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

    case PTR_CONT:
      ((Continuation*) ptr)->gcRecurse();
      break;

    case PTR_PROPAGATOR:
      ((OZ_Propagator *) ptr)->updateHeapRefs(isInGc);
      break;

    case PTR_DYNTAB:
      ((DynamicTable *) ptr)->gcRecurse();
      break;

    case PTR_CONSTTERM:
      ((ConstTerm *) ptr)->gcConstRecurse();
      break;

    default:
      Assert(NO);
    }
  }
}



Actor * Actor::gcActor() {
  if (this==0)
    return 0;

  Assert(board->derefBoard()->gcIsAlive());

  if (gcIsMarked())
    return gcGetFwd();

  size_t sz;

  if (isWait()) {
    sz = sizeof(WaitActor);
  } else if (isAsk()) {
    sz = sizeof(AskActor);
  } else {
    sz = sizeof(SolveActor);
  }

  Actor * ret = (Actor *) OZ_hrealloc(this, sz);

  gcStack.push(ret, PTR_ACTOR);

  storeFwdField(ret);

  return ret;
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
  osBlockSignals();
  ThreadList::dispose();
  Assert(onToplevel());

  /* do gc */
  gc(ozconf.gcVerbosity);

  /* calc limits for next gc */
  int used   = getUsedMemory();
  int wanted = ((ozconf.heapFree == 100)
                ? ozconf.heapMaxSize
                : max(min(((long) used) * (100 / (100 - ozconf.heapFree)),
                          ozconf.heapMaxSize),
                      ozconf.heapMinSize));

  /* Try to align as much as possible to end of blocksize */
  int block_size = ozconf.heapBlockSize / KB;
  int block_dist = wanted % block_size;

  if (block_dist > 0)
    block_dist = block_size - block_dist;

  wanted += min(block_dist,
                (((long) wanted) * ozconf.heapTolerance / 100));

  ozconf.heapThreshold = min(wanted, ozconf.heapMaxSize);

  unsetSFlag(StartGC);
  osUnblockSignals();
}

OzDebug *OzDebug::gcOzDebug() {
  OzDebug *ret = (OzDebug*) gcReallocStatic(this,sizeof(OzDebug));

  ret->Y = gcRefsArray(ret->Y);
  ret->G = gcRefsArray(ret->G);
  OZ_collectHeapTerm(ret->data,ret->data);
  ret->arguments = gcRefsArray(ret->arguments);

  return ret;
}

// special purpose to gc borrowtable entry which is a variable
TaggedRef gcTagged1(TaggedRef in) {
  TaggedRef x=deref(in);
  Assert(GCISMARKED(x));
  return makeTaggedRef((TaggedRef*)GCUNMARK(x));
}




/*
 * DO NOT MOVE, otherwise gcc will inline this thingie, which is
 * plain wrong: inlining this function will lead to a large spill
 * code sequence in the function in which it gets inlined
 *
 */
void * OZ_hrealloc(void * p, size_t sz) {
  // Use for blocks where size is not known at compile time
  DebugCheck(sz%sizeof(int) != 0,
             error("OZ_hrealloc: can only handle word sized blocks"););

  int32 * frm = (int32 *) p;
  int32 * to  = (int32 *) heapMalloc(sz);
  int32 * ret = to;

  register int32 f0, f1, f2, f3;

  while (sz > 16) {
    f0 = frm[3]; f1 = frm[2]; f2 = frm[1]; f3 = frm[0];
    sz -= 16; frm += 4;
    to[3] = f0;  to[2] = f1; to[1] = f2;  to[0] = f3;
    to += 4;
  }

  switch(sz) {
  case 16: to[3] = frm[3];
  case 12: to[2] = frm[2];
  case  8: to[1] = frm[1];
  case  4: to[0] = frm[0];
  }

  return ret;
}
