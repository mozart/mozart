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

#include <ctype.h>

#include "am.hh"

#include "gc.hh"

#include "dllist.hh"

#include "genvar.hh"
#include "ofgenvar.hh"
#include "fdhook.hh"
#include "fdprofil.hh"

#include "verbose.hh"

#ifdef OUTLINE
#define inline
#endif

/****************************************************************************
 * MACROS
 ****************************************************************************/

/* collect a pointer */
#define GCREF(field) if (field) { field = field->gc(); }


/****************************************************************************
 *               Forward Declarations
 ****************************************************************************/

static void processUpdateStack (void);
void gcTagged(TaggedRef &fromTerm, TaggedRef &toTerm);
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
    ThreadList *aux = allthreads;
    while(aux) {
      aux->elem->printDebug(NULL,NO);
      aux = aux->next;
    }
  }

};

ThreadList *ThreadList::allthreads = NULL;


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

typedef enum {IN_GC = 0, IN_TC} GcMode;

GcMode opMode;

/*
 * TC: groundness check needs to count the number of
 *    variables, names, cells & abstractions
 */
static int varCount;


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
  CHECK_POINTER(s);
/*  DebugGCT(extern MemChunks * from);
  DebugGC(gcing == 0 && !from->inChunkChain ((void *)s),
          error ("making TaggedRef pointing to 'to' space"));
          */
  return (TaggedRef) ToInt32(s);
}

//*****************************************************************************
//                        Consistency checks of trails
//*****************************************************************************

inline
void RebindTrail::gc()
{
  Assert(isEmpty());
}


// cursor points to next free position
inline
void Trail::gc()
{
  Assert(isEmpty());
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
  PTR_STUPLE,
  PTR_NAME,
  PTR_BOARD,
  PTR_ACTOR,
  PTR_THREAD,
  PTR_CONT,
  PTR_CFUNCONT,
  PTR_SUSPCONT,
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
  TypedPtrStack() : Stack() {}
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
  SavedPtrStack() : Stack() {}
  ~SavedPtrStack() {}
  void pushPtr(int32* ptr, int32 value)
  {
    ensureFree(2);
    push(ptr,NO);
    push((StackEntry) value,NO);
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
void storeForward (int32* fromPtr, void *newValue)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(fromPtr, (int32) *fromPtr);
  }
  DebugGC(opMode == IN_GC
          && MemChunks::list->inChunkChain((void *)fromPtr),
          error("storing marked value in 'TO' space"));
  DebugGC(opMode == IN_GC
          && from->inChunkChain(newValue),
          error("storing (marked) ref in to FROM-space"));
  *fromPtr = GCMARK(newValue);
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
Bool needsNoCollection(TaggedRef t)
{
  Assert(t != makeTaggedNULL());

  TypeOfTerm tag = tagTypeOf(t);
  return isSmallInt(tag) ||
         isLiteral(tag) && !tagged2Literal(t)->isDynName();
}


Bool gcProtect(TaggedRef *ref)
{
  if (needsNoCollection(*ref))
    return OK;

  if (extRefs->find(ref))
    return NO;

  extRefs = (ExtRefNode*) extRefs->add(ref);
  return OK;
}

Bool gcUnprotect(TaggedRef *ref)
{
  ExtRefNode *aux = (ExtRefNode*) extRefs->find(ref);

  if (aux == NULL)
    return needsNoCollection(*ref);

  extRefs = (ExtRefNode*) extRefs->remove(aux);
  return OK;
}


/****************************************************************************
 * The update stack contains REF's to not yet copied variables
 ****************************************************************************/


DebugCheckT(static int updateStackCount = 0;)

class UpdateStack: public Stack {
public:
  UpdateStack() : Stack() {}
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
    bb = bb->getParentFast();
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
    bb = bb->getParentFast();
    bb->unsetPathMark();
  } while (!bb->isRoot());
}


/*
 * Check if an entry of a suspension list is local to the copyBoard.
 */
inline
Bool isInTree(Board *b)
{
  Assert(opMode == IN_TC);
  Assert(b);
 loop:
  Assert(!b->isCommitted());
  if (b == fromCopyBoard) return OK;
  if (!isLocalBoard(b)) return NO;
  b = b->getParentAndTest();
  if (!b) return NO;
  goto loop;
}

/****************************************************************************
 * Collect all types of terms
 ****************************************************************************/

/*
 * Literals:
 *   forward in 'printName'
 * NOTE:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */
inline
Literal *Literal::gc()
{
  if (!isDynName()) return (this);

  if (opMode == IN_GC || isLocalBoard(getBoardFast())) {
    GCMETHMSG("Literal::gc");
    CHECKCOLLECTED(ToInt32(printName), Literal *);
    varCount++;
    Name *aux = (Name *) gcRealloc (this,sizeof (*this));
    GCNEWADDRMSG (aux);
    ptrStack.push (aux, PTR_NAME);
    storeForward(&printName, aux);

    PROFILE_CODE1(if (opMode == IN_TC) {
                    FDProfiles.inc_item(cp_size_literal, sizeof(*this));
                  })

    return (aux);
  } else {
    return (this);
  }
}

