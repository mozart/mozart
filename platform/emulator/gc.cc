/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

/****************************************************************************
 ****************************************************************************/

#undef TURNED_OFF
// #define TURNED_OFF

#ifdef LINUX_I486
/* FD_ISSET missing */
#include <sys/time.h>
#endif

#include <ctype.h>

#include "../include/config.h"

#include "gc.hh"
#include "builtins.hh"
#include "actor.hh"
#include "am.hh"
#include "board.hh"
#include "cell.hh"
#include "debug.hh"
#include "dllist.hh"
#include "genvar.hh"
#include "fdhook.hh"
#include "io.hh"
#include "misc.hh"
#include "objects.hh"
#include "stack.hh"
#include "thread.hh"
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
Bool isInTree(Board *b);


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
#  define VERBOSE
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

#ifdef CHECKSPACE

MemChunks *from;

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
void fastmemcpy(int *to, int *frm, int sz)
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
  if (opMode == IN_GC) memset(frm,sz,0xff);
#endif
}

inline
void *gcRealloc(void *ptr, size_t sz)
{
  void *ret = heapMalloc(sz);
  DebugCheck(sz%sizeof(int) != 0,
             error("gcRealloc: can only handle word sized blocks"););
  fastmemcpy((int*)ret,(int*)ptr,sz);
  return ret;
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
  PTR_SUSPCONT
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

  TypedPtr pop()  { return (TypedPtr) Stack::pop(); }
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
  void pushPtr(int* ptr, int value)
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
inline int  GCMARK(int S)      { return (S | GCTAG); }
inline int  GCMARK(void *S)    { return GCMARK((int) S); }

inline int  GCUNMARK(int S)    { return S & ~GCTAG; }
inline Bool GCISMARKED(int S)  { return S &  GCTAG ? OK : NO; }


/*
 * Check if object in from-space (elem) is already collected.
 *   Then return the forward pointer to to-space.
 */
#define CHECKCOLLECTED(elem,type)  \
  if (GCISMARKED((int)elem)) return (type) GCUNMARK((int) elem);


/*
 * Write a marked forward pointer (pointing into the to-space)
 * into a structure in the from-space.
 *
 *  If mode is IN_GC, store value in cell ptr only;
 *  else save cell at ptr and also store in this cell.
 *
 */
inline
void storeForward (int* fromPtr, int newValue)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(fromPtr, (int) *fromPtr);
  }
  DebugGC(opMode == IN_GC
          && MemChunks::list->inChunkChain((void *)fromPtr),
          error ("storing marked value in 'TO' space"));
  DebugGC(opMode == IN_GC
          && from->inChunkChain ((void *) newValue),
          error ("storing (marked) ref in to FROM-space"));
  *fromPtr = GCMARK(newValue);
}

inline
void storeForward (int* fromPtr, void *newValue) {
  storeForward (fromPtr, (int) newValue);
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
    am.stat.protectedCounter = 0;
#endif
    while(help) {
      gcTagged(*(TaggedRef*)help->elem, *(TaggedRef*)help->elem);
      help = (ExtRefNode*) help->next;
#ifdef PROFILE
      am.stat.protectedCounter++;
#endif
    }
  }
};


inline
Bool needsNoCollection(TaggedRef t)
{
  Assert(t!=makeTaggedNULL());

  TypeOfTerm tag = tagTypeOf(t);
  return (tag == SMALLINT ||
          (tag == LITERAL && (tagged2Literal(t)->isDynName ()) == NO))
         ? OK
         : NO;
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
  if (needsNoCollection(*ref))
    return OK;

  ExtRefNode *aux = (ExtRefNode*) extRefs->find(ref);

  if (aux == NULL)
    return NO;

  extRefs = (ExtRefNode*) extRefs->remove(aux);
  return OK;
}


/****************************************************************************
 * The update stack contains REF's to not yet copied variables
 ****************************************************************************/


DebugGCT(static int updateStackCount = 0);

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
 * TC: Path marks are used to decide if a board is local
 ****************************************************************************/

inline
Bool isLocalBoard (Board* b)
{
  return b->isPathMark() ? NO : OK;
}

/*
 * TC: before copying:  all nodes are marked, but node self
 */
inline
void setPathMarks (Board *bb)
{
  bb = bb->getParentBoard ();
  while (OK) {
    bb->setPathMark();
    if (bb->isRoot () == OK) {
      return;
    } else if (bb->isCommitted () == OK) {
      bb = bb->getBoard ();
    } else {
      bb = bb->getParentBoard ();
    }
  }
  error ("(gc) setPathMarks");
}

/*
 * TC: after copying
 */

inline
void unsetPathMarks (Board *bb)
{
  bb = bb->getParentBoard ();
  while (OK) {
    bb->unsetPathMark();
    if (bb->isRoot () == OK) {
      return;
    } else if (bb->isCommitted () == OK) {
      bb = bb->getBoard ();
    } else {
      bb = bb->getParentBoard ();
    }
  }
  error ("(gc) unsetPathMarks");
}


/****************************************************************************
 * Collect all types of terms
 ****************************************************************************/

/*
 * Literals:
 *   forward in 'printName'
 * NOTE:
 *   3 case: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */
inline
Literal *Literal::gc()
{
  if (isDynName() == NO) {
    return (this);
  }

  if (opMode == IN_GC || isLocalBoard (home) == OK) {
    GCMETHMSG("Literal::gc");
    CHECKCOLLECTED (printName, Literal *);
    varCount++;
    Name *aux = (Name *) gcRealloc (this,sizeof (*this));
    GCNEWADDRMSG (aux);
    ptrStack.push (aux, PTR_NAME);
    storeForward((int*) &printName, aux);
    return (aux);
  } else {
    return (this);
  }
}

