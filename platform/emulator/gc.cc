/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller, scheidhr, popow
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

/****************************************************************************
 ****************************************************************************/

#undef TURNED_OFF
// #define TURNED_OFF

#ifdef LINUX
/* FD_ISSET missing */
#include <sys/time.h>
#endif

#include "am.hh"

#include "gc.hh"
#include "tcl_tk.hh"

#include "dllist.hh"

#include "genvar.hh"

#include "fdhook.hh"
#include "fdprofil.hh"

#include "verbose.hh"
#include "fdomn.hh"

#include "dictionary.hh"

#ifdef OUTLINE
#define inline
#endif

#ifdef PERDIO
extern OwnerTable *ownerTable;
#endif

/****************************************************************************
 * MACROS
 ****************************************************************************/

/* collect a pointer */
#define GCREF(field) if (field) { field = field->gc(); }


/****************************************************************************
 *               Forward Declarations
 ****************************************************************************/

static void processUpdateStack(void);
void performCopying(void);


/****************************************************************************
 *               Debug
 ****************************************************************************/

/*
 * CHECKSPACE  - check if object is really copied from heap (1 chunk)
 * VERBOSE     - inform user about current state of gc
 * WIPEOUTFROM - fill from space after collection with 0xff
 *     Note: in VERBOSE modus big external file (verb-out.txt) is produced.
 *    It contains very detailed debug (trace) information;
 */

#ifdef DEBUG_GC
#  define CHECKSPACE
// #  define VERBOSE
// #  define WIPEOUTFROM
#endif



/*
 * CHECKSPACE -- check if object is really copied from heap
 *   has as set of macros:
 *    INITCHECKSPACE - save pointer to from-space & print from-space
 *    EXITCHECKSPACE - print to-space
 *    INFROMSPACE    - assert in from-space
 *    NOTINTOSPACE   - assert not in to-space
 *    INTOSPACE      - assert in to-space
 * NOTE: this works only for chunk
 */

DebugCheckT(MemChunks *from);

#ifdef CHECKSPACE

#   define INITCHECKSPACE                                                     \
{                                                                             \
  printf("FROM-SPACE:\n");                                                    \
  from = MemChunks::list;                                                     \
  from->print();                                                              \
}

#   define EXITCHECKSPACE                                                     \
{                                                                             \
  printf("TO-SPACE:\n");                                                      \
  MemChunks::list->print();                                                   \
}

/* assert that P is in from-space or that P is NULL */
#   define INFROMSPACE(P)                                                     \
  if (opMode == IN_GC && P != NULL && !from->inChunkChain((void*)P)) {        \
    error("not in from-space: 0x%x %d", P, __LINE__);                         \
  }

/* assert that P is not in to-space or that P is NULL */
#   define NOTINTOSPACE(P)                                                    \
  if (opMode == IN_GC && P != NULL                                            \
      && MemChunks::list->inChunkChain((void*)P)) {                           \
    error("in to-space 0x%x %d", P, __LINE__);                                \
  }


/* assert that P is in to-space or that P is NULL */
#   define INTOSPACE(P)                                                       \
  if (opMode == IN_GC && P != NULL                                            \
      && !MemChunks::list->inChunkChain((void*)P)) {                          \
    error("not in to-space 0x%x %d", P, __LINE__);                            \
  }

#else // CHECKSPACE
#   define INITCHECKSPACE
#   define EXITCHECKSPACE
#   define INFROMSPACE(P)
#   define NOTINTOSPACE(P)
#   define INTOSPACE(P)
#endif // CHECKSPACE



/*
 * VERBOSE  --- print various debug information to the file verb-out.txt
 */

#ifdef VERBOSE
#  define GCMETHMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s this: 0x%p ",(ptrStack.getUsed ()),S,this); \
    WHERE(verbOut);                                                           \
    fprintf(verbOut,"\n");                                                    \
    fflush(verbOut);
#  define GCNEWADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) --> 0x%p ",(void *)A);                              \
    WHERE(verbOut);                                                           \
    fprintf(verbOut,"\n");                                                    \
    fflush(verbOut);
#  define GCOLDADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) <-- 0x%p ",(void *)A);                              \
    WHERE(verbOut);                                                           \
    fprintf(verbOut,"\n");                                                    \
    fflush(verbOut);
#  define GCPROCMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s ",(ptrStack.getUsed ()),S);                 \
    WHERE(verbOut);                                                           \
    fprintf(verbOut,"\n");                                                    \
    fflush(verbOut);
#else
#  define GCMETHMSG(S)
#  define GCPROCMSG(S)
#  define GCNEWADDRMSG(A)
#  define GCOLDADDRMSG(A)
#endif


/**************************************************
 *  Invalidating of inline caches
 **************************************************/

const inlineCacheListBlockSize = 100;

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
      for (int i=0; i<aux->nextFree; i++) {
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
  ThreadList(Thread *el, ThreadList *nxt): elem(el), next(nxt) {};

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
      aux->elem->printTaskStack(NOCODE,NO);
    }
  }

  static OZ_Term list()
  {
    OZ_Term out = OZ_nil();
    AM *e = &am;

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

/****************************************************************************
 *   Modes of working:
 *     IN_GC: garbage collecting
 *     IN_TC: term copying
 ****************************************************************************/

Bool gc_is_running = NO;


typedef enum {IN_GC = 0, IN_TC} GcMode;

GcMode opMode;

/*
 * TC: groundness check needs to count the number of
 *    variables, names, cells & abstractions
 */

// #define COPY_ON_GROUND
#undef COPY_ON_GROUND

#ifdef COPY_ON_GROUND

#define setVarCopied

#else

static Bool varCopied;

#define setVarCopied \
 varCopied = OK;

#endif

/*
 * TC: the copy board in from-space and to-space
 */
static Board* fromCopyBoard;
static Board* toCopyBoard;


/****************************************************************************
 * copy from from-space to to-space
 ****************************************************************************/
inline
void fastmemcpy(int32 *to, int32 *frm, int sz)
{
#ifdef VERBOSE
  fprintf(verbOut,"(gc) \tcopy %d bytes from 0x%p to 0x%p\n",sz,frm,to);
#endif
  switch(sz) {
  case 36: *(to+8) = *(frm+8);
  case 32: *(to+7) = *(frm+7);
  case 28: *(to+6) = *(frm+6);
  case 24: *(to+5) = *(frm+5);
  case 20: *(to+4) = *(frm+4);
  case 16: *(to+3) = *(frm+3);
  case 12: *(to+2) = *(frm+2);
  case  8: *(to+1) = *(frm+1);
  case  4: *(to+0) = *(frm+0);
    break;
  default:
    while(sz>0) {
      *to++ = *frm++;
      sz -= sizeof(int);
    }
  }
#ifdef WIPEOUTFROM
  error("WIPEOUTFROM is crap, it never really worked");
  if (opMode == IN_GC) memset(frm,0xff,sz);
#endif
}

inline
void *gcRealloc(void *ptr, size_t sz)
{
  void *ret = heapMalloc(sz);
  DebugCheck(sz%sizeof(int) != 0,
             error("gcRealloc: can only handle word sized blocks"););
  fastmemcpy((int32*)ret,(int32*)ptr,sz);
  return ret;
}


/*****************************************************************************
 * makeTaggedRef without consitency check
 *****************************************************************************/

inline
TaggedRef makeTaggedRefToFromSpace(TaggedRef *s)
{
  CHECK_POINTER_N(s);
/*  DebugGCT(extern MemChunks * from);
  DebugGC(gcing == 0 && !from->inChunkChain ((void *)s),
          error ("making TaggedRef pointing to 'to' space"));
          */
  return (TaggedRef) ToInt32(s);
}


/****************************************************************************
 * The pointer stack is the recursion stack for garbage collection
 * to avoid the usage of the runtime call stack
 *
 * Elements of the stack are of the type "TypePtr" which has a tag
 * and a value field.
 *
 * The stack is stored in the global variable "ptrStack".
 ****************************************************************************/

enum TypeOfPtr {
  PTR_LTUPLE,
  PTR_SRECORD,
  PTR_NAME,
  PTR_BOARD,
  PTR_ACTOR,
  PTR_THREAD,
  PTR_CONT,
  PTR_CFUNCONT,
  PTR_PROPAGATOR,
  PTR_DYNTAB,
  PTR_CONSTTERM
};


typedef TaggedRef TypedPtr;

inline
TypedPtr makeTypedPtr(void *ptr, TypeOfPtr tag)
{
  return makeTaggedRef((TypeOfTerm) tag, ptr);
}

inline
TypeOfPtr getType(TypedPtr tp)
{
  return (TypeOfPtr) tagTypeOf(tp);
}

inline
void *getPtr(TypedPtr tp)
{
  return tagValueOf(tp);
}

class TypedPtrStack: public Stack {
public:
  TypedPtrStack() : Stack(1000,Stack_WithMalloc) {}
  ~TypedPtrStack() {}

  void push(void *ptr, TypeOfPtr type) {
    Stack::push((StackEntry)makeTypedPtr(ptr,type));
  }

  TypedPtr pop()  { return (TypedPtr) ToInt32(Stack::pop()); }
};

static TypedPtrStack ptrStack;


/****************************************************************************
 *  SavedPtrStack consists of SavedPtr's and is used by coping
 * of terms (in common sense) to save overwrited value.
 *
 *  We must make here some note:
 *  such technic is used since it is was already used (in gc):)),
 * and since it seems to be efficient.
 *  It relies heavyly on fact that first cell of every term contains
 * no data that can have GCTAG bit, and, moreover, this cell
 * contains no compiler-related information (for instance,
 * pointer to procedure table if we are using run-time resolution
 * of class' methods across class hierarchy
 ****************************************************************************/

class SavedPtrStack: public Stack {
public:
  SavedPtrStack() : Stack(1000,Stack_WithMalloc) {}
  ~SavedPtrStack() {}
  void pushPtr(int32* ptr)
  {
    ensureFree(2);
    push(ptr,NO);
    push((StackEntry) *ptr,NO);
  }
};

static SavedPtrStack savedPtrStack;


/****************************************************************************
 * GCMARK
 ****************************************************************************/

/*
 * set: only used in conjunction with the function setHeapCell ???
 */
inline int32  GCMARK(void *S)    { return makeTaggedRef(GCTAG,S); }
inline int32  GCMARK(int32 S)    { return makeTaggedRef(GCTAG,S); }

inline void *GCUNMARK(int32 S)   { return tagValueOf(GCTAG,S); }
inline Bool GCISMARKED(int32 S)  { return GCTAG==tagTypeOf((TaggedRef)S); }

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
void storeForward (int32* fromPtr, void *newValue, Bool domark=OK)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(fromPtr);
  }
  DebugGC(opMode == IN_GC
          && MemChunks::list->inChunkChain((void *)fromPtr),
          error("storing marked value in 'TO' space"));
  DebugGC(opMode == IN_GC
          && from->inChunkChain(newValue),
          error("storing (marked) ref in to FROM-space"));
  *fromPtr = domark ? GCMARK(newValue) : ToInt32(newValue);
}