inline
void Literal::gcRecurse ()
{
  GCMETHMSG("Literal::gcRecurse");
  DebugGC((isDynName() == NO),
          error ("non-dynamic name is found in gcRecurse"));
  home = home->gcBoard();
  Assert(home);
}


/*
 * Float
 * WARNING: the value field of floats has no bit left for a gc mark
 *   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
 */
inline
Float *Float::gc()
{
  Float *ret =  newFloat(value);
  return ret;
}

inline
BigInt *BigInt::gc()
{
  CHECKCOLLECTED(*(int *)&value.d, BigInt *);

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
    Equation *aux = (Equation*)gcRealloc(first,sz);
    GCNEWADDRMSG(aux);

    PROFILE_CODE1(if (opMode == IN_TC) {
                    FDProfiles.inc_item(cp_size_script, sz);
                  })

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

inline Bool refsArrayIsMarked(RefsArray r)
{
  return GCISMARKED(r[-1]);
}

inline void refsArrayMark(RefsArray r, void *ptr)
{
  storeForward((int32*)&r[-1],ptr);
}

inline RefsArray refsArrayUnmark(RefsArray r)
{
  return (RefsArray) GCUNMARK(r[-1]);
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

  RefsArray aux = allocateRefsArray(sz,NO);
  GCNEWADDRMSG(aux);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_refsarray,
                                      (sz + 1) * sizeof(TaggedRef));
                })

    if (isDirtyRefsArray(r)) {
    markDirtyRefsArray(aux);
  }

  DebugCheck(isFreedRefsArray(r),
             markFreedRefsArray(aux););

  refsArrayMark(r,aux);

  for(int i = sz-1; i >= 0; i--)
    gcTagged(r[i],aux[i]);

  return aux;
}

void CFuncContinuation::gcRecurse(void)
{
  GCMETHMSG("CFuncContinuation::gcRecurse");
  DebugCheck (isFreedRefsArray (xRegs),
              error ("freed refs array in CFunContinuation::gcRecurse ()"));
  xRegs = gcRefsArray(xRegs);
  board = board->gcBoard();
  Assert(board);
}

/* mm2: have to check for discarded node */
inline
CFuncContinuation *CFuncContinuation::gcCont(void)
{
  GCMETHMSG("CFuncContinuation::gcCont");
  CHECKCOLLECTED(ToInt32(cFunc), CFuncContinuation *);

  CFuncContinuation *ret = (CFuncContinuation*) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CFUNCONT);
  storeForward(&cFunc, ret);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_cfunccont, sizeof(*this));
                })

  return ret;
}

Continuation *Continuation::gc()
{
  GCMETHMSG("Continuation::gc");
  CHECKCOLLECTED(ToInt32(pc), Continuation *);

  Continuation *ret = (Continuation *) gcRealloc(this,sizeof(Continuation));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CONT);
  storeForward(&pc, ret);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_cont, sizeof(Continuation));
                })
  return ret;
}

inline
void Continuation::gcRecurse(){
  GCMETHMSG("Continuation::gcRecurse");
  Assert(!isFreedRefsArray (yRegs));
  yRegs = gcRefsArray(yRegs);
  Assert(!isFreedRefsArray (gRegs));
  gRegs = gcRefsArray(gRegs);
  Assert(!isFreedRefsArray (xRegs));
  xRegs = gcRefsArray(xRegs);
}

inline
SuspContinuation *SuspContinuation::gcCont()
{
  GCMETHMSG("SuspContinuation::gcCont");

  // a special continuation for solve: FAILURE
  if (pc != NOCODE) {
    CHECKCOLLECTED(ToInt32(pc), SuspContinuation *)
  }

  SuspContinuation *ret = (SuspContinuation*) gcRealloc(this, sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_SUSPCONT);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_suspcont, sizeof(*this));
                });

  Assert(opMode != IN_TC || isInTree(board->getBoardFast()));

  storeForward(&pc, ret);
  return ret;
}

inline
void SuspContinuation::gcRecurse(){
  GCMETHMSG("SuspContinuation::gcRecurse");
  board = board->gcBoard();
  Assert(board);
  Continuation::gcRecurse();
}


/* collect STuple, LTuple, SRecord */

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
STuple *STuple::gc()
{
  GCMETHMSG("STuple::gc");
  CHECKCOLLECTED(label, STuple *);

  int len = (size-1)*sizeof(TaggedRef)+sizeof(STuple);

  STuple *ret = (STuple*) gcRealloc(this,len);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_STUPLE);
  storeForward((int *) &label, ret);
  gcTaggedBlock(getRef(),ret->getRef(),getSize());
  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_stuple, len);
                })
  return ret;
}


inline
LTuple *LTuple::gc()
{
  GCMETHMSG("LTuple::gc");
  CHECKCOLLECTED(args[0], LTuple *);

  LTuple *ret = (LTuple*) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_LTUPLE);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_ltuple, sizeof(*this));
                })

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

  int len = (getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord);

  SRecord *ret = (SRecord*) gcRealloc(this,len);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_SRECORD);
  storeForward((int32*)&label, ret);
  gcTaggedBlock(getRef(),ret->getRef(),getWidth());

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_record, len);
                })

  return ret;
}