inline
void Literal::gcRecurse ()
{
  GCMETHMSG("Literal::gcRecurse");
  DebugGC((isDynName () == NO),
          error ("non-dynamic name is found in gcRecurse"));
  home = home->gcBoard ();
  if (home == (Board *) NULL) {
    /*
     * mm2
     *  kludge: 'home' mayn't be (Board *) NULL;
     *  therefore lets it be the rootBoard;
     */
    home = am.rootBoard;
  }
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
  CHECKCOLLECTED(*(int *)&value.alloc, BigInt *);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  storeForward((int *)&value.alloc, ret);
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
            auxTerm = GCUNMARK(auxTerm);
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


/* mm2: have to check for discarded node */
inline
SuspContinuation *SuspContinuation::gcCont()
{
  GCMETHMSG("SuspContinuation::gcCont");
  if (this == NULL) return NULL;

  if (pc != NOCODE) {
    CHECKCOLLECTED(pc, SuspContinuation *)
  }

  SuspContinuation *ret = (SuspContinuation*) gcRealloc(this, sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_SUSPCONT);

  DebugGC(opMode == IN_TC && (!isLocalBoard(board->gcGetBoardDeref ()) ||
                              !isInTree(board->gcGetBoardDeref ())),
          error ("non-local board in TC mode is being copied"));

  storeForward((int *)&pc, ret);
  return ret;
}

// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc bit set --> has already been copied

RefsArray gcRefsArray(RefsArray r)
{
  GCPROCMSG("gcRefsArray");
  GCOLDADDRMSG(r);
  if (r == NULL)
    return r;

  NOTINTOSPACE(r);

  CHECKCOLLECTED(r[-1], RefsArray);

  int sz = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(sz,NO);
  GCNEWADDRMSG(aux);

  if (isDirtyRefsArray(r)) {
    markDirtyRefsArray(aux);
  }

  DebugCheck(isFreedRefsArray(r),
             markFreedRefsArray(aux););

  storeForward((int*) &r[-1], aux);

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
}

/* mm2: have to check for discarded node */
CFuncContinuation *CFuncContinuation::gcCont(void)
{
  GCMETHMSG("CFuncContinuation::gcCont");
  CHECKCOLLECTED(cFunc, CFuncContinuation *);

  CFuncContinuation *ret = (CFuncContinuation*) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CFUNCONT);
  storeForward((int *)&cFunc, ret);
  return ret;
}


Continuation *Continuation::gc()
{
  GCMETHMSG("Continuation::gc");
  CHECKCOLLECTED(pc, Continuation *);

  Continuation *ret = (Continuation *) gcRealloc(this,sizeof(Continuation));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CONT);
  storeForward((int *)&pc, ret);
  return ret;
}

inline
void Continuation::gcRecurse(){
  GCMETHMSG("Continuation::gcRecurse");
  DebugCheck (isFreedRefsArray (yRegs),
              error ("freed 'y' refs array in Continuation::gcRecurse ()"));
  yRegs = gcRefsArray(yRegs);
  DebugCheck (isFreedRefsArray (gRegs),
              error ("freed 'g' refs array in Continuation::gcRecurse ()"));
  gRegs = gcRefsArray(gRegs);
  DebugCheck (isFreedRefsArray (xRegs),
              error ("freed 'x' refs array in Continuation::gcRecurse ()"));
  xRegs = gcRefsArray(xRegs);
}

inline
void SuspContinuation::gcRecurse(){
  GCMETHMSG("SuspContinuation::gcRecurse");
  board = board->gcBoard();
  Continuation::gcRecurse();
}


/* collect STuple, LTuple */