inline
void storeForward(void* fromPtr, void *newValue)
{
  storeForward((int32*) fromPtr, newValue);
}

//*****************************************************************************
//               Functions to gc external references into heap
//*****************************************************************************

class ExtRefNode;
static ExtRefNode *extRefs = NULL;

class ExtRefNode: public DLList {
public:
  ExtRefNode(DLListEntry el, DLList*p, DLList*n): DLList(el,p,n) {};

  static void gc()
  {
    ExtRefNode *help = extRefs;
#ifdef PROFILE
    ozstat.protectedCounter = 0;
#endif
    while(help) {
      gcTagged(*(TaggedRef*)help->elem, *(TaggedRef*)help->elem);
      help = (ExtRefNode*) help->next;
#ifdef PROFILE
      ozstat.protectedCounter++;
#endif
    }
  }
};


inline
Bool needsCollection(Literal *l)
{
  if (l->isAtom()) return NO;
  Name *nm = (Name*) l;
  return nm->isOnHeap() || nm->hasGName();
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
  if (extRefs->find(ref)) {
    Assert(0);
    return NO;
  }

  extRefs = (ExtRefNode*) extRefs->add(ref);
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
  ExtRefNode *aux = (ExtRefNode*) extRefs->find(ref);

  if (aux == NULL)
    return NO;

  extRefs = (ExtRefNode*) extRefs->remove(aux);
  return OK;
}


/****************************************************************************
 * The update stack contains REF's to not yet copied variables
 ****************************************************************************/


DebugCheckT(static int updateStackCount = 0;)

class UpdateStack: public Stack {
public:
  UpdateStack() : Stack(1000,Stack_WithMalloc) {}
  ~UpdateStack() {}
  void push(TaggedRef *t) {
    DebugGCT(updateStackCount++);
    Stack::push((StackEntry)t);
  }
  TaggedRef *pop() {
    DebugGCT(updateStackCount--;);
    return (TaggedRef*) Stack::pop();
  }
};

UpdateStack updateStack;


/****************************************************************************
 * TC: Path marks
 *  all nodes above search are marked
 *  to test if a variable,cell,name,... is local:
 *   check if the path mark is set in home board
 ****************************************************************************/

/*
 * Check if a variable, cell, procedure or name is local to the copy board.
 *  The argument is the home pointer.
 *
 * NOTE: this doesn't work for suspension list entries.
 */

inline
Bool isLocalBoard(Board* b)
{
  return !b->isPathMark();
}

/*
 * TC: before copying:  all nodes are marked, but node self
 */
inline
void setPathMarks(Board *bb)
{
  Assert(!bb->isRoot());
  do {
    bb = bb->getParent();
    bb->setPathMark();
  } while (!bb->isRoot());
}

/*
 * TC: after copying
 */

inline
void unsetPathMarks(Board *bb)
{
  Assert(!bb->isRoot());
  do {
    bb = bb->getParent();
    bb->unsetPathMark();
  } while (!bb->isRoot());
}


/*
 * Check if an entry of a suspension list is local to the copyBoard.
 */
inline
Bool isInTree (Board *b)
{
  Assert (opMode == IN_TC);
  Assert (b);
 loop:
  Assert (!b->isCommitted());
  if (b == fromCopyBoard) return (OK);
  if (!isLocalBoard(b)) return (NO);
  b = b->getParentAndTest ();
  if (!b) return (NO);
  goto loop;
}

/****************************************************************************
 * Collect all types of terms
 ****************************************************************************/

/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

inline
void dogcGName(GName *gn)
{
  if (opMode == IN_GC) {
    gcGName(gn);
  }
}

inline
Name *Name::gcName()
{
  CHECKCOLLECTED(homeOrGName, Name *);
  if (hasGName()) {
    dogcGName(getGName());
  }
  if (opMode == IN_GC && isOnHeap() ||
      opMode == IN_TC && isLocalBoard(getBoard())) {
    GCMETHMSG("Name::gc");
    COUNT(literal);
    setVarCopied;
    Name *aux = (Name*) gcRealloc(this,sizeof(Name));
    GCNEWADDRMSG(aux);
    ptrStack.push(aux, PTR_NAME);
    storeForward(&homeOrGName, aux);

    FDPROFILE_GC(cp_size_literal,sizeof(*this));
    return aux;
  } else {
    return this;
  }
}

inline
Literal *Literal::gc()
{
  if (!needsCollection(this))
    return this;

  Assert(isName());
  return ((Name*) this)->gcName();
}

inline
void Name::gcRecurse()
{
  GCMETHMSG("Name::gcRecurse");
  if (hasGName())
    return;

  homeOrGName = ToInt32(((Board*)ToPointer(homeOrGName))->gcBoard());
}

inline
Object *Object::gcObject()
{
  return (Object *) gcConstTerm();
}


/*
 * Float
 * WARNING: the value field of floats has no bit left for a gc mark
 *   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
 */