/* return NULL if contains pointer to discarded node */
inline
Suspension *Suspension::gcSuspension(Bool tcFlag)
{
  GCMETHMSG("Suspension::gcSuspension");
  if (this == 0) return 0;

  CHECKCOLLECTED(ToInt32(item.cont), Suspension*);

  if (isDead()) return 0;

  Board *bb=getBoardFast();
  if (!bb->gcIsAlive()) {
    // mm2 warning("gcSuspension: dead\n");
    return 0;
  }

  if (tcFlag && !isInTree(bb)) return 0;

  Assert(!tcFlag || !isPropagated());

  Suspension *newSusp = (Suspension *) gcRealloc(this, sizeof(*this));
  GCNEWADDRMSG(newSusp);

  PROFILE_CODE1(if (opMode == IN_TC) {
                  FDProfiles.inc_item(cp_size_susp, sizeof(*this));
                })

  if (flag & S_thread) {
    newSusp->item.thread = item.thread->gcThread();
  } else {
    switch (flag & (S_cont|S_cfun)){
    case S_null:
      newSusp->item.board = item.board->gcBoard();
      Assert(newSusp->item.board);
      break;
    case S_cont:
      newSusp->item.cont = item.cont->gcCont();
      Assert(newSusp->item.cont);
      break;
    case S_cont|S_cfun:
      newSusp->item.ccont = item.ccont->gcCont();
      Assert(newSusp->item.ccont);
      break;
    default:
      Assert(0);
    }
  }

  storeForward(&item.cont, newSusp);
  return newSusp;
}


/* we reverse the order of the list,
 * but this should be no problem
 */
SuspList * SuspList::gc(Bool tcFlag)
{
  GCMETHMSG("SuspList::gc");

  SuspList *ret = NULL;

  for(SuspList* help = this; help != NULL; help = help->next) {
    Suspension *aux = help->getSusp()->gcSuspension(tcFlag);
    if (!aux) {
      continue;
    }
    ret = new SuspList(aux, ret);

    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_susplist, sizeof(SuspList));
    })
  }
  GCNEWADDRMSG (ret);
  return (ret);
}

inline
void GenCVariable::gc(void)
{
  Assert(getType() == FDVariable ||
         getType() == OFSVariable ||
         getType() == MetaVariable ||
         getType() == BoolVariable ||
         getType() == AVAR
         );
  switch (getType()){
  case FDVariable:
    ((GenFDVariable*)this)->gc();
    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_fdvar, sizeof(GenFDVariable));
    });
    break;
  case OFSVariable:
    ((GenOFSVariable*)this)->gc();
    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_ofsvar, sizeof(GenOFSVariable));
    });
    break;
  case MetaVariable:
    ((GenMetaVariable*)this)->gc();
    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_metavar, sizeof(GenMetaVariable));
    });
    break;
  case BoolVariable:
    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_boolvar, sizeof(GenBoolVariable));
    });
    break;
  case AVAR:
    ((AVar *) this)->gc();
    break;
  default:
    break;
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

    PROFILE_CODE1(if (opMode == IN_TC) {
      FDProfiles.inc_item(cp_size_svar, cv_size);
    });

    storeForward(&cv->suspList, new_cv);

    if (opMode == IN_TC && new_cv->getBoardFast () == fromCopyBoard)
      new_cv->suspList = new_cv->suspList->gc(OK);
    else
      new_cv->suspList = new_cv->suspList->gc(NO);
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

  GenCVariable *new_gv = (GenCVariable*)gcRealloc(gv, gv_size);

  storeForward(&gv->suspList, new_gv);

  if (opMode == IN_TC && new_gv->getBoardFast () == fromCopyBoard)
    new_gv->suspList = new_gv->suspList->gc(OK);
  else
    new_gv->suspList = new_gv->suspList->gc(NO);
  new_gv->gc();

  Assert(opMode != IN_GC || new_gv->home != bb);

  new_gv->home = bb;
  GCNEWADDRMSG(makeTaggedCVar(new_gv));
  return makeTaggedCVar(new_gv);
}


inline
void FiniteDomain::gc(void)
{
#if defined(DEBUG_CHECK) && defined(DEBUG_FD)
  Assert(isConsistent());
#endif

  PROFILE_CODE1(if (opMode == IN_TC) {
    FDProfiles.inc_item(cp_size_fdvar, getDescrSize());
  });

  copyExtension();
}

void GenFDVariable::gc(void)
{
  GCMETHMSG("GenFDVariable::gc");
  finiteDomain.gc();

  int i;
  if (opMode == IN_TC && getBoardFast() == fromCopyBoard)
    for (i = fd_any; i--; )
      fdSuspList[i] = fdSuspList[i]->gc(OK);
  else
    for (i = fd_any; i--; )
      fdSuspList[i] = fdSuspList[i]->gc(NO);
}