inline
void gcTaggedBlock(TaggedRef *oldBlock, TaggedRef *newBlock,int sz)
{
  for(int i = sz-1; i>=0; i--) {
    if (!isRef(oldBlock[i]) && isAnyVar(oldBlock[i])) {
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
  storeForward((int *)&label, ret);
  gcTaggedBlock(getRef(),ret->getRef(),getSize());
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

  gcTaggedBlock(args,ret->args,2);

  storeForward((int*) &args[0], ret);
  return ret;
}


inline
SRecord *SRecord::gcSRecord()
{
  GCMETHMSG("SRecord::gcSRecord");
  if (this == NULL) return NULL; /* objects may contain an empty record */

  CHECKCOLLECTED(u.type, SRecord *);

  size_t sz;

  switch(getType()) {

  case R_ABSTRACTION:
    if (opMode == IN_TC && isLocalBoard (((Abstraction *) this)->getBoard ()) == NO)
      return this;
    sz = sizeof(Abstraction);
    DebugGCT(if (opMode == IN_GC)
             NOTINTOSPACE(((Abstraction *) this)->name););
    break;
  case R_OBJECT:
    sz = sizeof(Object);
    break;
  case R_CELL:
    if (opMode == IN_TC && isLocalBoard (((Cell *) this)->getBoard ()) == NO)
      return this;
    sz = sizeof(Cell);
    break;
  case R_CHUNK:
  case R_RECORD:
    sz = sizeof(SRecord);
    break;
  case R_BUILTIN:
    switch (((Builtin *) this)->getType()) {
    case BIsolveCont:
      sz = sizeof(OneCallBuiltin);
      //: kost@ 21.12.94: not necessary any more;
      //: if (((OneCallBuiltin *) this)->isSeen())
      //:       ((Builtin *) this)->gRegs = NULL;
      break;
    case BIsolved:
      sz = sizeof(SolvedBuiltin);
      break;
    default:
      sz = sizeof(Builtin);
    }
    break;
  default:
    sz = 0;
    error("SRecord::gc: unknown type");
  }

  SRecord *ret = (SRecord*) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_SRECORD);
  storeForward((int *)&u.type, ret);
  return ret;
}

// kost@: we have to split suspension lists of variables which are quantified
//        in "solve" board itself in two parts - "relevant" wrt copy and
//        'irrelevant';
inline
Bool isInTree (Board *b)
{
  DebugGC((opMode == IN_GC), error ("'isInTree(Board *)' is called in GC"));
  Board *rb = am.rootBoard;
  while (b != (Board *)NULL) {
    DebugCheck((b->isCommitted () == OK),
               error ("committed board in 'isInTree (Board *)'"));
    if (b == fromCopyBoard)
      return (OK);
    if (isLocalBoard (b) == NO)
      return (NO);
    b = b->getParentBoard();
    if (b != (Board *) NULL)
      b = b->gcGetBoardDeref();
  }
  return (NO);
}

inline
Bool isInTree (Actor *a)
{
  DebugGC((opMode == IN_GC), error ("'isInTree(Actor *)' is called in GC"));
  DebugCheck((a->isCommitted () == OK),
             error ("committed actor in isInTree(Actor *)"));
  Board *b = a->getBoard ()->gcGetBoardDeref ();
  Board *rb = am.rootBoard;
  DebugCheck ((b == rb), error ("isInTree(Actor *): under root Board"));
  while (b != (Board *) NULL && b != rb) {
    if (b == fromCopyBoard)
      return (OK);
    if (isLocalBoard (b) == NO)
      return (NO);
    b = b->getParentBoard ();
    if (b != (Board *) NULL)
      b = b->gcGetBoardDeref ();
  }
  return (NO);
}

/* return NULL if contains pointer to discarded node */
inline
Suspension *Suspension::gcSuspension(Bool tcFlag)
{
  GCMETHMSG("Suspension::gcSuspension");
  if (this == NULL)
    return ((Suspension *) NULL);

  CHECKCOLLECTED(flag, Suspension*);

  if (isDead ()) {
    return NULL;
  }

  Board *el = getBoard()->gcGetBoardDeref();

  if (el == NULL) {
    return NULL;
  }

  // mm2: el may be a GCMARK'ed board
  if (tcFlag == OK && isInTree(el) == NO) {
    return ((Suspension *) NULL);
  }

  Suspension *newSusp = (Suspension *) gcRealloc(this, sizeof(*this));
  GCNEWADDRMSG(newSusp);

  switch (flag & (S_cont|S_cfun)){
  case S_null:
    newSusp->item.board = item.board->gcBoard();
    break;
  case S_cont:
    newSusp->item.cont = item.cont->gcCont();
    break;
  case S_cont|S_cfun:
    newSusp->item.ccont = item.ccont->gcCont();
    break;
  default:
    error("Unexpected case in Suspension::gc().");
  }

  DebugCheck(!getBoard()->gcGetBoardDeref(),
             warning("gc: adding dead node (3)"));

  storeForward((int *)&flag, newSusp);
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
    if (help->isCondSusp()){
      Condition *auxConds = ((CondSuspList*)help)->getConds();
      unsigned int auxNumOfConds = ((CondSuspList*)help)->getCondNum();
      auxConds = (Condition*) gcRealloc(auxConds,
                                        auxNumOfConds * sizeof(Condition));
      for (int i = 0; i < auxNumOfConds; i++)
        gcTagged(((CondSuspList*)help)->getConds()[i].arg, auxConds[i].arg);

      ret = new CondSuspList(aux, ret, auxConds, auxNumOfConds);
    } else {
      ret = new SuspList(aux, ret);
    }
  }

  GCNEWADDRMSG (ret);
  return (ret);
}

inline
void GenCVariable::gc(void)
{
  Assert(getType()==FDVariable || getType()==OFSVariable);
  switch (getType()){
  case FDVariable:
    ((GenFDVariable*)this)->gc();
    break;
  case OFSVariable:
    ((GenOFSVariable*)this)->gc();
    break;
  default:
    break;
  }
}


// This procedure collects the entry points into heap provided by variables,
// without copying the tagged reference of the variable itself.
TaggedRef gcVariable(TaggedRef var)
{
  GCPROCMSG("gcVariable");
  GCOLDADDRMSG(var);
  TypeOfTerm varTag = tagTypeOf(var);

  switch(varTag){

  case UVAR:
    {
      Board *hom = tagged2VarHome(var);
      INFROMSPACE(hom);
      hom = hom->gcBoard();
      INTOSPACE (hom);
      GCNEWADDRMSG((hom ? makeTaggedUVar(hom) : makeTaggedUVar(am.rootBoard)));
      // kludge: if its board is not alive, lets its board to be the root...
      return hom ? makeTaggedUVar(hom) : makeTaggedUVar(am.rootBoard);
    }

  case SVAR:
    {
      SVariable *cv = tagged2SVar(var);
      INFROMSPACE(cv);
      if (GCISMARKED(*(int*)cv)) {
        GCNEWADDRMSG(makeTaggedSVar((SVariable*)GCUNMARK(*(int*)cv)));
        return makeTaggedSVar((SVariable*)GCUNMARK(*(int*)cv));
      }

      Board *newBoard = cv->home->gcBoard();
      if (!newBoard) {
        GCNEWADDRMSG(makeTaggedUVar(am.rootBoard));
        return makeTaggedUVar(am.rootBoard);
      }

      int cv_size;
      cv_size = sizeof(SVariable);

      SVariable *new_cv = (SVariable*)gcRealloc(cv,cv_size);

      storeForward((int *)cv, new_cv);

      if (opMode == IN_TC && new_cv->getHome () == fromCopyBoard)
        new_cv->suspList = new_cv->suspList->gc(OK);
      else
        new_cv->suspList = new_cv->suspList->gc(NO);

      DebugGC((opMode == IN_GC && new_cv->home == newBoard),
              error ("home node of variable is not copied"));

      new_cv->home = newBoard;
      GCNEWADDRMSG(makeTaggedSVar(new_cv));
      return makeTaggedSVar(new_cv);
    }
  case CVAR:
    {
      GenCVariable *gv = tagged2CVar(var);

      INFROMSPACE(gv);
      if (GCISMARKED(*(int*)gv)) {
        GCNEWADDRMSG(makeTaggedCVar((GenCVariable*)GCUNMARK(*(int*)gv)));
        return makeTaggedCVar((GenCVariable*)GCUNMARK(*(int*)gv));
      }

      Board *newBoard = gv->home->gcBoard();
      if (!newBoard) {
        GCNEWADDRMSG(makeTaggedUVar(am.rootBoard));
        return makeTaggedUVar(am.rootBoard);
      }

      int gv_size = gv->getSize();

      GenCVariable *new_gv = (GenCVariable*)gcRealloc(gv, gv_size);

      storeForward((int *)gv, new_gv);

      if (opMode == IN_TC && new_gv->getHome () == fromCopyBoard)
        new_gv->suspList = new_gv->suspList->gc(OK);
      else
        new_gv->suspList = new_gv->suspList->gc(NO);
      new_gv->gc();

      DebugGC((opMode == IN_GC && new_gv->home == newBoard),
              error ("home node of variable is not copied"));

      new_gv->home = newBoard;
      GCNEWADDRMSG(makeTaggedCVar(new_gv));
      return makeTaggedCVar(new_gv);
    }
  default:
    error("gcVariable: only variables allowed here.");
    return makeTaggedNULL();
  } // switch
}