inline
Float *Float::gc()
{
  COUNT(ozfloat);

  if (opMode == IN_TC)
    return this;

  Float *ret =  newFloat(value);
  return ret;
}

inline
BigInt *BigInt::gc()
{
  if (opMode == IN_TC)
    return this;

  CHECKCOLLECTED(*(int *)&value.d, BigInt *);
  COUNT(bigInt);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  storeForward(&value.d, ret);
  return ret;
}

inline
void Script::gc()
{
  GCMETHMSG("Script::gc");
  if(first){
    int sz = numbOfCons*sizeof(Equation);
    COUNT1(scriptLen,sz);
    Equation *aux = (Equation*)gcRealloc(first,sz);
    GCNEWADDRMSG(aux);

    FDPROFILE_GC(cp_size_script, sz);

    for(int i = 0; i < numbOfCons; i++){
#ifdef DEBUG_CHECK
      //  This is the very useful consistency check.
      //  'Equations' with non-variable at the left side are figured out;
      TaggedRef auxTerm = first[i].left;
      TaggedRef *auxTermPtr;
      if (opMode == IN_TC && IsRef(auxTerm)) {
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
      gcTagged(first[i].left,  aux[i].left);
      gcTagged(first[i].right, aux[i].right);
    }

    first = aux;
  }
}

#define RAGCTag (1<<31)

inline Bool refsArrayIsMarked(RefsArray r)
{
  return (r[-1]&RAGCTag);
}

inline void refsArrayMark(RefsArray r, void *ptr)
{
  storeForward((int32*)&r[-1],ToPointer(ToInt32(ptr)|RAGCTag),NO);
}

inline RefsArray refsArrayUnmark(RefsArray r)
{
  return (RefsArray) ToPointer(r[-1]&(~(RAGCTag)|mallocBase));
}


// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc tag set --> has already been copied

RefsArray gcRefsArray(RefsArray r)
{
  GCPROCMSG("gcRefsArray");
  GCOLDADDRMSG(r);
  if (r == NULL)
    return r;

  NOTINTOSPACE(r);

  if (refsArrayIsMarked(r)) {
    return refsArrayUnmark(r);
  }

  Assert(!isFreedRefsArray(r));
  int sz = getRefsArraySize(r);
  COUNT(refsArray);
  COUNT1(refsArrayLen,sz);

  RefsArray aux = allocateRefsArray(sz,NO);
  GCNEWADDRMSG(aux);

  FDPROFILE_GC(cp_size_refsarray, (sz + 1) * sizeof(TaggedRef));

  refsArrayMark(r,aux);

  for(int i = sz-1; i >= 0; i--)
    gcTagged(r[i],aux[i]);

  return aux;
}

/*
 *  Thread items methods;
 *
 */
//
//  RunnableThreadBody;


inline
ChachedOORegs gcChachedOORegs(ChachedOORegs regs)
{
  Object *o = getObject(regs)->gcObject();
  return setObject(regs,o);
}

RunnableThreadBody *RunnableThreadBody::gcRTBody ()
{
  GCMETHMSG ("RunnableThreadBody::gcRTBody");

  RunnableThreadBody *ret =
    (RunnableThreadBody *) gcRealloc (this, sizeof (*this));
  GCNEWADDRMSG (ret);
  taskStack.gc(&ret->taskStack);

  return (ret);
}

OZ_Propagator * OZ_Propagator::gc(void)
{
  GCMETHMSG("OZ_Propagator::gc");
  GCOLDADDRMSG(this);

  OZ_Propagator * p = (OZ_Propagator *) gcRealloc(this, sizeOf());

  GCNEWADDRMSG (p);

  ptrStack.push(p, PTR_PROPAGATOR);

  return p;
}

void OZ_updateHeapTerm(OZ_Term &t)
{
  Assert(isRef(t) || !isAnyVar(t));
  gcTagged(t, t);
}

//
//  ... Continuation;
inline
void Continuation::gcRecurse ()
{
  GCMETHMSG ("Continuation::gcRecurse");

  yRegs = gcRefsArray(yRegs);
  gRegs = gcRefsArray(gRegs);
  xRegs = gcRefsArray(xRegs);
}

Continuation *Continuation::gc()
{
  GCMETHMSG ("Continuation::gc");
  CHECKCOLLECTED (ToInt32 (pc), Continuation *);

  COUNT(continuation);
  Continuation *ret =
    (Continuation *) gcRealloc (this, sizeof (*this));
  GCNEWADDRMSG (ret);
  ptrStack.push (ret, PTR_CONT);
  storeForward (&pc, ret);

  FDPROFILE_GC(cp_size_cont, sizeof(Continuation));

  return (ret);
}

/* collect LTuple, SRecord */

inline Bool isDirectVar(TaggedRef t)
{
  return (!isRef(t) && isAnyVar(t));
}

inline
void gcTaggedBlock(TaggedRef *oldBlock, TaggedRef *newBlock,int sz)
{
  for(int i = sz-1; i>=0; i--) {
    if (isDirectVar(oldBlock[i])) {
      gcTagged(oldBlock[i],newBlock[i]);
    }
  }
}

inline
LTuple *LTuple::gc()
{
  GCMETHMSG("LTuple::gc");
  CHECKCOLLECTED(args[0], LTuple *);

  COUNT(lTuple);
  LTuple *ret = (LTuple*) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_LTUPLE);

  FDPROFILE_GC(cp_size_ltuple, sizeof(*this));

  gcTaggedBlock(args,ret->args,2);

  storeForward((int *) &args[0], ret);
  return ret;
}


inline
SRecord *SRecord::gcSRecord()
{
  GCMETHMSG("SRecord::gcSRecord");
  if (this==NULL) return NULL;
  CHECKCOLLECTED(label, SRecord *);
  COUNT(sRecord);
  COUNT1(sRecordLen,getWidth());
  int len = (getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord);

  SRecord *ret = (SRecord*) gcRealloc(this,len);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_SRECORD);
  storeForward((int32*)&label, ret);
  gcTaggedBlock(getRef(),ret->getRef(),getWidth());

  FDPROFILE_GC(cp_size_record, len);

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
//inline
Thread *Thread::gcThread ()
{
  GCMETHMSG ("Thread::gcThread");

  if (this == (Thread *) NULL) return ((Thread *) NULL);
  Thread *ret = isCollected(this,item.threadBody);
  if (ret) return ret;

  if (isDeadThread ()) return ((Thread *) NULL);

  //  Some invariants:
  // nothing can be copied (IN_TC) until stability;

  // first class threads: must only copy thread when local to solve!!!
  if (opMode == IN_TC && !isLocalBoard(getBoard())) {
    return this;
  }

  Assert (opMode == IN_GC || !(isRunnable ()));

  //
  //  Note that runnable threads can be also counted
  // in solve actors (for stabilittty check), and, therefore,
  // might not just dissappear!
  if (isSuspended () && !((getBoard())->gcIsAlive ())) {
    return ((Thread *) NULL);
  }

  COUNT(thread);
  Thread *newThread = (Thread *) gcRealloc (this, sizeof (*this));
  GCNEWADDRMSG (newThread);

  Assert(opMode==IN_TC || opMode==IN_GC);

  if (isRunnable () || hasStack ()) {
    ThreadList::add (newThread);
  }

  ptrStack.push (newThread, PTR_THREAD);

  FDPROFILE_GC(cp_size_susp, sizeof(*this));

  storeForward (&item.threadBody, newThread);

  return (newThread);
}

Thread *Thread::gcDeadThread()
{
  Assert(isDeadThread());

  COUNT(thread);
  Thread *newThread = (Thread *) gcRealloc (this, sizeof (*this));
  GCNEWADDRMSG (newThread);

  newThread->setBoard(am.rootBoard->gcBoard());
  newThread->state.flags=0;
  newThread->item.threadBody=0;

  storeForward (&item.threadBody, newThread);
  setSelf(getSelf()->gcObject());

  return (newThread);
}