void GenMetaVariable::gc(void)
{
  GCMETHMSG("GenMetaVariable::gc");
  gcTagged(data, data);
}

void AVar::gc(void)
{
  GCMETHMSG("AVar::gc");
  gcTagged(value, value);
}

DynamicTable* DynamicTable::gc(void)
{
    GCMETHMSG("DynamicTable::gc");

    Assert(isPwrTwo(size));
    // Copy the table:
    size_t len = (size-1)*sizeof(HashElement)+sizeof(DynamicTable);
    DynamicTable* ret = (DynamicTable*) gcRealloc(this,len);

    PROFILE_CODE1(if (opMode == IN_TC) {
                     FDProfiles.inc_item(cp_size_ofsvar, len);
                   })

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
    return tagged2VarHome(var)->getBoardFast();
  }
  if (isSVar(var)) {
    return tagged2SVar(var)->getBoardFast();
  }
  Assert(isCVar(var));
  return tagged2CVar(var)->getBoardFast();
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
  case LITERAL:  toTerm = makeTaggedLiteral(tagged2Literal(auxTerm)->gc()); break;
  case LTUPLE:   toTerm = makeTaggedLTuple(tagged2LTuple(auxTerm)->gc()); break;
  case STUPLE:   toTerm = makeTaggedSTuple(tagged2STuple(auxTerm)->gc()); break;
  case SRECORD:  toTerm = makeTaggedSRecord(tagged2SRecord(auxTerm)->gcSRecord()); break;
  case BIGINT:   toTerm = makeTaggedBigInt(tagged2BigInt(auxTerm)->gc()); break;
  case FLOAT:    toTerm = makeTaggedFloat(tagged2Float(auxTerm)->gc());   break;

  case CONST:
    {
      ConstTerm *con=tagged2Const(auxTerm)->gcConstTerm();
      toTerm = con ? makeTaggedConst(con) : nil();
    }
    break;

  case SVAR:
  case UVAR:
  case CVAR:
    varCount++;
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

  ozstat.initGcMsg(msgLevel);

  MemChunks *oldChain = MemChunks::list;

  VariableNamer::cleanup();  /* drop bound variables */

  INITCHECKSPACE;
  initMemoryManagement();

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

  trail.gc();
  rebindTrail.gc();

  rootBoard = rootBoard->gcBoard();   // must go first!
  Assert(rootBoard);
  setCurrent(currentBoard->gcBoard(),NO);

  GCPROCMSG("Predicate table");
  CodeArea::gc();

  aritytable.gc ();
  ThreadsPool::doGC ();
  Assert(rootThread);

  if (FDcurrentTaskSusp != (Suspension *) NULL) {
    warning("FDcurrentTaskSusp must be NULL!");
    FDcurrentTaskSusp = FDcurrentTaskSusp->gcSuspension (NO);
  }
#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc(NO);
#endif

  suspendVarList=makeTaggedNULL(); /* no valid data */
  gcTagged(suspCallHandler,suspCallHandler);

  gcTagged(aVarUnifyHandler,aVarUnifyHandler);
  gcTagged(aVarBindHandler,aVarBindHandler);

  GCPROCMSG("ioNodes");
  for(int i = 0; i < osOpenMax(); i++) {
    for(int mode=SEL_READ; mode <= SEL_WRITE; mode++) {
      if (osIsWatchedFD(i,mode) && i != compStream->csfileno()) {
        TaggedRef &t = ioNodes[i].readwritepair[mode];
        gcTagged(t,t);
        if (t == makeTaggedNULL()) {
          osClrWatchedFD(i,mode);
          DebugCheckT(warning("selectNode discarded/failed"));
        }
      }
    }
  }
  performCopying();

  GCPROCMSG("toplevelVars");
  am.toplevelVars = gcRefsArray(am.toplevelVars);

  GCPROCMSG("updating external references to terms into heap");
  ExtRefNode::gc();

  PROFILE_CODE1(FDProfiles.gc());

  performCopying();

// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  GCPROCMSG("updating references");
  processUpdateStack ();

  if(!ptrStack.isEmpty())
    error("ptrStack should be empty");

  EXITCHECKSPACE;

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  ozstat.printGcMsg(msgLevel);

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
          break;
        case SVAR:
          *tt = makeTaggedRef(newTaggedSVar(tagged2SVar(newVar)));
          break;
        case CVAR:
          *tt = makeTaggedRef(newTaggedCVar(tagged2CVar(newVar)));
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
Board* AM::copyTree (Board* bb, Bool *isGround)
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
  varCount = 0;
  unsigned int starttime = osUserTime();

  Assert(!bb->isCommitted());
  fromCopyBoard = bb;
  setPathMarks(fromCopyBoard);
  toCopyBoard = fromCopyBoard->gcBoard();
  Assert(toCopyBoard);

  // kost@ : FDcurrentTaskSusp ??? mm2

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
    if (varCount == 0)
      *isGround = OK;
    else
      *isGround = NO;
  }

  PROFILE_CODE1(FDProfiles.setBoard(toCopyBoard);)

  return toCopyBoard;
}