void GenFDVariable::gc(void)
{
  GCMETHMSG("GenFDVariable::gc");
  finiteDomain.gc();

  int i;
  if (opMode == IN_TC && getHome() == fromCopyBoard)
    for (i = 0; i < fd_any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(OK);
  else
    for (i = 0; i < fd_any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(NO);
}

void DynamicTable::gc(void)
{
    Assert(isPwrTwo(size));
    // Copy the actual table:
    HashElement* tableCopy=(HashElement*) gcRealloc(table,size*sizeof(HashElement));
    // Take care of all TaggedRefs in the table:
    for (dt_index i=0; i<size; i++) {
        if (table[i].ident!=makeTaggedNULL()) {
            gcTagged(table[i].ident, tableCopy[i].ident);
            gcTagged(table[i].value, tableCopy[i].value);
        }
    }
    // Update the pointer in the copied block:
    table=tableCopy;
}


void GenOFSVariable::gc(void)
{
    dynamictable.gc();
}


inline
Bool updateVar(TaggedRef var)
{
  GCPROCMSG("updateVar");
  Board * varhome;
  if (opMode == IN_TC) {
    if (isUVar(var)) {
      varhome = tagged2VarHome(var);
    } else if (isSVar(var)) {
      varhome = tagged2SVar(var)->getHome1();
    } else {
      varhome = tagged2CVar(var)->getHome1();
    }

    varhome = varhome->gcGetBoardDeref();
    return (varhome != NULL && isLocalBoard(varhome));
  }

  return OK;
}


// If an object has been copied, all slots of type
// 'TaggedRef' have to be
// treated particularly:
// - references pointing to non-variables have to be derefenced
// - references to variables have to be treated after the whole term structure
//   has been collected and therefore the address of such a reference has to
//   put on the update stack
// - variables, which are part of the block being copied, have to be marked as
//   copied
// - if non-collected variables are dereferenced, the entry points into heap
//   provided by them, have to be collected by 'gcVariable'


void gcTagged(TaggedRef &fromTerm, TaggedRef &toTerm)
{
  GCPROCMSG("gcTagged");
  TaggedRef auxTerm = fromTerm;

  TaggedRef *auxTermPtr = NULL;

#ifdef DEBUG_GC
  auxTermPtr = NULL;
  if (opMode == IN_GC && from->inChunkChain(&toTerm))
    error ("having toTerm in FROM space");
#endif

  if (IsRef(auxTerm)) {

    do {
      if (auxTerm == makeTaggedNULL()) {
        toTerm = auxTerm;
        return;
      }

      if (GCISMARKED(auxTerm)) {
        toTerm = makeTaggedRef((TaggedRef*)GCUNMARK(auxTerm));
        return;
      }
      auxTermPtr = tagged2Ref(auxTerm);
      auxTerm = *auxTermPtr;
    } while (IsRef(auxTerm));
  }

  TypeOfTerm auxTermTag = tagTypeOf(auxTerm);

  DebugGCT(NOTINTOSPACE(auxTermPtr));

  switch (auxTermTag) {

  case SMALLINT:
    toTerm = auxTerm;
    return;

  case LITERAL:
    toTerm = makeTaggedLiteral (tagged2Literal (auxTerm)->gc ());
    return;

  case LTUPLE:
    toTerm = makeTaggedLTuple(tagged2LTuple(auxTerm)->gc());
    return;

  case STUPLE:
    toTerm = makeTaggedSTuple(tagged2STuple(auxTerm)->gc());
    return;

  case SRECORD:
    toTerm = makeTaggedSRecord(tagged2SRecord(auxTerm)->gcSRecord());
    return;

  case CONST:
    toTerm = makeTaggedConst(tagged2Const(auxTerm)->gcConstTerm());
    return;

  case BIGINT:
    toTerm = makeTaggedBigInt(tagged2BigInt(auxTerm)->gc());
    return;

  case FLOAT:
    toTerm = makeTaggedFloat(tagged2Float(auxTerm)->gc());
    return;

  case SVAR:
  case UVAR:
  case CVAR:
    varCount++;
    if (auxTerm == fromTerm) {   // (fd-)variable is component of this block

      DebugGCT(toTerm = fromTerm); // otherwise 'makeTaggedRef' complains
      if (updateVar(auxTerm)) {
        storeForward((int*) &fromTerm, makeTaggedRef(&toTerm));

        // updating toTerm AFTER fromTerm:
        toTerm = gcVariable(auxTerm);
      } else {
        toTerm = fromTerm;
      }
      return;
    }

#ifdef RS
    if (updateVar(auxTerm) && auxTermTag == CVAR) {
      TaggedRef *ref = (TaggedRef *) heapMalloc(sizeof(TaggedRef));
      *ref = gcVariable(auxTerm);
      storeForward(auxTermPtr, ref);
      return;
    }
#endif

    // put address of ref cell to be updated onto update stack
    if (updateVar(auxTerm)) {
      updateStack.push(&toTerm);
      gcVariable(auxTerm);
      toTerm = (TaggedRef) auxTermPtr;
      DebugGC((auxTermPtr == NULL), error ("auxTermPtr == NULL"));
    } else {
      toTerm = fromTerm;
    }
    return;

  default:
    error("Unknown tag in term: %0x",auxTermTag);
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

  stat.initGcMsg(msgLevel);

  MemChunks *oldChain = MemChunks::list;

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
  setCurrent(currentBoard->gcBoard(),NO);

  GCPROCMSG("Predicate table");
  CodeArea::gc();

  SRecord::aritytable.gc ();

  GCREF(currentThread);
  GCREF(rootThread);

  if (currentThread && currentThread->isNormal()) {
    am.currentTaskStack = currentThread->getTaskStack();
  } else {
    am.currentTaskStack = (TaskStack *) NULL;
  }

  if (FDcurrentTaskSusp != (Suspension *) NULL) {
    warning("FDcurrentTaskSusp must be NULL!");
    FDcurrentTaskSusp = FDcurrentTaskSusp->gcSuspension (NO);
  }
#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc(NO);
#endif

  Thread::GC();

  GCPROCMSG("ioNodes");
  for(int i = 0; i < IO::openMax; i++)
    if(FD_ISSET(i,&IO::globalReadFDs)) {
      if (i != IO::QueryFILE->csfileno()) {
        IO::ioNodes[i] = IO::ioNodes[i]->gcBoard();
      }
    }
  performCopying();

  GCPROCMSG("toplevelVars");
  am.toplevelVars = gcRefsArray(am.toplevelVars);

  GCPROCMSG("updating external references to terms into heap");
  ExtRefNode::gc();

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

  stat.printGcMsg();

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
      TaggedRef *Term = updateStack.pop();
      TaggedRef auxTerm     = *Term;
      TaggedRef *auxTermPtr = NULL;

      while(IsRef(auxTerm)) {
        if (auxTerm == (TaggedRef) NULL)
          // kost@: Reachable variables can be quantified in dead boards too;
          goto loop;
        if (GCISMARKED(auxTerm)) {
          *Term = makeTaggedRef((TaggedRef*)GCUNMARK(auxTerm));
          goto loop;
        }
        auxTermPtr = tagged2Ref(auxTerm);
        auxTerm = *auxTermPtr;
      }

      TaggedRef newVar = gcVariable(auxTerm);

//      Assert(tagTypeOf(newVar) == tagTypeOf(auxTerm));

      if (newVar == makeTaggedNULL()) {
        /*
         * mm2: I don't know if this is correct
         *   the handling of dead nodes should be reconsidered carefully
         */
        *Term = newVar;
        *auxTermPtr = newVar;
      } else {
        switch(tagTypeOf(newVar)){
        case UVAR:
          *Term = makeTaggedRef(newTaggedUVar(tagged2VarHome(newVar)));
          break;
        case SVAR:
          *Term = makeTaggedRef(newTaggedSVar(tagged2SVar(newVar)));
          break;
        case CVAR:
#ifndef RS
          *Term = makeTaggedRef(newTaggedCVar(tagged2CVar(newVar)));
          break;
#endif
        default:
          error("processUpdateStack: variable expected here.");
        } // switch
        INFROMSPACE(auxTermPtr);
        storeForward((int*) auxTermPtr,
                     *Term);
      }
    } // while

  DebugGC(updateStackCount != 0,
          error ("updateStackCount != 0"));
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
  if (isGround == (Bool *) NULL) {
    GCMETHMSG(" ********** AM::copyTree **********");
  } else {
    GCMETHMSG(" ********** AM::copyTree (groundnes) **********");
  }
  opMode = IN_TC;
  gcing = 0;
  varCount = 0;
  stat.timeForCopy -= usertime();

  DebugGC ((bb->isCommitted () == OK), error ("committed board to be copied"));
  fromCopyBoard = bb;
  setPathMarks(fromCopyBoard);
  toCopyBoard = fromCopyBoard->gcBoard();
  // kost@ : FDcurrentTaskSusp ???

  performCopying();

  processUpdateStack();

  if (!ptrStack.isEmpty())
    error("ptrStack should be empty");

  while (!savedPtrStack.isEmpty()) {
    int value = (int)  savedPtrStack.pop();
    int* ptr  = (int*) savedPtrStack.pop();
    *ptr = value;
  }

  unsetPathMarks(fromCopyBoard);
  fromCopyBoard = NULL;
  gcing = 1;

  stat.timeForCopy += usertime();
  // Note that parent, right&leftSibling must be set in this subtree -
  // for instance, with "setParent"

  if (isGround != (Bool *) NULL) {
    if (varCount == 0)
      *isGround = OK;
    else
      *isGround = NO;
  }

  return toCopyBoard;
}

//*****************************************************************************
//                                GC-METHODS
//*****************************************************************************

inline void AbstractionEntry::gc()
{
  abstr = (Abstraction *) abstr->gcSRecord();
  g     = gcRefsArray(g);
}

void AbstractionTable::gc()
{
  // there may be NULL entries in the table during gc

  GCMETHMSG("AbstractionTable::gc");

  HashNode * hn = getFirst();
  for (; hn != NULL; hn = getNext(hn)) {
    ((AbstractionEntry*) hn->value)->gc();
  }
}

void ArityTable::gc ()
{
  GCMETHMSG("ArityTable::gc");

  for (int i = 0; i < size; i++) {
    if (table[i] == (Arity *) NULL)
      continue;
    (table[i])->gc ();
  }
}

void Arity::gc ()
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

void CodeArea::gc()
{
  abstractionTab.gc();
}


TaskStack *TaskStack::gc()
{
  GCMETHMSG("TaskStack::gc");
  TaskStack *newStack = new TaskStack(size);

  newStack->gcInit();

  while (!isEmpty()) {

    TaggedBoard tb = (TaggedBoard) pop();
    ContFlag cFlag = getContFlag(tb);
    Board *newBB = getBoard(tb, cFlag)->gcBoard();

    if (newBB == NULL) {
      switch (cFlag){
      case C_NERVOUS:    continue;
      case C_XCONT:      pop(4); continue;
      case C_CONT:       pop(3); continue;
      case C_DEBUG_CONT: pop(1); continue;
      case C_CFUNC_CONT: pop(3); continue;
      case C_CALL_CONT:  pop(2); continue;
      }
    }

    newStack->gcQueue((TaskStackEntry) setContFlag(newBB,cFlag));

    switch (cFlag){

    case C_NERVOUS:
      break;

    case C_CONT:
      newStack->gcQueue(pop());                           // PC
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // Y
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // G
      break;

    case C_XCONT:
      newStack->gcQueue(pop());                           // PC
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // Y
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // G
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // X
      break;

    case C_DEBUG_CONT:
      newStack->gcQueue(((OzDebug *) pop())->gcOzDebug());
      break;

    case C_CALL_CONT:
      newStack->gcQueue(((SRecord *) pop())->gcSRecord());
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));
      break;

    case C_CFUNC_CONT:
      newStack->gcQueue(pop());                // OZ_CFun
      newStack->gcQueue(((Suspension*) pop())->gcSuspension());
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));
      break;

    default:
      error("Unexpected case in TaskStack::gc().");
      break;
    }
  } // while not task stack is empty

  newStack->gcEnd();

  return newStack;
} // TaskStack::gc