void Thread::gcRecurse ()
{
  GCMETHMSG("Thread::gcRecurse");

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

    getBoard()->incSuspCount ();

    //
    //  Convert the thread to a 'wakeup' type, and just throw away
    // the body;
    setWakeUpTypeGC ();
    item.threadBody = (RunnableThreadBody *) NULL;
  } else {
    setBoard(newBoard);
  }

  //
  switch (getThrType ()) {
  case S_RTHREAD:
    item.threadBody = item.threadBody->gcRTBody ();
    break;

  case S_WAKEUP:
    //  should not contain any reference;
    Assert (item.threadBody == (RunnableThreadBody *) NULL);
    break;

  case S_PR_THR:
    item.propagator = item.propagator->gc();
    Assert (item.propagator);
    break;

  default:
    error ("Unknown type of a runnable thread?\n");
  }

  setSelf(getSelf()->gcObject());

  gcTertiary();
}

/*
 *  We reverse the order of the list, but this should be no problem.
 *
 * kost@ : ... in any case, this is complaint with the
 * 'The Definition of Kernel Oz';
 *
 */
inline
SuspList * SuspList::gc()
{
  GCMETHMSG("SuspList::gc");

  SuspList *ret = NULL;

  for(SuspList* help = this; help != NULL; help = help->next) {
    Thread *aux = (help->getElem ())->gcThread ();
    if (!aux) {
      continue;
    }
    ret = new SuspList(aux, ret);
    COUNT(suspList);

    FDPROFILE_GC(cp_size_susplist, sizeof(SuspList));
  }
  GCNEWADDRMSG (ret);
  return (ret);
}

inline
void GenCVariable::gc(void)
{
  switch (getType()){
  case FDVariable:
    ((GenFDVariable*)this)->gc();
    FDPROFILE_GC(cp_size_fdvar, sizeof(GenFDVariable));

    break;
  case OFSVariable:
    ((GenOFSVariable*)this)->gc();
    FDPROFILE_GC(cp_size_ofsvar, sizeof(GenOFSVariable));
    break;
  case MetaVariable:
    ((GenMetaVariable*)this)->gc();
    FDPROFILE_GC(cp_size_metavar, sizeof(GenMetaVariable));
    break;
  case BoolVariable:
    FDPROFILE_GC(cp_size_boolvar, sizeof(GenBoolVariable));
    break;
  case AVAR:
    ((AVar *) this)->gcAVar();
    break;
  case PerdioVariable:
    ((PerdioVar *) this)->gcPerdioVar();
    break;
  case FSetVariable:
    ((GenFSetVariable *) this)->gc();
    break;
  default:
    Assert(0);
  }
}


/*
 * This procedure collects the entry points into heap provided by variables,
 * without copying the tagged reference of the variable itself.
 * NOTE: there is maybe junk in X/Y registers, so home node may be dead
 */
TaggedRef gcVariable(TaggedRef var)
{
  GCPROCMSG("gcVariable");
  GCOLDADDRMSG(var);
  if (var==nil()) { return nil(); }
  if (isUVar(var)) {
    Board *bb = tagged2VarHome(var);
    INFROMSPACE(bb);
    bb = bb->gcBoard();
    if (!bb) return nil();
    INTOSPACE (bb);
    TaggedRef ret= makeTaggedUVar(bb);
    COUNT(uvar);
    GCNEWADDRMSG(ret);
    return ret;
  }
  if (isSVar(var)) {
    SVariable *cv = tagged2SVar(var);
    INFROMSPACE(cv);
    if (GCISMARKED(ToInt32(cv->suspList))) {
      GCNEWADDRMSG(makeTaggedSVar((SVariable*)GCUNMARK(ToInt32(cv->suspList))));
      return makeTaggedSVar((SVariable*)GCUNMARK(ToInt32(cv->suspList)));
    }

    Board *bb = cv->home;
    bb=bb->gcBoard();
    if (!bb) return nil();

    int cv_size;
    cv_size = sizeof(SVariable);

    SVariable *new_cv = (SVariable*)gcRealloc(cv,cv_size);
    COUNT(svar);

    FDPROFILE_GC(cp_size_svar, cv_size);

    storeForward(&cv->suspList, new_cv);

    new_cv->suspList = new_cv->suspList->gc();

    Assert(opMode != IN_GC || new_cv->home != bb);

    new_cv->home = bb;
    GCNEWADDRMSG(makeTaggedSVar(new_cv));
    return makeTaggedSVar(new_cv);
  }

  Assert(isCVar(var));
  GenCVariable *gv = tagged2CVar(var);

  INFROMSPACE(gv);
  if (GCISMARKED(ToInt32(gv->suspList))) {
    GCNEWADDRMSG(makeTaggedCVar((GenCVariable*)GCUNMARK(ToInt32(gv->suspList))));
    return makeTaggedCVar((GenCVariable*)GCUNMARK(ToInt32(gv->suspList)));
  }

  Board *bb = gv->home->gcBoard();
  if (!bb) return nil();

  int gv_size = gv->getSize();

  COUNT(cvar);
  GenCVariable *new_gv = (GenCVariable*)gcRealloc(gv, gv_size);

  storeForward(&gv->suspList, new_gv);

  new_gv->suspList = new_gv->suspList->gc();

  new_gv->gc();

  Assert(opMode != IN_GC || new_gv->home != bb);

  new_gv->home = bb;
  GCNEWADDRMSG(makeTaggedCVar(new_gv));
  return makeTaggedCVar(new_gv);
}


inline
void OZ_FiniteDomainImpl::gc(void)
{
  FDPROFILE_GC(cp_size_fdvar, getDescrSize());

  copyExtension();
}

void GenFDVariable::gc(void)
{
  GCMETHMSG("GenFDVariable::gc");
  ((OZ_FiniteDomainImpl *) &finiteDomain)->gc();

  int i;
  for (i = fd_prop_any; i--; )
    fdSuspList[i] = fdSuspList[i]->gc();
}

void GenFSetVariable::gc(void)
{
  GCMETHMSG("GenFSetVariable::gc");

  int i;
  for (i = fs_prop_any; i--; )
    fsSuspList[i] = fsSuspList[i]->gc();
}

FSetValue * FSetValue::gc(void)
{
  return (FSetValue *) gcRealloc(this, sizeof(*this));
}

void GenMetaVariable::gc(void)
{
  GCMETHMSG("GenMetaVariable::gc");
  gcTagged(data, data);
}

void AVar::gcAVar(void)
{
  GCMETHMSG("AVar::gc");
  gcTagged(value, value);
}

DynamicTable* DynamicTable::gc(void)
{
    GCMETHMSG("DynamicTable::gc");

    Assert(isPwrTwo(size));
    // Copy the table:
    COUNT(dynamicTable);
    COUNT1(dynamicTableLen,size);
    size_t len = (size-1)*sizeof(HashElement)+sizeof(DynamicTable);
    DynamicTable* ret = (DynamicTable*) gcRealloc(this,len);

    FDPROFILE_GC(cp_size_ofsvar, len);

    GCNEWADDRMSG(ret);
    // Take care of all TaggedRefs in the table:
    ptrStack.push(ret,PTR_DYNTAB);
    // (no storeForward needed since only one place points to the dynamictable)
    for (dt_index i=0; i<size; i++) {
        if (table[i].ident!=makeTaggedNULL()
            && (!isRef(table[i].ident) && isAnyVar(table[i].ident))) {
            gcTagged(table[i].ident, ret->table[i].ident);
            gcTagged(table[i].value, ret->table[i].value);
        }
    }
    return ret;
}


void DynamicTable::gcRecurse()
{
    for (dt_index i=0; i<size; i++) {
        if (table[i].ident!=makeTaggedNULL()
            && (isRef(table[i].ident) || !isAnyVar(table[i].ident))) {
            gcTagged(table[i].ident, table[i].ident);
            gcTagged(table[i].value, table[i].value);
        }
    }
}



void GenOFSVariable::gc(void)
{
    GCMETHMSG("GenOFSVariable::gc");
    gcTagged(label, label);
    // Update the pointer in the copied block:
    dynamictable=dynamictable->gc();
}