//*****************************************************************************
//                                GC-METHODS
//*****************************************************************************

inline void Arity::gc()
{
  Arity *aux = this;
  while(aux) {
    GCMETHMSG("Arity::gc");
    for (int i = 0; i < aux->size; i++) {
      if (aux->keytable[i] != NULL) {
        aux->keytable[i] = aux->keytable[i]->gc ();
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

inline void AbstractionEntry::gcAbstractionEntry()
{
  abstr = (Abstraction *) abstr->gcConstTerm();
  if (abstr == NULL) {
    DebugGCT(warning("abstraction entry dead\n")); // mm2;
    g = 0;
    return;
  }
  g = gcRefsArray(g);
}

inline void AbstractionTable::gcAbstractionTable()
{
  // there may be NULL entries in the table during gc

  GCMETHMSG("AbstractionTable::gc");

  defaultEntry.gcAbstractionEntry();

  HashNode *hn = getFirst();
  for (; hn != NULL; hn = getNext(hn)) {
    ((AbstractionEntry*) hn->value)->gcAbstractionEntry();
  }
}

void CodeArea::gc()
{
  abstractionTab.gcAbstractionTable();
}

void ThreadsPool::doGC ()
{
  int pri, prioInd = nextPrioInd;
  ThreadQueue *thq;

  currentThread = currentThread->gcThread ();
  rootThread = rootThread->gcThread ();
  threadsFreeList = NULL;

  thq = currentQueue;
  pri = currentPriority;
  while (thq) {
    thq->doGC ();

    if (prioInd >= 0) {
      pri = nextPrio[prioInd--];
      thq = &queues[pri];
    } else {
      thq = (ThreadQueue *) NULL;
    }
  }
}

void ThreadQueue::doGC ()
{
  int asize = size;
  int ahead = head;
  int mod = maxsize - 1;

  while (asize) {
    queue[ahead] = queue[ahead]->gcThread ();
    ahead = (ahead + 1) & mod;
    asize--;
  }
}

void TaskStack::gc(TaskStack *newstack)
{
  /* allocate new stack and save reference for
   * TaskStack::gcRecurse on the new stack
   */
  newstack->allocate(getMaxSize(),heapMalloc);
  newstack->push(this);
}

void TaskStack::gcRecurse()
{
  GCMETHMSG("TaskStack::gc");
  TaskStack *oldstack = (TaskStack *) pop();

  gcInit();
  TaskStackEntry *savedTop=oldstack->getTop();

  while (!oldstack->isEmpty()) {
    TaskStackEntry oldEntry=oldstack->pop();
    TaggedBoard tb = (TaggedBoard) ToInt32(oldEntry);
    ContFlag cFlag = getContFlag(tb);
    if (cFlag == C_COMP_MODE) {
      gcQueue(oldEntry);
      continue;
    }

    Board *newBB = getBoard(tb, cFlag)->gcBoard();
    if (!newBB) {
      switch (cFlag){
      case C_NERVOUS:    continue;
      case C_COMP_MODE:  Assert(0);
      case C_XCONT:      oldstack->pop(4); continue;
      case C_CONT:       oldstack->pop(3); continue;
      case C_DEBUG_CONT: oldstack->pop(1); continue;
      case C_CFUNC_CONT: oldstack->pop(3); continue;
      case C_CALL_CONT:  oldstack->pop(2); continue;
      }
    }

    gcQueue((TaskStackEntry) setContFlag(newBB,cFlag));

    switch (cFlag){

    case C_NERVOUS:
      break;

    case C_COMP_MODE:
      Assert(0);
      break;

    case C_CONT:
      gcQueue(oldstack->pop());                           // PC
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));  // Y
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));  // G
      break;

    case C_XCONT:
      gcQueue(oldstack->pop());                           // PC
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));  // Y
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));  // G
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));  // X
      break;

    case C_DEBUG_CONT:
      gcQueue(((OzDebug *) oldstack->pop())->gcOzDebug());
      break;

    case C_CALL_CONT:
      gcQueue(((SRecord *) oldstack->pop())->gcSRecord());
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));
      break;

    case C_CFUNC_CONT:
      gcQueue(oldstack->pop());                // OZ_CFun
      gcQueue(((Suspension*) oldstack->pop())->gcSuspension());
      gcQueue(gcRefsArray((RefsArray) oldstack->pop()));
      break;

    default:
      error("Unexpected case in TaskStack::gc().");
      break;
    }
  } // while not task stack is empty

  gcEnd();
  oldstack->setTop(savedTop);
} // TaskStack::gc



//*********************************************************************
//                           NODEs
//*********************************************************************