//*****************************************************************************
//                           NODEs
//*****************************************************************************

ConstTerm *ConstTerm::gcConstTerm()
{
  GCMETHMSG("ConstTerm::gcConstTerm");
  CHECKCOLLECTED(type, ConstTerm *);

  switch (typeOf()) {
  case Co_Board:
    //: kost@ 22.12.94: work-around for current! register allocation;
    //: return ((Board *) this)->gcBoard();
    {
      Board *newBoard = ((Board *) this)->gcBoard();
      if (newBoard == (Board *) NULL) {
        if (opMode == IN_TC) return (toCopyBoard);
        else return (am.rootBoard);
      } else {
        return (newBoard);
      }
    }
  case Co_Actor:
    return ((Actor *) this)->gc();
  case Co_Thread:
    return ((Thread *) this)->gc();

  default:
    error("ConstTerm::gcConstTerm: unknown tag 0x%x", typeOf());
    return NULL;
  }
}

void Thread::GC()
{
  GCREF(Head);
  GCREF(Tail);
}

Thread *Thread::gc()
{
  GCMETHMSG("Thread::gc");
  CHECKCOLLECTED(flags, Thread *);
  size_t sz = sizeof(Thread);
  Thread *ret = (Thread *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_THREAD);
  storeForward((int *)&flags, ret);
  return ret;
}