inline
Board *gcGetVarHome(TaggedRef var)
{
  if (isUVar(var)) {
    return tagged2VarHome(var)->derefBoard();
  }
  if (isSVar(var)) {
    return tagged2SVar(var)->getBoard();
  }
  Assert(isCVar(var));
  return tagged2CVar(var)->getBoard();
}

/*
 * If an object has been copied, all slots of type
 * 'TaggedRef' have to be
 * treated particularly:
 * - references pointing to non-variables have to be derefenced
 * - references to variables have to be treated after the whole term structure
 *   has been collected and therefore the address of such a reference has to
 *   put on the update stack
 * - variables, which are part of the block being copied, have to be marked as
 *   copied
 * - if non-collected variables are dereferenced, the entry points into heap
 *   provided by them, have to be collected by 'gcVariable'
 */
void gcTagged(TaggedRef &fromTerm, TaggedRef &toTerm)
{
  GCPROCMSG("gcTagged");
  TaggedRef auxTerm = fromTerm;

  Assert(opMode != IN_GC || !from->inChunkChain(&toTerm));

  /* initalized but unused cell in register array */
  if (auxTerm == makeTaggedNULL()) {
    toTerm = makeTaggedNULL();
    return;
  }


  DEREF(auxTerm,auxTermPtr,auxTermTag);

  if (GCISMARKED(auxTerm)) {
    toTerm = makeTaggedRef((TaggedRef*)GCUNMARK(auxTerm));
    return;
  }

  DebugGCT(NOTINTOSPACE(auxTermPtr));

  switch (auxTermTag) {

  case SMALLINT: toTerm = auxTerm; break;
  case FSETVALUE: toTerm = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(auxTerm))->gc()); break;
  case LITERAL:  toTerm = makeTaggedLiteral(tagged2Literal(auxTerm)->gc()); break;
  case LTUPLE:   toTerm = makeTaggedLTuple(tagged2LTuple(auxTerm)->gc()); break;
  case SRECORD:  toTerm = makeTaggedSRecord(tagged2SRecord(auxTerm)->gcSRecord()); break;
  case BIGINT:   toTerm = makeTaggedBigInt(tagged2BigInt(auxTerm)->gc()); break;
  case OZFLOAT:  toTerm = makeTaggedFloat(tagged2Float(auxTerm)->gc());   break;

  case OZCONST:
    {
      ConstTerm *con=tagged2Const(auxTerm)->gcConstTerm();
      toTerm = con ? makeTaggedConst(con) : nil();
    }
    break;

  case SVAR:
  case UVAR:
  case CVAR:
    setVarCopied;
    if (auxTerm == fromTerm) {   // no DEREF needed

      DebugGCT(toTerm = fromTerm); // otherwise 'makeTaggedRef' complains
      if (opMode!=IN_TC || isLocalBoard(gcGetVarHome(auxTerm))) {
        storeForward((int *) &fromTerm, &toTerm);

        // updating toTerm AFTER fromTerm:
        toTerm = gcVariable(auxTerm);
      } else {
        toTerm = fromTerm;
      }
    } else {

      // put address of ref cell to be updated onto update stack
      if (opMode!=IN_TC || isLocalBoard(gcGetVarHome(auxTerm))) {
        updateStack.push(&toTerm);
        gcVariable(auxTerm);
        toTerm = makeTaggedRefToFromSpace(auxTermPtr);
        Assert(auxTermPtr != 0);
      } else {
        toTerm = fromTerm;
      }
    }
    break;
  default:
    Assert(0);
  }
}


//*****************************************************************************
//                               AM::gc
//*****************************************************************************

int gcing = 1;

// This method is responsible for the heap garbage collection of the
// abstract machine, ie that all entry points into heap are properly
// treated and references to variables are properly updated
void AM::gc(int msgLevel)
{
#ifdef VERBOSE
  verbReopen ();
#endif
  GCMETHMSG(" ********** AM::gc **********");
  opMode = IN_GC;
  gcing = 0;
  gc_is_running = OK;

  ozstat.initGcMsg(msgLevel);

  MemChunks *oldChain = MemChunks::list;

  VariableNamer::cleanup();  /* drop bound variables */
  cacheList->cacheListGC();  /* invalidate inline caches */

  INITCHECKSPACE;
  initMemoryManagement();
  INITCOUNT();

  { /* initialize X regs; this IS necessary ! */
    int sz = getRefsArraySize(xRegs);
    for (int j=0; j < sz; j++) {
      xRegs[j] = makeTaggedNULL();
    }
  }

//                 actual garbage collection starts here
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

// colouring root pointers grey
//-----------------------------------------------------------------------------

  Assert(trail.isEmpty());
  Assert(rebindTrail.isEmpty());

  rootBoard = rootBoard->gcBoard();   // must go first!
  Assert(cachedSelf==0);
  Assert(rootBoard);
  setCurrent(currentBoard->gcBoard(),NO);

  GCPROCMSG("Predicate table");

  CodeArea::gc();

  aritytable.gc ();
  ThreadsPool::doGC ();
  Assert(rootThread);

#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc ();
#endif

  suspendVarList=makeTaggedNULL(); /* no valid data */

#ifdef PERDIO
  gcOwnerTable();
#endif

  gcTagged(aVarUnifyHandler,aVarUnifyHandler);
  gcTagged(aVarBindHandler,aVarBindHandler);

  gcTagged(methApplHdl,methApplHdl);

  gcTagged(defaultExceptionHandler,defaultExceptionHandler);

  gcTagged(threadStream,threadStream);
  gcTagged(threadStreamTail,threadStreamTail);

  gcTagged(tcl_lock, tcl_lock);
  gcTagged(tcl_rets, tcl_rets);
  gcTagged(tcl_dict, tcl_dict);

  performCopying();

  GCPROCMSG("toplevelVars");
  am.toplevelVars = gcRefsArray(am.toplevelVars);

  GCPROCMSG("updating external references to terms into heap");
  ExtRefNode::gc();

  PROFILE_CODE1(FDProfiles.gc());

  gcBorrowTableRoots();
  performCopying();

// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  GCPROCMSG("updating references");
  processUpdateStack ();

  if(!ptrStack.isEmpty())
    error("ptrStack should be empty");

  EXITCHECKSPACE;

#ifdef PERDIO
  gcPostOwnerTable();
  gcBorrowTable();
  gcGNameTable();
#endif

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  Assert(currentThread==NULL);
  cachedStack = NULL;

  // HERE !!!!!!!!!!!!!!

  ozstat.printGcMsg(msgLevel);

  gc_is_running = NO;
  gcing = 1;
} // AM::gc


/*
 *   Process updateStack -
 *
 */
void processUpdateStack(void)
{
 loop:

  while (!updateStack.isEmpty())
    {
      TaggedRef *tt = updateStack.pop();
      TaggedRef auxTerm     = *tt;
      TaggedRef *auxTermPtr = NULL;

      while(IsRef(auxTerm)) {
        Assert(auxTerm);
        auxTermPtr = tagged2Ref(auxTerm);
        auxTerm = *auxTermPtr;
      }

      if (GCISMARKED(auxTerm)) {
        *tt = makeTaggedRef((TaggedRef*)GCUNMARK(auxTerm));
        goto loop;
      }

      TaggedRef newVar = gcVariable(auxTerm);

      if (newVar == nil()) {
        *tt = nil();
        *auxTermPtr = nil();
      } else {
        Assert(tagTypeOf(newVar) == tagTypeOf(auxTerm));
        switch(tagTypeOf(newVar)){
        case UVAR:
          *tt = makeTaggedRef(newTaggedUVar(tagged2VarHome(newVar)));
          COUNT(uvar);
          break;
        case SVAR:
          *tt = makeTaggedRef(newTaggedSVar(tagged2SVar(newVar)));
          COUNT(svar);
          break;
        case CVAR:
          *tt = makeTaggedRef(newTaggedCVar(tagged2CVar(newVar)));
          COUNT(cvar);
          break;
        default:
          Assert(NO);
        }
        INFROMSPACE(auxTermPtr);
        storeForward((int *) auxTermPtr,ToPointer(*tt));
      }
    } // while

  Assert(updateStackCount==0);
}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */

Board* AM::copyTree(Board* bb, Bool *isGround)
{
#ifdef VERBOSE
  if (verbOut == (FILE *) NULL)
    verbReopen ();
#endif

  PROFILE_CODE1(FDProfiles.add(bb); FDVarsTouched.discard();)

  if (isGround == (Bool *) NULL) {
    GCMETHMSG(" ********** AM::copyTree **********");
  } else {
    GCMETHMSG(" ********** AM::copyTree (groundness) **********");
  }
  opMode = IN_TC;
  gcing = 0;
#ifndef COPY_ON_GROUND
  varCopied = NO;
#endif
  unsigned int starttime = osUserTime();

  Assert(!bb->isCommitted());
  fromCopyBoard = bb;
  setPathMarks(fromCopyBoard);
  toCopyBoard = fromCopyBoard->gcBoard();
  Assert(toCopyBoard);

  performCopying();

  processUpdateStack();

  Assert(ptrStack.isEmpty());

  while (!savedPtrStack.isEmpty()) {
    int value = ToInt32(savedPtrStack.pop());
    int* ptr  = (int*) savedPtrStack.pop();
    *ptr = value;
  }

  unsetPathMarks(fromCopyBoard);
  fromCopyBoard = NULL;
  gcing = 1;

  ozstat.timeForCopy.incf(osUserTime()-starttime);
  // Note that parent, right&leftSibling must be set in this subtree -
  // for instance, with "setParent"

  if (isGround != (Bool *) NULL) {
#ifdef COPY_ON_GROUND
    *isGround = NO;
#else
    if (varCopied == NO)
      *isGround = OK;
    else
      *isGround = NO;
#endif
  }

  PROFILE_CODE1(FDProfiles.setBoard(toCopyBoard);)

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
    GCMETHMSG("Arity::gc");
    if (!aux->isTuple()) {
      for (int i = 0; i < aux->size; i++) {
        if (aux->keytable[i] != makeTaggedNULL()) {
          gcTagged(aux->keytable[i],aux->keytable[i]);
        }
      }
    }
    gcTagged(aux->list, aux->list);
    aux = aux->next;
  }
}

void ArityTable::gc()
{
  GCMETHMSG("ArityTable::gc");

  for (int i = 0; i < size; i++) {
    if (table[i] != NULL) {
      (table[i])->gc();
    }
  }
}

void AbstractionEntry::gcAbstractionEntries()
{
  // there may be NULL entries in the table during gc
  AbstractionEntry *aux = allEntries;
  while(aux) {
    aux->abstr = (Abstraction *) aux->abstr->gcConstTerm();
    aux->g = (aux->abstr == NULL) ? (RefsArray) NULL : gcRefsArray(aux->g);
    aux = aux->next;
  }
}

void CodeArea::gc()
{
  abstractionTab.gcAbstractionTable();
}

void ThreadsPool::doGC ()
{
  Assert(currentThread==NULL);
  rootThread         = rootThread->gcThread();
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

LocalThreadQueue * LocalThreadQueue::gc()
{
  if (!this) return NULL;

  Assert(opMode == IN_GC);
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

void TaskStack::gc(TaskStack *newstack)
{
  COUNT(taskStack);
  COUNT1(taskStackLen,getMaxSize());

  newstack->allocate(suggestNewSize());
  TaskStack *oldstack = this;

  TaskStackEntry *oldtop = oldstack->getTop();
  int offset             = oldstack->getUsed();
  TaskStackEntry *newtop = newstack->array + offset;

  while (1) {

    PopFrame(oldtop,PC,Y,G);

    *(--newtop) = PC;

    if (PC==C_EMPTY_STACK) {
      *(--newtop) = Y;
      *(--newtop) = G;
      break;
    }

    if (PC==C_ACTOR_Ptr) {
      *(--newtop) = ((AWActor *) Y)->gcActor();
      *(--newtop) = G;
      continue;
    }

    if (PC==C_XCONT_Ptr) {
      *(--newtop) = gcRefsArray(Y); // X
      *(--newtop) = G;
      continue;
    }

    if (PC==C_LOCK_Ptr) {
      *(--newtop) = ((OzLock *) Y)->gcConstTerm();
      *(--newtop) = G;
      continue;
    }

    if (PC==C_CATCH_Ptr) {
      *(--newtop) = Y;
      *(--newtop) = G;
      continue;
    }

    if (PC==C_LTQ_Ptr) {
      *(--newtop) = ((Actor *) Y)->gcActor();
      *(--newtop) = G;
      continue;
    }

    if (PC==C_SET_SELF_Ptr) {
      *(--newtop) = ((Object*)Y)->gcConstTerm();
      *(--newtop) = G;
      continue;
    }

    if (PC==C_DEBUG_CONT_Ptr) {
      *(--newtop) = ((OzDebug *) Y)->gcOzDebug();
      *(--newtop) = G;
      continue;
    }

    if (PC==C_CALL_CONT_Ptr) {
      TaggedRef tt=deref((TaggedRef) ToInt32(Y));
      Assert(!isAnyVar(tt));
      gcTagged(tt,tt);
      *(--newtop) = ToPointer(tt);
      *(--newtop) = gcRefsArray(G);
      continue;
    }

    if (PC==C_CFUNC_CONT_Ptr) {
      *(--newtop) = Y;                // OZ_CFun
      *(--newtop) = gcRefsArray(G);
      continue;
    }

    COUNT(cCont);
    // PC is already queued
    *(--newtop) = gcRefsArray(Y);
    *(--newtop) = gcRefsArray(G);
  } // while not task stack is empty

  Assert(newstack->array == newtop);
  newstack->setTop(newstack->array+offset);
}



//*********************************************************************
//                           NODEs
//*********************************************************************


void ConstTerm::gcConstRecurse()
{
  setVarCopied;
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;
      o->setClass(o->getClass()->gcClass());
      o->setFreeRecord(o->getFreeRecord()->gcSRecord());
      if (o->isDeep()){
        ((DeepObject*)o)->home = o->getBoard()->gcBoard();
      }
      o->setState(o->getState()->gcSRecord());
      int oldFlags = o->flagsAndLock&(~ObjFlagMask);
      o->flagsAndLock = ToInt32(o->getLock()->gcConstTerm())|oldFlags;
      break;
    }

  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
      a->gRegs = gcRefsArray(a->gRegs);
      a->gcTertiary();
      if (a->isProxy()) {
        ProcProxy *pp = (ProcProxy*) a;
        gcTagged(pp->suspVar,pp->suspVar);
      } else if (a->isLocal()) {
        a->setBoard(a->getBoard()->gcBoard());
      }
      break;
    }

  case Co_Cell:
    {
      Tertiary *t=(Tertiary*)this;
      switch(t->getTertType()){
      case Te_Local:{
        CellLocal *cl=(CellLocal*)t;
        cl->setBoard(cl->getBoard()->gcBoard()); /* ATTENTION */
        gcTagged(cl->val,cl->val);
        break;}
      case Te_Proxy:{
        t->gcProxy();
        break;}
      case Te_Frame:{
        CellFrame *cf=(CellFrame*)t;
        CellSec *cs=cf->sec;
        cf->sec=(CellSec*)gcRealloc(cs,sizeof(CellSec));
        cf->gcCellFrame();
        break;}
      case Te_Manager:{
        CellManager *cm=(CellManager*)t;
        CellFrame *cf=(CellFrame*)t;
        CellSec *cs=cf->sec;
        cf->sec=(CellSec*)gcRealloc(cs,sizeof(CellSec));
        cm->gcCellManager();
        break;}
      default:{
        Assert(0);}}
      break;
    }

  case Co_Port:
    {
      Port *p = (Port*) this;
      switch(p->getTertType()){
      case Te_Local:{
        p->setBoard(p->getBoard()->gcBoard()); /* ATTENTION */
        PortWithStream *pws = (PortWithStream *) this;
        gcTagged(pws->strm,pws->strm);
        break;}
      case Te_Proxy:{
        p->gcProxy();
        break;}
      case Te_Manager:{
        p->gcManager();
        PortWithStream *pws = (PortWithStream *) this;
        gcTagged(pws->strm,pws->strm);
        break;}
      default:{
        Assert(0);}}
      break;
    }
  case Co_Space:
    {
      Space *s = (Space *) this;
      if (!s->isProxy()) {
        if (s->solve != (Board *) 1)
        s->solve = s->solve->gcBoard();
        if (s->isLocal()) {
          s->setBoard(s->getBoard()->gcBoard());
        }
      }
      break;
    }

  case Co_Chunk:
    {
      SChunk *c = (SChunk *) this;

      c->gcTertiary();
      if (c->value)
        gcTagged(c->value,c->value);
      c->setPtr(((Board *) c->getPtr())->gcBoard());

      break;
    }

  case Co_Array:
    {
      OzArray *a = (OzArray*) this;

      a->home = a->home->gcBoard();

      if (a->getWidth() > 0) {
        TaggedRef *oldargs = a->getArgs();
        TaggedRef *newargs = (TaggedRef*) gcRealloc(oldargs,
                                                    sizeof(TaggedRef)*a->getWidth());
        for (int i=0; i < a->getWidth(); i++) {
          gcTagged(oldargs[i],newargs[i]);
        }

        a->setPtr(newargs);
      }
      break;
    }

  case Co_Dictionary:
    {
      OzDictionary *dict = (OzDictionary *) this;
      dict->home  = dict->home->gcBoard();
      dict->table = dict->table->gc();
      break;
    }

  case Co_Lock:
    {
      OzLock *lock = (OzLock *) this;
      lock->home  = lock->home->gcBoard();
      gcTagged(lock->threads,lock->threads);
      lock->locker = lock->locker->gcThread();
      break;
    }

  case Co_Thread:
      break;

  default:
    Assert(0);
  }
}