void Chunk::gcRecurse()
{
  setRecord(getRecord()->gcSRecord());
  switch(getType()) {

  case Co_Object:
    {
      Object *o = (Object *) this;
      Bool isc = o->getIsClass();
      o->claas = o->getClass()->gcClass();
      gcTagged(o->cell,o->cell);
      if (o->getIsClass())
        o->setIsClass();
      if (o->isDeep()){
        ((DeepObject*)o)->home = o->getBoardFast()->gcBoard();
      }
      break;
    }

  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
      Assert(!isFreedRefsArray(a->gRegs));
      a->gRegs = gcRefsArray(a->gRegs);

      a->home = a->home->gcBoard();
      varCount++;

      DebugGC((a->home == (Board *) ToPointer(ALLBITS) ||
               a->home == NULL),
              error ("non-dynamic name is met in Abstraction::gcRecurse"));
      INTOSPACE(a->home);
      break;
    }

  case Co_Cell:
    {
      Cell *c = (Cell *) this;

      c->home = c->home->gcBoard();
      varCount++;

      gcTagged(c->val,c->val);
      break;
    }

  case Co_Builtin:
    {
      Builtin *bi = (Builtin *) this;
      gcTagged(bi->suspHandler,bi->suspHandler);
      Assert(!isFreedRefsArray (bi->gRegs));
      bi->gRegs = gcRefsArray(bi->gRegs);
      break;
    }

  default:
    Assert(0);
  }
}


void ConstTerm::gcConstRecurse()
{
  Assert(isConstChunk(this));
  ((Chunk *) this)->gcRecurse();
}


ConstTerm *ConstTerm::gcConstTerm()
{
  GCMETHMSG("ConstTerm::gcConstTerm");
  if (this == NULL) return NULL;
  CHECKCOLLECTED(*getGCField(), ConstTerm *);

  switch (typeOf()) {
  case Co_Board:     return ((Board *) this)->gcBoard();
  case Co_Actor:     return ((Actor *) this)->gcActor();
  case Co_Thread:    return ((Thread *) this)->gcThread();
  case Co_Class:     return ((ObjectClass *) this)->gcClass();
  case Co_HeapChunk: return ((HeapChunk *) this)->gc();

  default:
    {
      Assert(isConstChunk(this));
      size_t sz;
      switch(((Chunk*) this)->getType()) {
      case Co_Abstraction: {
        Abstraction *a = (Abstraction *) this;
        Board *bb=a->getBoardFast();
        if (!bb->gcIsAlive()) return 0;
        if (opMode == IN_TC && !isLocalBoard(bb)) return this;
        sz = sizeof(Abstraction);
//      DebugGCT(if (opMode == IN_GC) NOTINTOSPACE(bb));
        break;
      }
      case Co_Object:
        sz = ((Object*)this)->isDeep() ? sizeof(DeepObject): sizeof(Object);
        break;
      case Co_Cell:
        {
          Board *bb=((Cell *) this)->getBoardFast();
          if (!bb->gcIsAlive()) return 0;
          if (opMode == IN_TC && !isLocalBoard(bb)) return this;
          sz = sizeof(Cell);
        }
        break;
      case Co_Builtin:
        switch (((Builtin *) this)->getType()) {
        case BIsolveCont:
          sz = sizeof(OneCallBuiltin);
          break;
        case BIsolved:
          sz = sizeof(SolvedBuiltin);
          break;
        default:
          sz = sizeof(Builtin);
        }
        break;
      default:
        Assert(0);
        return 0;
      }
      Chunk *ret = (Chunk*) gcRealloc(this,sz);
      GCNEWADDRMSG(ret);
      ptrStack.push(ret,PTR_CONSTTERM);
      storeForward(getGCField(), ret);
      return ret;
    }
  }
}


HeapChunk * HeapChunk::gc(void)
{
  GCMETHMSG("HeapChunk::gc");

  HeapChunk * ret = (HeapChunk *) gcRealloc(this, sizeof(HeapChunk));

  ret->chunk_data = copyChunkData();

  GCNEWADDRMSG(ret);
  storeForward(getGCField(), ret);
  return ret;
}

/*
 * NOTE: discarded threads must survive gc, because the solve counter
 *       has to be updated.
 * NOTE: if threads may be discarded,
 *       then check for prev, next and threadsHead, threadsTail
 */
Thread *Thread::gcThread()
{
  GCMETHMSG("Thread::gc");
  if (this==0) return 0;
  CHECKCOLLECTED(*getGCField(), Thread *);

#ifdef DEBUG_GC
  //mm2
  Board *bb=home?getBoardFast():0;
  if (!bb || !bb->gcIsAlive()) {
    warning("discarded thread detected ...");
  }
#endif

  size_t sz = sizeof(Thread);
  Thread *ret = (Thread *) gcRealloc(this,sz);
  ThreadList::add(ret);
  taskStack.gc(&ret->taskStack);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_THREAD);
  storeForward(getGCField(), ret);
  return ret;
}

void Thread::gcThreadRecurse()
{
  GCMETHMSG("Thread::gcRecurse");

  Board *newHome = home->gcBoard();
#ifdef NEWCOUNTER
  if (!newHome) {
    newHome=home->gcGetNotificationBoard()->gcBoard();
    //  virtually the new thread, because in LBLkillThread a suspension
    // counter will be decremented for 'newHome';
    newHome->incSuspCount ();
  }
#endif
  home=newHome;
  // Assert(home);

  taskStack.gcRecurse();
  DebugCheckT(Board *sob = notificationBoard);
  notificationBoard = (notificationBoard->gcGetNotificationBoard ())->gcBoard();
  Assert(sob == (Board *) NULL || notificationBoard != (Board *) NULL);
}