void Thread::gcRecurse()
{
  GCMETHMSG("Thread::gcRecurse");

  GCREF(next);
  GCREF(prev);

  DebugGC ((opMode == IN_TC), error ("thread is gc'ed in 'copy' mode?"));
  if (resSusp != NULL)
   resSusp = resSusp->gcSuspension(NO);
  if (isNormal()) {
    GCREF(u.taskStack);
  } else if (isSuspCont()) {
    u.suspCont=u.suspCont->gcCont();
    // suspension may be dead
    if (!u.suspCont) {
      error("mm2: never be here");
      flags=0; // == T_Normal
    }
  } else if (isSuspCCont()) {
    u.suspCCont=u.suspCCont->gcCont();
    // suspension may be dead
    if (!u.suspCCont) {
      error("mm2: never be here");
      flags=0; // == T_Normal
    }
  } else if (isNervous()) {
    u.board=u.board->gcBoard();
  } else {
    error("Thread::gcRecurse");
  }
  DebugGCT(Board *sob = notificationBoard);
  notificationBoard = (notificationBoard->gcGetNotificationBoard ())->gcBoard ();
  DebugGC((sob != (Board *) NULL && notificationBoard == (Board *) NULL),
          error ("notification Board is removed in Thread::gcRecurse"));
}