inline
void PrTabEntry::gcPrTabEntry()
{
  dogcGName(gname);
}


#define CheckLocal(CONST)                                       \
{                                                               \
   Board *bb=(CONST)->getBoard();                               \
   if (!bb->gcIsAlive()) return NULL;                           \
   if (opMode == IN_TC && !isLocalBoard(bb)) return this;       \
}

ConstTerm *ConstTerm::gcConstTerm()
{
  GCMETHMSG("ConstTerm::gcConstTerm");
  if (this == NULL) return NULL;
  CHECKCOLLECTED(*getGCField(), ConstTerm *);

  size_t sz = 0;
  switch (getType()) {
  case Co_Board:     return ((Board *) this)->gcBoard();
  case Co_Actor:     return ((Actor *) this)->gcActor();
  case Co_HeapChunk: return ((HeapChunk *) this)->gc();
  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
      CheckLocal(a);
      sz = a->isProxy() ? sizeof(ProcProxy) : sizeof(Abstraction);
      dogcGName(a->getGName());
      a->getPred()->gcPrTabEntry();
      COUNT(abstraction);
      break;
    }

  case Co_Object:
    {
      Object *o = (Object *) this;
      CheckLocal(o);
      sz = o->isDeep() ? sizeof(DeepObject): sizeof(Object);
      COUNT1(deepObject,o->isDeep()?1:0);
      COUNT1(flatObject,o->isDeep()?0:1);
      break;
    }
  case Co_Cell:
    {
      COUNT(cell);
      switch(((Tertiary *)this)->getTertType()) {
      case Te_Local:{
        CellLocal *cl=(CellLocal*)this;
        CheckLocal(cl);
        sz = sizeof(CellLocal);
        break;}
      case Te_Proxy:{
        sz = sizeof(CellProxy);
        break;}
      case Te_Manager:{
        sz = sizeof(CellManager);
        break;}
      case Te_Frame:{
        CellFrame *cf=(CellFrame *)this;
        if(cf->isAccessBit()){
          DebugCode(cf->resetAccessBit());
          void* forward=cf->getForward();
          ((CellFrame*)forward)->resetAccessBit();
          *getGCField()=GCMARK(forward);
          return (ConstTerm*) forward;}
        sz = sizeof(CellFrame);
        break;}
      default:{
        Assert(0);
        break;}}
      break;
    }

  case Co_Port:  /* TODO: what to count TODO: no need for local check?? */
    {
      COUNT(port);
      switch(((Tertiary *)this)->getTertType()) {
      case Te_Local:{
        CheckLocal((PortLocal *) this);
        sz = sizeof(PortLocal);
        break;}

      case Te_Manager:{
        sz = sizeof(PortManager);
        break;}

      case Te_Proxy:{
        sz = sizeof(PortProxy);
        break;}

      default:{
        Assert(0);
        break;}}

      break;
    }
  case Co_Space:
    CheckLocal((Space *) this);
    sz = sizeof(Space);
    COUNT(space);
    break;

  case Co_Chunk:
    CheckLocal((SChunk *) this);
    sz = sizeof(SChunk);
    COUNT(chunk);
    break;

  case Co_Array:
    CheckLocal((OzArray *) this);
    sz = sizeof(OzArray);
    break;

  case Co_Dictionary:
    CheckLocal((OzDictionary *) this);
    sz = sizeof(OzDictionary);
    break;

  case Co_Lock:
    CheckLocal((OzLock *) this);
    sz = sizeof(OzLock);
    break;

  case Co_Thread:
    {
      Thread *th = ((Thread *)this)->gcThread();
      if (!th) th = ((Thread *)this)->gcDeadThread();
      return (ConstTerm *) th;
    }

  /* builtins are allocate dusing malloc */
  case Co_Builtin:
    return this;

  default:
    Assert(0);
    return 0;
  }
  ConstTerm *ret = (ConstTerm *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_CONSTTERM);
  storeForward(getGCField(), ret);
  return ret;
}

/* the purpose of this procedure is to provide an additional entry
   into gc so to be able to distinguish between owned perdio-objects that
   are locally accssible to those that are not - currently this is needed
   only for cell-frames */

ConstTerm *ConstTerm::gcConstTermSpec()
{
  GCMETHMSG("ConstTerm::gcConstTerm");
  CHECKCOLLECTED(*getGCField(), ConstTerm *);
  if(getType()!=Co_Cell){return gcConstTerm();}
  Tertiary *t=(Tertiary*)this;
  if(t->getTertType()!=Te_Frame){return gcConstTerm();}

  CellFrame *cf=(CellFrame*)t;
  cf->setAccessBit();
  COUNT(cell);
  size_t sz = sizeof(CellFrame);
  ConstTerm *ret = (ConstTerm *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_CONSTTERM);
  cf->myStoreForward(ret);
  return ret;
}

HeapChunk * HeapChunk::gc(void)
{
  GCMETHMSG("HeapChunk::gc");

  COUNT(heapChunk);
  HeapChunk * ret = (HeapChunk *) gcRealloc(this, sizeof(HeapChunk));

  ret->chunk_data = copyChunkData();

  GCNEWADDRMSG(ret);
  storeForward(getGCField(), ret);
  return ret;
}

/*
 * notification board == home board of thread
 * Although this may be discarded/failed, the solve actor must be announced.
 * Therefore this procedures searches for another living board.
 */
Board* Board::gcGetNotificationBoard()
{
  GCMETHMSG("Board::gcGetNotificationBoard");
  if (this == 0) return 0; // no notification board

  Board *bb = this->derefBoard();
  Board *nb = bb;
 loop:
  if (GCISMARKED(*bb->getGCField()) || bb->isRoot())  return nb;
  Assert(!bb->isCommitted());
  Actor *aa=bb->getActor();
  if (bb->isFailed() || aa->isCommitted()) {
    /*
     * notification board must be changed
     */
    bb=aa->getBoard();
    nb = bb;   // probably not dead;
    goto loop;
  }
  if (GCISMARKED(*aa->getGCField())) return nb;
  bb = aa->getBoard();
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

  if (bb->isFailed ()) return (NO);
  if (bb->isRoot () || GCISMARKED (*(bb->getGCField ()))) return (OK);

  aa=bb->getActor();
  if (aa->isCommitted ()) return (NO);
  if (GCISMARKED (*(aa->getGCField ()))) return (OK);
  bb = aa->getBoard();
  goto loop;
}