/*
 * notification board == home board of thread
 * Although this may be discarded/failed, the solve actor must be announced.
 * Therefor this procedures searches for another living board.
 */
Board* Board::gcGetNotificationBoard()
{
  GCMETHMSG("Board::gcGetNotificationBoard");
  if (this == 0) return 0; // no notification board

  Board *bb = this->getBoardFast();
  Board *nb = bb;
 loop:
  if (GCISMARKED(*bb->getGCField()) || bb->isRoot())  return nb;
  Assert(!bb->isCommitted());
  Actor *aa=bb->getActor();
  if (bb->isFailed() || aa->isCommitted()) {
    /*
     * notification board must be changed
     */
    bb=aa->getBoardFast();
    nb = bb;   // probably not dead;
    goto loop;
  }
  if (GCISMARKED(*aa->getGCField())) return nb;
  bb = aa->getBoardFast();
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
  Board *bb=this;
 loop:
  Assert(!bb->isCommitted());
  if (bb->isFailed()) return NO;
  if (bb->isRoot() || GCISMARKED(*(bb->getGCField()))) return OK;
  Actor *aa=bb->getActor();
  if (aa->isCommitted()) return NO;
  if (GCISMARKED(*(aa->getGCField()))) return OK;
  if (opMode == IN_TC && aa->isWait() && WaitActor::Cast(aa)->hasUnsetBoard())
    return NO;
  bb=aa->getBoardFast();
  goto loop;
}

// This procedure derefences cluster chains and collects only the object at
// the end of such a chain.
Board *Board::gcBoard()
{
  GCMETHMSG("Board::gcBoard");
  if (!this) return 0;

  Board *bb = this->getBoardFast();
  Assert(bb);

  CHECKCOLLECTED(*bb->getGCField(), Board *);
  if (!bb->gcIsAlive()) return 0;

  Assert(opMode != IN_TC || isInTree(bb));

  size_t sz = sizeof(Board);
  Board *ret = (Board *) gcRealloc(bb,sz);

  PROFILE_CODE1(if (opMode == IN_TC) {
                   FDProfiles.inc_item(cp_size_board, sz);
                 })

  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_BOARD);
  storeForward(bb->getGCField(),ret);
  return ret;
}

void Board::gcRecurse()
{
  GCMETHMSG("Board::gcRecurse");
  Assert(!isCommitted() && !isFailed());
  Assert(!isFreedRefsArray(body.getY ()));
  body.gcRecurse();
  u.actor=u.actor->gcActor();

  script.Script::gc();
}

ObjectClass *ObjectClass::gcClass()
{
  if (this==0) return 0;

  GCMETHMSG("ObjectClass::gcClass");
  CHECKCOLLECTED(*getGCField(), ObjectClass *);

  ObjectClass *ret = (ObjectClass *) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  storeForward(getGCField(), ret);
  ret->fastMethods = fastMethods->gcSRecord();
  ret->printName = printName->gc();
  gcTagged(slowMethods,ret->slowMethods);
  ret->send = (Abstraction *) send->gcConstTerm();
  ret->unfreeFeatures = ret->unfreeFeatures->gcSRecord();
  gcTagged(ozclass,ret->ozclass);
  return ret;
}



Actor *Actor::gcActor()
{
  if (this==0) return 0;

  Assert(board->getBoardFast()->gcIsAlive());

  GCMETHMSG("Actor::gc");
  CHECKCOLLECTED(*getGCField(), Actor *);
  // by kost@; flags are needed for getBoardFast
  size_t sz;
  if (isWait()) {
    sz = sizeof(WaitActor);
  } else if (isAsk () == OK) {
    sz = sizeof(AskActor);
  } else {
    sz = sizeof (SolveActor);
  }
  Actor *ret = (Actor *) gcRealloc(this,sz);

  PROFILE_CODE1(if (opMode == IN_TC) {
                   FDProfiles.inc_item(isWait() ? cp_size_waitactor
                                       : (isAsk() ? cp_size_askactor
                                          : cp_size_solveactor), sz);
                 })

  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_ACTOR);
  storeForward(getGCField(), ret);
  return ret;
}

void Actor::gcRecurse()
{
  GCMETHMSG("Actor::gcRecurse");
  if (isWait()) {
    ((WaitActor *)this)->gcRecurse();
  } else if (isAsk () == OK) {
    ((AskActor *)this)->gcRecurse();
  } else {
    ((SolveActor *)this)->gcRecurse();
  }
}