Board* Board::gcGetNotificationBoard ()
{
  GCMETHMSG("Board::gcGetNotificationBoard");
  Board *bb = this;
  if (bb == (Board *) NULL)
    return (bb);
  Board *nb = this;
  Actor *auxActor;
  while (OK) {
    if (bb->isRoot () == OK)
      return (nb);
    if (bb->isDiscarded () == OK || bb->isFailed () == OK) {
      auxActor = bb->u.actor;
      DebugGC((auxActor == (Actor *) NULL ||
               ((ConstTerm *) auxActor)->getType () != Co_Actor),
              error ("non-actor is got in Board::gcGetNotificationBoard"));
      bb = auxActor->getBoard ();
      nb = bb;   // probably not dead;
      continue;
    }
    if (bb->isCommitted () == OK) {
      bb = bb->u.board;
    } else {
      auxActor = bb->u.actor;
      DebugGC((auxActor == (Actor *) NULL ||
               ((ConstTerm *) auxActor)->getType () != Co_Actor),
              error ("non-actor is got in Board::gcGetNotificationBoard"));
      bb = auxActor->getBoard ();
    }
  }
}

#define  OUR_SPECS
// #undef   OUR_SPECS
#ifdef   OUR_SPECS
//  The idea:
//  In canonical version of the Board::gcGetBoardDeref a node can be determined
// as living, though among its parents there is a dead one. It means, that
// gcGetBoardDeref is sound but not complete. There is an attempt to eliminate
// this problem.
Board *Board::gcGetBoardDeref()
{
  GCMETHMSG("Board::gcGetBoardDeref");
  Board *bb = this;
  while (OK) {
    if (!bb || GCISMARKED(bb->suspCount)) {
      return bb;
    }
    if (bb->isDiscarded() || bb->isFailed()) {
      return NULL;
    } else if (bb->isCommitted()) {
      bb = bb->u.board;
    } else {
      Board *retB = bb;
      while (OK) {
        if (!bb || GCISMARKED(bb->suspCount)) {
          return retB;
        }
        if (bb->isDiscarded() || bb->isFailed()) {
          return NULL;
        } else if (bb->isCommitted()) {
          bb = bb->u.board;
        } else {
          if (bb->isRoot () == OK) {
            return retB;
          } else {
            bb = bb->getParentBoard ();
          }
        }
      }
    }
  }
  error("Board::gcGetBoardDeref");
}
#else
Board *Board::gcGetBoardDeref()
{
  GCMETHMSG("Board::gcGetBoardDeref");
  Board *bb = this;
  while (OK) {
    if (!bb || GCISMARKED(bb->suspCount)) {
      return bb;
    }
    if (bb->isDiscarded() || bb->isFailed()) {
      return NULL;
    } else if (bb->isCommitted()) {
      bb = bb->u.board;
    } else {
      return bb;
    }
  }
  error("Board::gcGetBoardDeref");
}
#endif

// This procedure derefences cluster chains and collects only the object at
// the end of such a chain.
Board *Board::gcBoard()
{
  GCMETHMSG("Board::gcBoard");

  Board *bb;
  bb=this->gcGetBoardDeref();

  return bb ? bb->gcBoard1() : bb;
}

Board *Board::gcBoard1()
{
  CHECKCOLLECTED(suspCount, Board *);
  DebugGC (opMode == IN_TC && !isLocalBoard (this),
            error ("non-local board is copied!"));
  // Kludge: because of allocation of 'y' registers a non-'isInTree' board
  //         can be reached.
  // Moreover: because of allocation of 'x' registers (for instance, a
  // 'SuspContinuation' may containt in these registers any ***irrelevant***
  // values) a non-'isInTree' board can be reached;
  if (opMode == IN_TC && isInTree (this) == NO) {
    storeForward ((int *) &suspCount, this);
    return (this);
  }
  size_t sz = sizeof(Board);
  Board *ret = (Board *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_BOARD);
  storeForward((int *)&suspCount, ret);
  return ret;
}

void Board::gcRecurse()
{
  GCMETHMSG("Board::gcRecurse");
  if (isCommitted()) {
    error("Board::gcRecurse:: never collect committed nodes");
//    body.defeat();
//    GCREF(u.board);
  } else {
    DebugCheck (isFreedRefsArray(body.getY ()),
                error ("freed 'y' regs in Board::gcRecurse ()"));
    body.gcRecurse();
    GCREF(u.actor);
  }
  script.Script::gc();
}