// This procedure derefences cluster chains and collects only the object at
// the end of such a chain.
Board * Board::gcBoard()
{
  GCMETHMSG("Board::gcBoard");
  if (!this) return 0;

  Board *bb = this->derefBoard();
  Assert(bb);

  CHECKCOLLECTED(*bb->getGCField(), Board *);
  if (!bb->gcIsAlive()) return 0;

  // kost@, TMUELLER
  // process.oz causes the assertion to fire!!!
  // Presumably a register allocation problem
  // See the cure below:
  //  if (opMode == IN_TC && !isInTree(bb)) return 0;

  // In an optimized machine the assertion is absent
  Assert(opMode != IN_TC || isInTree(bb));

  COUNT(board);
  size_t sz = sizeof(Board);
  Board *ret = (Board *) gcRealloc(bb,sz);

  FDPROFILE_GC(cp_size_board, sz);

  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_BOARD);
  storeForward(bb->getGCField(),ret);
  return ret;
}

void Board::gcRecurse()
{
  GCMETHMSG("Board::gcRecurse");
  Assert(!isCommitted() && !isFailed());
  body.gcRecurse();
  u.actor=u.actor->gcActor();

  script.Script::gc();
}

ObjectClass *ObjectClass::gcClass()
{
  if (this==0) return 0;

  GCMETHMSG("ObjectClass::gcClass");
  CHECKCOLLECTED(ToInt32(fastMethods), ObjectClass *);

  COUNT(objectClass);
  ObjectClass *ret = (ObjectClass *) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  OzDictionary *fastm = fastMethods;
  storeForward(&fastMethods, ret);
  ret->fastMethods    = (OzDictionary*) fastm->gcConstTerm();
  ret->slowMethods    = (OzDictionary*) slowMethods->gcConstTerm();
  ret->defaultMethods = (OzDictionary*) defaultMethods->gcConstTerm();
  ret->printName      = printName->gc();
  ret->send           = (Abstraction *) send->gcConstTerm();
  ret->unfreeFeatures = ret->unfreeFeatures->gcSRecord();
  gcTagged(ozclass,ret->ozclass);
  return ret;
}



Actor *Actor::gcActor()
{
  if (this==0) return 0;

  Assert(board->derefBoard()->gcIsAlive());

  GCMETHMSG("Actor::gc");
  CHECKCOLLECTED(*getGCField(), Actor *);
  // by kost@; flags are needed for getBoard
  size_t sz;
  if (isWait()) {
    COUNT(waitActor);
    sz = sizeof(WaitActor);
  } else if (isAsk () == OK) {
    COUNT(askActor);
    sz = sizeof(AskActor);
  } else {
    COUNT(solveActor);
    sz = sizeof (SolveActor);
  }
  Actor *ret = (Actor *) gcRealloc(this,sz);

  FDPROFILE_GC(isWait() ? cp_size_waitactor
                : (isAsk() ? cp_size_askactor
                   : cp_size_solveactor), sz);

  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_ACTOR);
  storeForward(getGCField(), ret);
  return ret;
}

void Actor::gcRecurse()
{
  GCMETHMSG("Actor::gcRecurse");
  if (isWait()) {
    ((AWActor *)this)->gcRecurse();
    ((WaitActor *)this)->gcRecurse();
  } else if (isAsk()) {
    ((AWActor *)this)->gcRecurse();
    ((AskActor *)this)->gcRecurse();
  } else {
    ((SolveActor *)this)->gcRecurse();
  }
}

void AWActor::gcRecurse()
{
  thread = thread->gcThread();
}

void WaitActor::gcRecurse()
{
  GCMETHMSG("WaitActor::gcRecurse");
  board = board->gcBoard();
  Assert(board);

  next.gcRecurse ();

  int32 num = ToInt32(children[-1]);
  COUNT1(waitChild,num);
  Board **newChildren=(Board **) heapMalloc((num+1)*sizeof(Board *));

  FDPROFILE_GC(cp_size_waitactor, (num+1)*sizeof(Board *));

  *newChildren++ = (Board *) num;
  for (int i=0; i < num; i++) {
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

void AskActor::gcRecurse () {
  GCMETHMSG("AskActor::gcRecurse");
  next.gcRecurse ();
  board = board->gcBoard ();
  Assert(board);
}

void SolveActor::gcRecurse () {
  GCMETHMSG("SolveActor::gcRecurse");
  if (opMode == IN_GC || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
    Assert(board);
  }
  solveBoard = solveBoard->gcBoard();
  Assert(solveBoard);

  if (opMode == IN_GC || !isGround())
    gcTagged(solveVar, solveVar);

  gcTagged(result, result);
  suspList         = suspList->gc();
  cpb              = cpb->gc();
  localThreadQueue = localThreadQueue->gc();
}

CpBag * CpBag::gc(void) {
  GCMETHMSG("CpBag::gc");

  CpBag *  copy = (CpBag *) 0;
  CpBag ** cur  = &copy;
  CpBag *  old  = this;

  while (old) {
    WaitActor * wa = old->choice;

    if (wa && wa->isAliveUpToSolve() && wa->getBoard()->gcIsAlive()) {
      Assert(!(opMode == IN_TC && !isInTree(wa->getBoard())));

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

/* collect LTuple */

inline
void gcTaggedBlockRecurse(TaggedRef *block,int sz)
{
  for(int i = sz-1; i>=0; i--) {
    if (!isDirectVar(block[i])) {
      gcTagged(block[i],block[i]);
    }
  }
}


inline
void SRecord::gcRecurse()
{
  GCMETHMSG("SRecord::gcRecurse");
  gcTagged(label,label);
  gcTaggedBlockRecurse(getRef(),getWidth());
}


inline
void LTuple::gcRecurse()
{
  GCMETHMSG("LTuple::gcRecurse");
  gcTaggedBlockRecurse(args,2);
}



void performCopying(void)
{
  while (!ptrStack.isEmpty()) {
    TypedPtr tptr     = ptrStack.pop();
    void *ptr         = getPtr(tptr);
    TypeOfPtr ptrType = getType(tptr);

    switch(ptrType) {

    case PTR_LTUPLE:    ((LTuple *) ptr)->gcRecurse();           break;
    case PTR_SRECORD:   ((SRecord *) ptr)->gcRecurse();          break;
    case PTR_NAME:      ((Name *) ptr)->gcRecurse ();            break;
    case PTR_BOARD:     ((Board *) ptr)->gcRecurse();            break;
    case PTR_ACTOR:     ((Actor *) ptr)->gcRecurse();            break;
    case PTR_THREAD:    ((Thread *) ptr)->gcRecurse();           break;
    case PTR_CONT:      ((Continuation*) ptr)->gcRecurse();      break;
    case PTR_PROPAGATOR:((OZ_Propagator *) ptr)->updateHeapRefs(opMode == IN_GC); break;
    case PTR_DYNTAB:    ((DynamicTable *) ptr)->gcRecurse();     break;
    case PTR_CONSTTERM: ((ConstTerm *) ptr)->gcConstRecurse();   break;
    default:
      Assert(NO);
    }
  }
}




//*****************************************************************************
//             AM methods to launch gc under certain conditions
//*****************************************************************************


// signal handler
void checkGC()
{
  Assert(!am.isCritical());
  if (getUsedMemory() > unsigned(ozconf.heapThreshold) && ozconf.gcFlag) {
    am.setSFlag(StartGC);
  }
}

void AM::doGC()
{
  osBlockSignals();
  ThreadList::dispose();
  Assert(isToplevel());

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

OzDebug *OzDebug::gcOzDebug()
{
  gcTagged(info,info);
  return this;
}

// special purpose to gc borrowtable entry which is a variable
TaggedRef gcTagged1(TaggedRef in) {
  TaggedRef x=deref(in);
  Assert(GCISMARKED(x));
  return makeTaggedRef((TaggedRef*)GCUNMARK(x));
}