void WaitActor::gcRecurse()
{
  GCMETHMSG("WaitActor::gcRecurse");
  Assert(!isFreedRefsArray(next.getY()));
  board = board->gcBoard();
  Assert(board);

  next.gcRecurse ();

  int32 num = ToInt32(childs[-1]);
  Board **newChilds=(Board **) heapMalloc((num+1)*sizeof(Board *));

  PROFILE_CODE1(if (opMode == IN_TC) {
                   FDProfiles.inc_item(cp_size_waitactor, (num+1)*sizeof(Board *));
                 })

  *newChilds++ = (Board *) num;
  for (int i=0; i < num; i++) {
    if (childs[i]) {
      newChilds[i] = childs[i]->gcBoard();
      Assert(newChilds[i]);
    } else {
      newChilds[i] = (Board *) NULL;
    }
  }
  childs=newChilds;
}

void AskActor::gcRecurse ()
{
  GCMETHMSG("AskActor::gcRecurse");
  DebugCheck (isFreedRefsArray(next.getY ()),
              error ("freed 'y' regs in AskActor::gcRecurse ()"));
  next.gcRecurse ();
  board = board->gcBoard ();
  Assert(board);
}

void SolveActor::gcRecurse ()
{
  GCMETHMSG("SolveActor::gcRecurse");
  if (opMode == IN_GC || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
    Assert(board);
  }
  solveBoard = solveBoard->gcBoard();
  Assert(solveBoard);

  boardToInstall = boardToInstall->gcBoard ();
  gcTagged (solveVar, solveVar);
  gcTagged (guidance, guidance);
  gcTagged (result, result);
  suspList = suspList->gc(NO);
  stable_sl = stable_sl->gc(NO);
  orActors.gc (SolveActor::StackEntryGC);   // higher order :))
}


DLLStackEntry SolveActor::StackEntryGC (DLLStackEntry entry)
{
  Actor *aa=(Actor *) entry;
  if (aa->isCommitted()) return 0;
  Board *bb=aa->getBoardFast();
  if (!bb->gcIsAlive()) {
    // mm2 warning("SolveActor::StackEntryGC: dead node\n");
    return 0;
  }
  if (opMode == IN_TC && !isInTree(bb)) return 0;
  return ((DLLStackEntry) ((Actor *) entry)->gcActor());
}

void DLLStack::gc (DLLStackEntry (*f)(DLLStackEntry))
{
  DLLStackBodyEntry *rd = l, *current = c;
  clear ();
  while (rd != (DLLStackBodyEntry *) NULL) {
    DLLStackEntry el = (*f)(rd->elem);
    if (el == (DLLStackEntry) NULL) {
      if (current == rd)
        current = current->next;
    } else {
      push (el);
      if (current == rd)
        c = s;
    }
    rd = rd->next;
  }
}

//*****************************************************************************
//                           collectGarbage
//*****************************************************************************

#define ERROR(Fun, Msg)                                                       \
        error("%s in %s at %s:%d", Msg, Fun, __FILE__, __LINE__);

/* collect STuple, LTuple */

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
void STuple::gcRecurse()
{
  GCMETHMSG("STuple::gcRecurse");
  gcTagged(label,label);
  gcTaggedBlockRecurse(getRef(),getSize());
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
    case PTR_STUPLE:    ((STuple *) ptr)->gcRecurse();           break;
    case PTR_SRECORD:   ((SRecord *) ptr)->gcRecurse();          break;
    case PTR_DYNTAB:    ((DynamicTable *) ptr)->gcRecurse();     break;
    case PTR_NAME:      ((Literal *) ptr)->gcRecurse ();         break;
    case PTR_CONT:      ((Continuation*) ptr)->gcRecurse();      break;
    case PTR_SUSPCONT:  ((SuspContinuation*) ptr)->gcRecurse();  break;
    case PTR_CFUNCONT:  ((CFuncContinuation*) ptr)->gcRecurse(); break;
    case PTR_ACTOR:     ((Actor *) ptr)->gcRecurse();            break;
    case PTR_THREAD:    ((Thread *) ptr)->gcThreadRecurse();     break;
    case PTR_BOARD:     ((Board *) ptr)->gcRecurse();            break;
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
  if ((int) getUsedMemory() > ozconf.heapThreshold && ozconf.gcFlag) {
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

  /* calc upper limits for next gc */
  int used = getUsedMemory();
  if (used > (ozconf.heapThreshold*ozconf.heapMargin)/100) {
    ozconf.heapThreshold = ozconf.heapThreshold*(100+ozconf.heapIncrement)/100;
  }

  unsetSFlag(StartGC);
  osUnblockSignals();
}


Bool AM::idleGC()
{
  Assert(isToplevel());

  if ((int)getUsedMemory() > (ozconf.heapIdleMargin*ozconf.heapThreshold)/100 && ozconf.gcFlag) {
    if (ozconf.showIdleMessage) {
      printf("gc ... ");
      fflush(stdout);
    }
    int save = ozconf.gcVerbosity;
    ozconf.gcVerbosity = 0;
    doGC();
    ozconf.gcVerbosity = save;
    return OK;
  }
  return NO;
}

OzDebug *OzDebug::gcOzDebug()
{
  pred = (Chunk *) pred->gcConstTerm();
  Assert(pred);
  args = gcRefsArray(args);
  return this;
}