Actor *Actor::gc()
{
  GCMETHMSG("Actor::gc");
  CHECKCOLLECTED(priority, Actor *);
  // by kost@; flags are needed for getBoardDeref
  size_t sz;
  if (isWait()) {
    sz = sizeof(WaitActor);
  } else if (isAsk () == OK) {
    sz = sizeof(AskActor);
  } else {
    sz = sizeof (SolveActor);
  }
  Actor *ret = (Actor *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_ACTOR);
  storeForward((int *)&priority, ret);
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
  DebugCheck (isFreedRefsArray(next.getY ()),
              error ("freed 'y' regs in WaitActor::gcRecurse ()"));
  board = board->gcBoard ();
  next.gcRecurse ();

  int num = (int) childs[-1];
  Board **newChilds=(Board **) heapMalloc((num+1)*sizeof(Board *));
  *newChilds++ = (Board *) num;
  for (int i=0; i < num; i++) {
    if (childs[i]) {
      newChilds[i] = childs[i]->gcBoard();
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
}

void SolveActor::gcRecurse ()
{
  GCMETHMSG("SolveActor::gcRecurse");
  if (opMode == IN_GC || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
  }
  solveBoard = solveBoard->gcBoard ();
  boardToInstall = boardToInstall->gcBoard ();
  gcTagged (solveVar, solveVar);
  gcTagged (result, result);
  suspList = suspList->gc(NO);
  orActors.gc (SolveActor::StackEntryGC);   // higher order :))
}


DLLStackEntry SolveActor::StackEntryGC (DLLStackEntry entry)
{
  if (((Actor *) entry)->isCommitted () == OK) {
    return ((DLLStackEntry) NULL);
  } else if (opMode == IN_TC && isInTree((Actor *) entry) == NO) {
    return ((DLLStackEntry) NULL);
  } else {
    return ((DLLStackEntry) ((Actor *) entry)->gc ());
  }
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

void SRecord::gcRecurse()
{
  GCMETHMSG("SRecord::gcRecurse");
  switch (getType()) {

  case R_ABSTRACTION:
    {
      Abstraction *a = (Abstraction *) this;
      DebugCheck (isFreedRefsArray (a->gRegs),
                  error ("freed 'g' refs (abs) array in SRecord::gcRecurse ()"));
      a->gRegs = gcRefsArray(a->gRegs);
      a->name = a->name->gc ();
      DebugGC((a->name->getHome () == (Board *) ALLBITS ||
               a->name->getHome () == (Board *) NULL),
              error ("non-dynamic name is met in Abstraction::gcRecurse"));
      INTOSPACE(a->name);
      break;
    }

  case R_OBJECT:
    {
      Object *o      = (Object *) this;
      DebugCheck (isFreedRefsArray (o->gRegs),
                  error ("freed 'g' refs (obj) array in SRecord::gcRecurse ()"));
      o->gRegs       = gcRefsArray(o->gRegs);
      o->cell        = (Cell*)o->cell->gcSRecord();
      o->fastMethods = o->fastMethods->gcSRecord();
      // "Literal *printName" needs no collection
      break;
    }

  case R_CELL:
    {
      Cell *c = (Cell *) this;
      c->name = c->name->gc ();
      gcTagged(c->val,c->val);
      break;
    }

  case R_BUILTIN:
    {
      Builtin *bi = (Builtin *) this;
      gcTagged(bi->suspHandler,bi->suspHandler);
      DebugCheck (isFreedRefsArray (bi->gRegs),
                  error ("freed 'g' refs (bi) array in SRecord::gcRecurse ()"));
      bi->gRegs = gcRefsArray(bi->gRegs);
      break;
    }
  case R_CHUNK:
    break;

  case R_RECORD:
    gcTagged(u.label, u.label);
    break;

  default:
    error("SRecord::gcRecurse:: unknown type");
  }

  DebugCheck (isFreedRefsArray (args),
              error ("freed 'g' refs (args) array in SRecord::gcRecurse ()"));
  args = gcRefsArray(args);
}

/* collect STuple, LTuple */

inline
void gcTaggedBlockRecurse(TaggedRef *block,int sz)
{
  for(int i = sz-1; i>=0; i--) {
    if (isRef(block[i]) || !isAnyVar(block[i])) {
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
void LTuple::gcRecurse()
{
  GCMETHMSG("LTuple::gcRecurse");
  gcTaggedBlockRecurse(args,2);
}



void performCopying(void){
  while (!ptrStack.isEmpty()) {
    TypedPtr tptr    = ptrStack.pop();
    void *ptr         = getPtr(tptr);
    TypeOfPtr ptrType = getType(tptr);

    switch(ptrType) {

    case PTR_LTUPLE:
      ((LTuple *) ptr)->gcRecurse();
      break;

    case PTR_STUPLE:
      ((STuple *) ptr)->gcRecurse();
      break;

    case PTR_SRECORD:
      ((SRecord *) ptr)->gcRecurse();
      break;

    case PTR_NAME:
      ((Literal *) ptr)->gcRecurse ();
      break;

    case PTR_CONT:
      ((Continuation*) ptr)->gcRecurse();
      break;

    case PTR_SUSPCONT:
      ((SuspContinuation*) ptr)->gcRecurse();
      break;

    case PTR_CFUNCONT:
      ((CFuncContinuation*) ptr)->gcRecurse();
      break;

    case PTR_ACTOR:
       ((Actor *) ptr)->gcRecurse();
       break;

    case PTR_THREAD:
       ((Thread *) ptr)->gcRecurse();
       break;

    case PTR_BOARD:
       ((Board *) ptr)->gcRecurse();
       break;

     default:
       error("Unknown type tag on pointer stack: 0x%d",ptrType);
     }
   } // while
}




//*****************************************************************************
//             AM methods to launch gc under certain conditions
//*****************************************************************************

/*
#ifdef DEBUG_GC
#  define InitialBigGCLimit   (1 * (1<<20))
#  define InitialSmallGCLimit (1<<20)
#else
#  define InitialBigGCLimit   (9 * (1<<20))  // 9 * 2^10 = 9 MB
#  define InitialSmallGCLimit (1<<20)        // 1 * 2^10 = 1 MB
#endif
*/


// signal handler
void checkGC()
{
  Assert(!am.isCritical());
  if (getUsedMemory() > am.conf.heapMaxSize && am.conf.gcFlag) {
    am.setSFlag(StartGC);
  }
}

void AM::doGC()
{
  blockSignals();

  /*  --> empty trail */
  deinstallPath(rootBoard);

  /* do gc */
  gc(conf.gcVerbosity);

  /* calc upper limits for next gc */
  int used = getUsedMemory();
  if (used > (conf.heapMaxSize*conf.heapMargin)/100) {
    conf.heapMaxSize = conf.heapMaxSize*(100+conf.heapIncrement)/100;
  }

  unsetSFlag(StartGC);
  unblockSignals();
}


// pre-condition: root node is installed
Bool AM::idleGC()
{
  if (getUsedMemory() > (conf.heapIdleMargin*conf.heapMaxSize)/100 && conf.gcFlag) {
    if (conf.showIdleMessage) {
      printf("gc ... ");
      fflush(stdout);
    }
    int save = conf.gcVerbosity;
    conf.gcVerbosity = 0;
    doGC();
    conf.gcVerbosity = save;
    return OK;
  }
  return NO;
}

OzDebug *OzDebug::gcOzDebug()
{
  pred = pred->gcSRecord();
  args = gcRefsArray(args);
  return this;
}


#ifdef OUTLINE
#undef inline
#endif
