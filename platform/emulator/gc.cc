/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#undef TURNED_OFF
#define TURNED_OFF

#include <ctype.h>

#include "../include/config.h"

#include "builtins.hh"
#include "actor.hh"
#include "am.hh"
#include "board.hh"
#include "cell.hh"
#include "dllist.hh"
#include "fdgenvar.hh"
#include "gc.hh"
#include "misc.hh"
#include "objects.hh"
#include "stack.hh"
#include "thread.hh"

#ifdef OUTLINE
#define inline
#endif

#define GCREF(field) if (field) { field = field->gc(); }

//*****************************************************************************
//               Forward Declarations
//*****************************************************************************

static void processUpdateStack (void);
void gcTagged(TaggedRef &fromTerm, TaggedRef &toTerm);
void performCopying(void);

//*****************************************************************************
//               Macros and Inlines
//*****************************************************************************

#define MEMCPY(N,P,S) memcpy(N, P, S); INITMEM(P, S, 0xff); MOVEMSG(P, N, S);

#define CHECKCOLLECTED(elem,type)  \
  if (GCISMARKED((int)elem)) return (type) GCUNMARK((int) elem);


#ifdef DEBUG_GC
#define DebugGc(Code) Code
#else
#define DebugGc(Code)
#endif

//// switches for debug macros
#define CHECKSPACE  // check if object is really copied from heap (1 chunk)
//#define INITFROM    // initialise copied object
//#define VERBOSE     // inform user about current state of gc

// the debug macros themselves
#ifndef DEBUG_GC
#  undef CHECKSPACE
#  undef INITFROM
#  undef VERBOSE
#endif


#ifdef CHECKSPACE
   void *from;
#   define INITCHECKSPACE printf("FROM-SPACE:\n");                            \
                          printChunkChain(from = heapGetStart());
#   define INFROMSPACE(P)                                                     \
      if (opMode == IN_GC && P != NULL && !inChunkChain(from, (void*)P))      \
         if ((void*)P < (void*)am.globalStore ||                              \
         ((void*)(am.globalStore + NumberOfYRegisters)) < (void*)P)           \
           error("NOT taken from heap.");
#   define INTOSPACE(P)                                                       \
      if (opMode == IN_GC && P != NULL && inChunkChain(heapGetStart(), (void*)P)) \
                             error("Taken from TO-SPACE 0x%x %d", P, __LINE__);
#   define TOSPACE(P)                                                         \
      if (opMode == IN_GC && P != NULL && !inChunkChain(heapGetStart(), (void*)P))\
                        error("Not taken from TO-SPACE 0x%x %d", P, __LINE__);

#   define TAGINTOSPACE(T) if(!(isRef(T) || isAnyVar(T) ||                  \
                                tagTypeOf(T) == ATOM || tagTypeOf(T) == SMALLINT || \
                                tagTypeOf(T) == FLOAT || tagTypeOf(T) == BIGINT))   \
                          if (opMode == IN_GC &&                              \
                              inChunkChain(heapGetStart(), tagValueOf(T)))       \
                              error("Taken from TO-SPACE 0x%x %d",            \
                                   tagValueOf(T), __LINE__);
#   define PRINTTOSPACE   printf("TO:\n");                                    \
                          printChunkChain(heapGetStart());
#else
#   define INITCHECKSPACE
#   define INFROMSPACE(P)
#   define INTOSPACE(P)
#   define TOSPACE(P)
#   define TAGINTOSPACE(T)
#   define PRINTTOSPACE
#endif

#ifdef INITFROM
#  define INITMEM(A,S,B)   memset(A,B,S)
#else
#  define INITMEM(A,S,B)
#endif

#ifdef VERBOSE
#  define PUTCHAR(C) putchar(C)
#  define GCMETHMSG(S)                                                        \
    fprintf(stdout,"%s this: 0x%p %s:%d\n",S,this,__FILE__,__LINE__);         \
    fflush(stdout);
# define GCPROCMSG(S)                                                         \
    fprintf(stdout,"%s %s:%d\n",S,__FILE__,__LINE__);                         \
    fflush(stdout);
# define MOVEMSG(F,T,S)                                                       \
     fprintf(stdout,"\t%d bytes moved from 0x%p to 0x%p\n",S,F,T);            \
     fflush(stdout)
#else
#  define PUTCHAR(C)
#  define GCMETHMSG(S)
#  define GCPROCMSG(S)
#  define MOVEMSG(F,T,S)
#endif

/*
 *   Modes of working:
 */
typedef enum {IN_GC = 0, IN_TC} GcMode;

GcMode opMode;
static int varCount;
static Board* fromCopyBoard;
static Board* toCopyBoard;

inline int  GCMARK(int S)      { return (S | GCTAG); }
inline int  GCMARK(void *S)    { return GCMARK((int) S); }
inline int  GCUNMARK(int S)    { return S & ~GCTAG; }
inline Bool GCISMARKED(int S)  { return S &  GCTAG ? OK : NO; }

inline void fastmemcpy(int *to, int *from, int size)
{
  switch(size) {
  case 36: *(to+8) = *(from+8);
  case 32: *(to+7) = *(from+7);
  case 28: *(to+6) = *(from+6);
  case 24: *(to+5) = *(from+5);
  case 20: *(to+4) = *(from+4);
  case 16: *(to+3) = *(from+3);
  case 12: *(to+2) = *(from+2);
  case  8: *(to+1) = *(from+1);
  case  4: *(to+0) = *(from+0);
    break;
  default:
    while(size>0) {
      *to++ = *from++;
      size -= sizeof(int);
    }
  }
}


inline void *gcRealloc(void *ptr, size_t size)
{
  void *ret = heapMalloc(size);
  DebugCheck(size%sizeof(int) != 0,
             error("gcRealloc: can only handle word sized blocks"););
  fastmemcpy((int*)ret,(int*)ptr,size);
  return ret;
}



inline Bool needsNoCollection(TaggedRef t)
{
  DebugCheck(t==makeTaggedNULL(),
             error("needsNoCollection: non NULL TaggedRef expected"););

  TypeOfTerm tag = tagTypeOf(t);
  return (tag == SMALLINT || tag == ATOM)
         ? OK
         : NO;
}

//*****************************************************************************
//                        Consistency checks of trails
//*****************************************************************************

inline void RebindTrail::gc()
{
  DebugCheck(!isEmpty(),
             error("Inconsistency found:\n\tRebind trail should be empty."););
}


// cursor points to next free position
inline void Trail::gc()
{
  DebugCheck(lowBound < cursor,
             error("Trail::gc: trail is not empty"););
}


//*****************************************************************************
//               ADT  TypedPtr
//*****************************************************************************

enum TypeOfPtr {
  PTR_LTUPLE,
  PTR_SRECORD,
  PTR_STUPLE,
  PTR_BOARD,
  PTR_ACTOR,
  PTR_THREAD,
  PTR_CONT,
  PTR_CFUNCONT,
  PTR_SUSPCONT
};


typedef TaggedRef TaggedPtr;

inline TaggedPtr makeTaggedPtr(void *ptr, TypeOfPtr tag)
{
  return makeTaggedRef((TypeOfTerm) tag, ptr);
}

inline TypeOfPtr getType(TaggedPtr tp)
{
  return (TypeOfPtr) tagTypeOf(tp);
}

inline void *getPtr(TaggedPtr tp)
{
  return tagValueOf(tp);
}

//*****************************************************************************
//               Recursion stack for GC
//*****************************************************************************


class TypedPtrStack: public Stack {
public:

  void push(void *ptr, TypeOfPtr type) {
    Stack::push((StackEntry)makeTaggedPtr(ptr,type));
  }

  TaggedPtr pop()  { return (TaggedPtr) Stack::pop(); }
};



//**************************************
//               SavedPtrStack
//**************************************

/*
 *  SavedPtrStack consists of SavedPtr's and is used by coping
 * of terms (in common sense) to save overwrited value.
 *
 *  We must make here some note:
 *  such technic is used since it is was already used (in gc):)),
 * and since it seems to be efficient.
 *  It ralays heavy on fact that first cell of every term contains
 * no data that can have GCTAG bit, and, moreover, this cell
 * contains no compiler-related information (for instance,
 * pointer to procedure table if we are using run-time resolution
 * of class' methods across class hierarchy
 */

class SavedPtrStack: public Stack {
public:
  void pushPtr(int* ptr, int value)
  {
    ensureFree(2);
    push(ptr,NO);
    push((StackEntry) value,NO);
  }
};


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
    while(help) {
      gcTagged(*(TaggedRef*)help->elem, *(TaggedRef*)help->elem);
      help = (ExtRefNode*) help->next;
    }
  }
};


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


//*****************************************************************************


class ExtRefsCopy;
static ExtRefsCopy *extRefsCopy = NULL;

class ExtRefsCopy: public DLList {
friend TaggedRef gcProtectCopy(TaggedRef r);
public:
  ExtRefsCopy(DLListEntry el, DLList*p, DLList*n): DLList(el,p,n) {};

  static void gc()
  {
    ExtRefsCopy *aux = extRefsCopy;
    while (aux) {
      gcTagged((TaggedRef&)aux->elem,(TaggedRef&)aux->elem);
      aux = (ExtRefsCopy*) aux->next;
    }
  }


  ExtRefsCopy *find(TaggedRef *r)
  {
    ExtRefsCopy *aux = this;
    while(aux) {
      if (& aux->elem == (DLListEntry *)r) {
        break;
      }
      aux = (ExtRefsCopy*) aux->next;
    }
    return aux;
  }
};

TaggedRef gcProtectCopy(TaggedRef r)
{
  CHECK_NONVAR(r);

  extRefsCopy = (ExtRefsCopy*) extRefsCopy->add((DLListEntry)r);
  DebugCheck(r != (TaggedRef)extRefsCopy->elem,
             error("Expecting 'DLList::add' to add at beginning of list"););
  return makeTaggedRef((TaggedRef*)&(extRefsCopy->elem));
}

void gcUnprotectCopy(TaggedRef *r)
{
  ExtRefsCopy *f = extRefsCopy->find(r);
  if (f == NULL) {
    error("gcUnprotectCopy: cannot find 0x%d",*r);
  } else {
    extRefsCopy = (ExtRefsCopy*)extRefsCopy->remove(f);
  }
}




//*****************************************************************************
//                          Global variables
//*****************************************************************************

// special treatment for static member data
static char _arityTableOnce;


class TaggedRefStack: public Stack {
public:
  void       push(TaggedRef *t) { Stack::push((StackEntry)t); }
  TaggedRef *pop()              { return (TaggedRef*) Stack::pop(); }
};

TaggedRefStack updateStack;

static TypedPtrStack ptrStack;

static SavedPtrStack savedPtrStack;

DebugGc(static int updateStackCount);


//*****************************************************************************
//                    Auxiliary stuff to collect TERMs
//*****************************************************************************


/*
 *  Destructive modification of term:
 *  If mode is IN_GC, store value in cell ptr only;
 *  else save cell at ptr and also store in this cell.
 *
 */
inline void setHeapCell (int* ptr, int newValue)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(ptr, (int) *ptr);
  }
#ifdef DEBUG_GC
  if (opMode == IN_GC &&
      inChunkChain (heapGetStart (), (void *)ptr) && GCISMARKED (newValue))
    error ("storing marked value in 'TO' space");
#endif
#ifdef DEBUG_GC
  if (opMode == IN_GC && inChunkChain (from, (void *) GCUNMARK (newValue)))
    error ("storing (marked) ref in to FROM-space");
#endif
  *ptr = newValue;
}


inline void gcTaggedBlock(TaggedRef *oldBlock, TaggedRef *newBlock,int size)
{
  for(int i = size-1; i>=0; i--) {
    if (!isRef(oldBlock[i]) && isAnyVar(oldBlock[i])) {
      gcTagged(oldBlock[i],newBlock[i]);
    }
  }
}

inline void gcTaggedBlockRecurse(TaggedRef *block,int size)
{
  for(int i = size-1; i>=0; i--) {
    if (isRef(block[i]) || !isAnyVar(block[i])) {
      gcTagged(block[i],block[i]);
    }
  }
}


inline Bool isLocalBoard (Board* b)
{
  return b->isPathMark() ? NO : OK;
}


// WARNING: the value field of floats has no bit left for a gc mark
//   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
inline Float *Float::gc()
{
  Float *ret =  new Float(value);
  return ret;
}

inline BigInt *BigInt::gc()
{
  CHECKCOLLECTED(*(int *)&value.alloc, BigInt *);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  setHeapCell((int *)&value.alloc, GCMARK(ret));
  return ret;
}

inline void ConsList::gc()
{
  GCMETHMSG("ConsList::gc");
  if(first){
    int size = numbOfCons*sizeof(Equation);
    Equation *aux = (Equation*)gcRealloc(first,size);
    for(int i = 0; i < numbOfCons; i++){
      gcTagged(*first[i].getLeftRef(),  *aux[i].getLeftRef());
      gcTagged(*first[i].getRightRef(), *aux[i].getRightRef());
    }

    first = aux;
  }
}

inline SuspContinuation *SuspContinuation::gcCont()
{
  if (this == NULL) return NULL;

  if (pc != NOCODE) {
    CHECKCOLLECTED(pc, SuspContinuation *)
  }

  SuspContinuation *ret = (SuspContinuation*) gcRealloc(this, sizeof(*this));
  ptrStack.push(ret, PTR_SUSPCONT);

#ifdef DEBUG_GC
  if (opMode == IN_TC && !isLocalBoard(node))
    error ("non-local board in TC mode is being copied");
#endif

  setHeapCell((int *)&pc, GCMARK(ret));
  return ret;
}

// AM::globalStore is the only RefsArray which is not allocated on heap but
// has to be garbage collected. There may be references to AM::globalStore
// from within the tree and that's why there is an if-statement at the
// beginning of gcRefsArray necessary.

// Structure of type 'RefsArray' (see ./tagged.h)
// r[0]..r[n-1] data
// r[-1] gc bit set --> has already been copied

RefsArray gcRefsArray(RefsArray r)
{
  if ( (r == NULL) || (r == am.globalStore))
    return r;

  INTOSPACE(r);

  CHECKCOLLECTED(r[-1], RefsArray);

  int size = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(size,NO);

  if (isDirtyRefsArray(r)) {
    markDirtyRefsArray(aux);
  }

  DebugCheck(isFreedRefsArray(r),
             markFreedRefsArray(aux););

  setHeapCell((int*) &r[-1], GCMARK(aux));

  for(int i = size-1; i >= 0; i--)
    gcTagged(r[i],aux[i]);

  return aux;
}

void CFuncContinuation::gcRecurse(void)
{
  xRegs = gcRefsArray(xRegs);
  GCREF(node);
}

CFuncContinuation *CFuncContinuation::gcCont(void)
{
  CHECKCOLLECTED(cFunc, CFuncContinuation *);

  CFuncContinuation *ret = (CFuncContinuation*) gcRealloc(this,sizeof(*this));
  ptrStack.push(ret, PTR_CFUNCONT);
  setHeapCell((int *)&cFunc, GCMARK(ret));
  return ret;
}


Continuation *Continuation::gc()
{
  CHECKCOLLECTED(pc, Continuation *);

  Continuation *ret = (Continuation *) gcRealloc(this,sizeof(Continuation));
  ptrStack.push(ret, PTR_CONT);
  setHeapCell((int *)&pc, GCMARK(ret));
  return ret;
}

inline void Continuation::gcRecurse(){
  yRegs = gcRefsArray(yRegs);
  gRegs = gcRefsArray(gRegs);
  xRegs = gcRefsArray(xRegs);
}

inline void SuspContinuation::gcRecurse(){
  GCREF(node);
  Continuation::gcRecurse();
}


inline STuple *STuple::gc()
{
  CHECKCOLLECTED(label, STuple *);

  STuple *ret = (STuple*) gcRealloc(this,getRealSize());
  ptrStack.push(ret,PTR_STUPLE);
  setHeapCell((int *)&label, GCMARK(ret));
  gcTaggedBlock(getRef(),ret->getRef(),getSize());
  return ret;
}


inline LTuple *LTuple::gc()
{
  CHECKCOLLECTED(args[0], LTuple *);

  LTuple *ret = (LTuple*) gcRealloc(this,sizeof(*this));
  ptrStack.push(ret,PTR_LTUPLE);

  gcTaggedBlock(args,ret->args,2);

  setHeapCell((int*) &args[0], GCMARK(ret));
  return ret;
}


inline SRecord *SRecord::gc()
{
  CHECKCOLLECTED(u.type, SRecord *);

  size_t size;

  switch(getType()) {

  case R_ABSTRACTION:
    size = sizeof(Abstraction);
    break;
  case R_OBJECT:
    size = sizeof(Object);
    break;
  case R_CELL:
    if (opMode == IN_TC &&
        !isLocalBoard(((Cell*)this)->getBoard()) ) {
      return this;
    }
    size = sizeof(Cell);
    break;
  case R_CHUNK:
  case R_RECORD:
    size = sizeof(SRecord);
    break;
  case R_BUILTIN:
    switch (((Builtin *) this)->getType()) {
    case BIsolveCont:
      size = sizeof(OneCallBuiltin);
      if (((OneCallBuiltin *) this)->isSeen())
        ((Builtin *) this)->gRegs = NULL;
      break;
    case BIsolved:
      size = sizeof(SolvedBuiltin);
      break;
    default:
      size = sizeof(Builtin);
    }
    break;
  default:
    error("SRecord::gc: unknown type");
  }

  SRecord *ret = (SRecord*) gcRealloc(this,size);
  ptrStack.push(ret,PTR_SRECORD);
  setHeapCell((int *)&u.type, GCMARK(ret));
  return ret;
}

ConstTerm *ConstTerm::gc()
{
  CHECKCOLLECTED(type, ConstTerm *);

  switch (typeOf()) {
  case Co_Board:
    return ((Board *) this)->gc();
  case Co_Actor:
    return ((Actor *) this)->gc();
  case Co_Thread:
    return ((Thread *) this)->gc();

  default:
    error("ConstTerm::gc: unknown tag 0x%x", typeOf());
    return NULL;
  }
}


// mm2: what shall we check here ???
inline Bool isInTree (Board *b)
{
  while (b != (Board *)NULL) {
    if (b == fromCopyBoard)
      return OK;
    b = b->getParentBoard()->getBoardDeref();
  }
  return NO;
}


/* return NULL if either is marked as dead
 * or contains pointer to dead node;
 */
inline Suspension *Suspension::gc(Bool tcFlag){
  if (this == NULL) return NULL;

  CHECKCOLLECTED(flag, Suspension*);

  Board *el = getNode()->gcGetBoardDeref();

  if (isDead() || !el) {
    return NULL;
  }

  if (tcFlag && !isInTree(el)) {
    return NULL;
  }

  Suspension *newSusp = (Suspension *) gcRealloc(this, sizeof(*this));

  switch (flag & (cont|cfun)){
  case null:
    newSusp->item.node = (Board*)item.node->gc();
    break;
  case cont:
    newSusp->item.cont = item.cont->gcCont();
    break;
  case cont|cfun:
    newSusp->item.ccont = item.ccont->gcCont();
    break;
  default:
    error("Unexpected case in Suspension::gc().");
  }

  DebugCheck(!getNode()->gcGetBoardDeref(),
             warning("gc: adding dead node (3)"));

  setHeapCell((int *)&flag, GCMARK(newSusp));
  return newSusp;
}


/* we reverse the order of the list,
 * but this should be no problem
 */
SuspList *SuspList::gc(Bool tcFlag)
{
  SuspList *ret = NULL;

  for(SuspList* help = this; help != NULL; help = help->next) {
    Suspension *aux = help->getSusp()->gc(tcFlag);
    if (aux == NULL)
      continue;

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
  } // for

  return ret;
}

Board *Board::gcGetBoardDeref()
{
  Board *bb = this;
  while (OK) {
    if (GCISMARKED(*(int*)bb)) {
      return (Board*) GCUNMARK(*(int*)bb);
    }
    if (bb->isDiscarded() || bb->isFailed()) {
      return NULL;
    } else if (bb->isCommitted()) {
      bb = bb->board;
    } else {
      return bb;
    }
  }
  error("Board::gcGetBoardDeref");
}

// This procedure derefences cluster chains and collects only the object at
// the end of such a chain.
Board *gcBoardChain(Board* bb)
{
  GCPROCMSG("gcBoardChain");

  bb=bb->gcGetBoardDeref();

  return bb ? bb->gc() : bb;
}

// This procedure collects the entry points into heap provided by variables,
// without copying the tagged reference of the variable itself.
TaggedRef gcVariable(TaggedRef var)
{
  TypeOfTerm varTag = tagTypeOf(var);

  switch(varTag){

  case UVAR:
    {
      Board *home = tagged2VarHome(var);
      INFROMSPACE(home);
      home = gcBoardChain(home);
      TOSPACE (home);
      return makeTaggedUVar(home);
    }

  case SVAR:
    {
      SVariable *cv = tagged2SVar(var);
      INFROMSPACE(cv);
      if (GCISMARKED(*(int*)cv))
        return makeTaggedSVar((SVariable*)GCUNMARK(*(int*)cv));

      int cv_size;
      cv_size = sizeof(SVariable);

      SVariable *new_cv = (SVariable*)gcRealloc(cv,cv_size);

      setHeapCell((int *)cv, GCMARK(new_cv));

      if (opMode == IN_TC && new_cv->getHome () == fromCopyBoard)
        new_cv->suspList = new_cv->suspList->gc(OK);
      else
        new_cv->suspList = new_cv->suspList->gc(NO);

#ifdef DEBUG_GC
      {
        Board *newBoard = gcBoardChain(new_cv->clusterNode);
        if (new_cv->clusterNode == newBoard)
          error ("home node of variable is not copied");
        new_cv->clusterNode = newBoard;
      }
#else
      new_cv->clusterNode = gcBoardChain(new_cv->clusterNode);
#endif

      return makeTaggedSVar(new_cv);
    }
  case CVAR:
    {
      GenCVariable *gv = tagged2CVar(var);

      INFROMSPACE(gv);
      if (GCISMARKED(*(int*)gv))
        return makeTaggedCVar((GenCVariable*)GCUNMARK(*(int*)gv));

      int gv_size = gv->getSize();

      GenCVariable *new_gv = (GenCVariable*)gcRealloc(gv, gv_size);

      setHeapCell((int *)gv, GCMARK(new_gv));

      if (opMode == IN_TC && new_gv->getHome () == fromCopyBoard)
        new_gv->suspList = new_gv->suspList->gc(OK);
      else
        new_gv->suspList = new_gv->suspList->gc(NO);
      new_gv->gc();

#ifdef DEBUG_GC
      {
        Board *newBoard = gcBoardChain(new_gv->clusterNode);
        if (new_gv->clusterNode == newBoard)
          error ("home node of variable is not copied");
        new_gv->clusterNode = newBoard;
      }
#else
      new_gv->clusterNode = gcBoardChain(new_gv->clusterNode);
#endif //DEBUG_GC

      return makeTaggedCVar(new_gv);
    }
  default:
    error("gcVariable: only variables allowed here.");
    return makeTaggedNULL();
  } // switch
}


void GenCVariable::gc(void){
  switch (type){
  case FDVariable:
    ((GenFDVariable*)this)->gc();
    break;
  default:
    error("Unexpected type generic variable at %s:%d.",
          __FILE__, __LINE__);
    break;
  }
} // GenCVariable::gc

void GenFDVariable::gc(void)
{
  finiteDomain.gc();

  int i;
  if (opMode == IN_TC && getHome() == fromCopyBoard)
    for (i = 0; i < any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(OK);
  else
    for (i = 0; i < any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(NO);
}


inline Bool updateVar(TaggedRef var)
{
  Bool toUpdate = OK;
  if (opMode == IN_TC) {
    if (isUVar(var)) {
      toUpdate = isLocalBoard(tagged2VarHome(var));
    } else {
      toUpdate = isLocalBoard((isCVar(var)?tagged2CVar(var):tagged2SVar(var))
                             ->getHome());
    }
  }

  return toUpdate;
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
  TaggedRef auxTerm = fromTerm;

  TaggedRef *auxTermPtr;

#ifdef DEBUG_GC
  auxTermPtr = NULL;
  if (opMode == IN_GC && inChunkChain (from, &toTerm))
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

  DebugGc(INTOSPACE(auxTermPtr));

  switch (auxTermTag) {

  case SMALLINT:
  case ATOM:
    toTerm = auxTerm;
    return;

  case LTUPLE:
    toTerm = makeTaggedLTuple(tagged2LTuple(auxTerm)->gc());
    return;

  case STUPLE:
    toTerm = makeTaggedSTuple(tagged2STuple(auxTerm)->gc());
    return;

  case SRECORD:
    toTerm = makeTaggedSRecord(tagged2SRecord(auxTerm)->gc());
    return;

  case CONST:
    toTerm = makeTaggedConst(tagged2Const(auxTerm)->gc());
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

      DebugGc(toTerm = fromTerm); // otherwise 'makeTaggedRef' complains
      if (updateVar(auxTerm)) {
        setHeapCell((int*) &fromTerm, GCMARK(makeTaggedRef(&toTerm)));

        // updating toTerm AFTER fromTerm:
        toTerm = gcVariable(auxTerm);
      } else {
        toTerm = fromTerm;
      }
      return;
    }

    // put address of ref cell to be updated onto update stack
    if (updateVar(auxTerm)) {
      updateStack.push(&toTerm);
      DebugGc(updateStackCount++);
      gcVariable(auxTerm);
      toTerm = (TaggedRef) auxTermPtr;
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
void AM::gc(int msgLevel) {
  opMode = IN_GC;
  gcing = 0;
  DebugGc(updateStackCount = 0);
#ifdef TURNED_OFF
  fprintf(stdout, "I'm terribly sorry, but gc is currently turned off.\n");
  fflush(stdout);
  return;
#endif
  static int gcSoFar = 0;
  long utime = usertime();
  int i;
  // print initial message
  if (msgLevel>0) {
    message("\nHeap garbage collection in progress.\n");

  } // if (msgLevel>0)

  char *oldChain = heapGetStart();
  unsigned int usedMem = getUsedMemory();
  unsigned int allocMem = getAllocatedMemory();
  INITCHECKSPACE;
  _arityTableOnce = 0;
  initMemoryManagement();

  { /* initialize X regs; this IS necessary ! */
    int size = getRefsArraySize(xRegs);
    for (int i=0; i < size; i++) {
      xRegs[i] = makeTaggedNULL();
    }
  }

//                 actual garbage collection starts here
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

// colouring root pointers grey
//-----------------------------------------------------------------------------

  trail.gc();
  rebindTrail.gc();

  GCPROCMSG("Predicate table");
  CodeArea::gc();

  Board::GC();
  Thread::GC();

  GCPROCMSG("ioNodes");
  for(i = 0; i < FD_SETSIZE; i++)
    if(FD_ISSET(i,&globalReadFDs)) {
      if (i != fileno(QueryFILE)) {
        GCREF(ioNodes[i]);
      }
    }
  performCopying();

  // collect only the entry points into heap (don't copy 'globalStore')
  GCPROCMSG("globalStore");
  for(i = 0; i < getRefsArraySize(globalStore); i++)
    gcTagged(globalStore[i],globalStore[i]);

  GCPROCMSG("updating external references to terms into heap");
  ExtRefNode::gc();
  ExtRefsCopy::gc();

  performCopying();

// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  GCPROCMSG("updating references");
  processUpdateStack ();

  if(!ptrStack.empty())
    error("ptrStack should be empty");
  PRINTTOSPACE;

  deleteChunkChain(oldChain);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

// print final message
  gcSoFar += (usedMem - getUsedMemory())/1048576;
  utime = usertime() - utime;
  timeForGC += utime;
  heapAllocated += (usedMem - getUsedMemory());

  if (msgLevel > 0) {
    fprintf(stdout,"Done (Disposed %d bytes in %d msec).\n",
            usedMem - getUsedMemory(),
            utime);
    if (msgLevel > 1) {
      fprintf(stdout,"Statistics:\t\t     before    afterwards\n");
      fprintf(stdout,"\tmemory used      %10d%14d\n",
              usedMem ,getUsedMemory());
      fprintf(stdout,"\tmemory allocated %10d%14d\n",
              allocMem ,getAllocatedMemory());
      fprintf(stdout,"Total garbage collected: %d Mega bytes\n",
              gcSoFar);
    } // if (msgLevel > 1)
    fflush(stdout);
  } // if (msgLevel > 0)
  gcing = 1;
} // AM::gc


/*
 *   Process updateStack -
 *
 */
void processUpdateStack(void)
{
 loop:

  while (!updateStack.empty())
    {
      TaggedRef *Term = updateStack.pop();
      DebugGc(updateStackCount--;);
      TaggedRef auxTerm     = *Term;
      TaggedRef *auxTermPtr = NULL;

      while(IsRef(auxTerm)) {
        if (GCISMARKED(auxTerm)) {
          *Term = makeTaggedRef((TaggedRef*)GCUNMARK(auxTerm));
          goto loop;
        }
        auxTermPtr = tagged2Ref(auxTerm);
        auxTerm = *auxTermPtr;
      }

      switch(tagTypeOf(auxTerm)){
      case UVAR:
        {
          TaggedRef newVar = gcVariable(auxTerm);
          *Term = makeTaggedRef(newTaggedUVar(tagged2VarHome(newVar)));
        }
        break;
      case SVAR:
        {
          TaggedRef newVar = gcVariable(auxTerm);
          *Term = makeTaggedRef(newTaggedSVar(tagged2SVar(newVar)));
        }
        break;
      case CVAR:
        {
          TaggedRef newVar = gcVariable(auxTerm);
          *Term = makeTaggedRef(newTaggedCVar(tagged2CVar(newVar)));
        }
        break;
      default:
        error("processUpdateStack: variable expected here.");
      } // switch
      INFROMSPACE(auxTermPtr);
      setHeapCell((int*) auxTermPtr, GCMARK(makeTaggedRef(tagged2Ref(*Term))));
    } // while

#ifdef DEBUG_GC
  if (updateStackCount != 0)
    error ("updateStackCount != 0");
#endif
}

// all nodes but node self
inline void setPathMarks (Board *bb)
{
  while (bb->isCommitted())
    {
      bb = bb->getBoard();
      bb->setPathMark();
    }
}

inline void unsetPathMarks (Board *bb)
{
  while (bb->isCommitted())
    {
      bb = bb->getBoard();
      bb->unsetPathMark();
    }
}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */
Board* AM::copyTree (Board* bb,Bool *isGround)
{
  opMode = IN_TC;
  gcing = 0;
  varCount=0;
  timeForCopy -= usertime();

  DebugGc(updateStackCount = 0);

  fromCopyBoard = bb;
  setPathMarks(fromCopyBoard);
  toCopyBoard = (Board *) fromCopyBoard->gc();

  performCopying();

  processUpdateStack();

  if (!ptrStack.empty())
    error("ptrStack should be empty");

  while (!savedPtrStack.empty()) {
    int value = (int)  savedPtrStack.pop();
    int* ptr  = (int*) savedPtrStack.pop();
    *ptr = value;
  }

  unsetPathMarks(fromCopyBoard);
  fromCopyBoard = NULL;
  gcing = 1;

  timeForCopy += usertime();
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

void AbstractionTable::gc()
{
  AbstractionTable *aux = this;

  while(aux != NULL)
    {
      // there may be NULL entries in the table during gc
      if (aux->pred) {
        if (GCISMARKED(*(int*)aux->pred)) {
          aux->pred = (Abstraction*) GCUNMARK(*(int*)aux->pred);
        } else {
          INFROMSPACE(aux->pred);
          Abstraction *newAddr =
            (Abstraction*) gcRealloc(aux->pred,sizeof(Abstraction));

          setHeapCell((int *)aux->pred, GCMARK(newAddr));
          aux->pred = newAddr;

          newAddr->gRegs = gcRefsArray(newAddr->gRegs);
        } //if
      } // if
      aux->left->gc();
      aux = aux->right;    // tail recursion optimization
    }
}

void CodeArea::gc()
{
  abstractionTab->gc();
}


TaskStack *TaskStack::gc()
{
  TaskStack *newStack = new TaskStack(size);
  Board *newBB;

  int usedSize = gcGetUsedSize();

  if (usedSize == 0) {
    return newStack;
  }

  newStack->gcInit(usedSize);

  while (!isEmpty()) {

    Board *bb = (Board *) pop();
    ContFlag cFlag = getContFlag(bb);
    bb = clrContFlag(bb, cFlag);
    RefsArray ra;

    switch (cFlag){
      case C_NERVOUS:
      newBB = bb->gc();
      if (newBB) {
        newStack->gcQueue(setContFlag(newBB,cFlag));
      }
      break;

    case C_XCONT:
    case C_CONT:
      // Continuation to continue at codearea PC
      newBB = bb->gc();
      if (!newBB) {
        pop(); // pc
        pop(); // Y
        pop(); // G

        if (cFlag == C_XCONT) {
          pop();
        }
        break;
      }

      newStack->gcQueue(setContFlag(newBB,cFlag));

      newStack->gcQueue(pop()); // pc
      // y
      ra = (RefsArray) pop();
      newStack->gcQueue(gcRefsArray(ra));
      // g
      ra = (RefsArray) pop();
      newStack->gcQueue(gcRefsArray(ra));

      if (cFlag == C_XCONT) {
        ra = (RefsArray) pop();
        newStack->gcQueue(gcRefsArray(ra));
      } // if
      break;

    case C_CFUNC_CONT:
      {
        // Continuation to continue at c codeaddress
        newBB = bb->gc();
        if (!newBB) {
          pop(); // BIFun
          pop(); // Suspension
          pop(); // x regs
          break;
        } // if

        newStack->gcQueue(setContFlag(newBB,cFlag));

        newStack->gcQueue(pop()); // BIFun

        Suspension* susp = (Suspension*) pop();
        newStack->gcQueue(susp->gc(NO));

        ra = (RefsArray) pop();
        newStack->gcQueue(gcRefsArray(ra));

        break;
      }

    default:
      error("Unexpected case in TaskStack::gc().");
      break;
    } // switch
  } // while not task stack is empty

  newStack->gcEnd(usedSize);

  return newStack;
} // TaskStack::gc


int TaskStack::gcGetUsedSize()
{
  int ret = 0;
  void **oldTop = top;

  while (!isEmpty()) {
    Board *n = (Board *) pop();
    ContFlag cFlag = getContFlag(n);
    n = clrContFlag(n, cFlag);

    switch (cFlag){
    case C_NERVOUS:
      if (n->gcGetBoardDeref()) {
        ret++;
      }
      break;

    case C_XCONT:
      pop(4);
      if (n->gcGetBoardDeref()) {
        ret += 5;
      }
      break;

    case C_CONT:
      pop(3);
      if (n->gcGetBoardDeref()) {
        ret += 4;
      }
      break;

    case C_CFUNC_CONT:
      pop(3);
      if (n->gcGetBoardDeref()) {
        ret += 4;
      }
      break;

    default:
      error("Unexpected case in TaskStack::gcGetUsedSize().");
      break;
    } // switch
  } // while
  top = oldTop;
  return ret;
}

//*****************************************************************************
//                           NODEs
//*****************************************************************************

void Thread::GC()
{
  GCREF(Head);
  GCREF(Tail);
  GCREF(Current);
  GCREF(Root);
  if (am.currentTaskStack && Current) {
    am.currentTaskStack=Current->taskStack;
  } else {
    am.currentTaskStack=(TaskStack *) NULL;
  }
}

Thread *Thread::gc()
{
  CHECKCOLLECTED(flags, Thread *);
  size_t size;
  size = sizeof(Thread);
  Thread *ret = (Thread *) gcRealloc(this,size);
  ptrStack.push(ret,PTR_THREAD);
  setHeapCell((int *)&flags, GCMARK(ret));
  return ret;
}

void Thread::gcRecurse()
{
  GCMETHMSG("Thread::gc");

  GCREF(next);
  GCREF(prev);

  if (isNormal()) {
    GCREF(taskStack);
  } else if (isWarm()) {
    suspension=suspension->gc(OK); // mm2 ???
  } else if (isNervous()) {
    GCREF(board);
  } else {
    error("Thread::gcRecurse");
  }
}

void Board::GC()
{
  GCREF(Root);
  Board::SetCurrent(Current->gc(),NO);
}

Board *Board::gc()
{
  CHECKCOLLECTED(flags, Board *);
  size_t size;
  size = sizeof(Board);
  Board *ret = (Board *) gcRealloc(this,size);
  ptrStack.push(ret,PTR_BOARD);
  setHeapCell((int *)&flags, GCMARK(ret));
  return ret;
}

void Board::gcRecurse()
{
  GCMETHMSG("Board::gc");
  if (isCommitted()) {
    body.defeat();
    GCREF(board);
  } else {
    body.gcRecurse();
    GCREF(actor);
  }
  script.gc();
}

Actor *Actor::gc()
{
  CHECKCOLLECTED(flags, Actor *);
  size_t size;
  if (isWait()) {
    size = sizeof(WaitActor);
  } else {
    size = sizeof(AskActor);
  }
  Actor *ret = (Actor *) gcRealloc(this,size);
  ptrStack.push(ret,PTR_ACTOR);
  setHeapCell((int *)&flags, GCMARK(ret));
  return ret;
}

void Actor::gcRecurse()
{
  GCMETHMSG("Actor::gc");
  next.gcRecurse();
  GCREF(board);
  if (isWait()) {
    ((WaitActor *)this)->gcRecurse();
  }
}

void WaitActor::gcRecurse()
{
  int no = (int) childs[-1];
  Board **newChilds=(Board **) heapMalloc((no+1)*sizeof(Board *));
  *newChilds++ = (Board *) no;
  for (int i=0; i < no; i++) {
    if (childs[i]) {
      newChilds[i] = childs[i]->gc();
    } else {
      newChilds[i] = (Board *) NULL;
    }
  }
  childs=newChilds;
}


//*****************************************************************************
//                           collectGarbage
//*****************************************************************************

#define ERROR(Fun, Msg)                                                       \
        error("%s in %s at %s:%d", Msg, Fun, __FILE__, __LINE__);

void SRecord::gcRecurse()
{
  switch (getType()) {

  case R_ABSTRACTION:
    {
      Abstraction *a = (Abstraction *) this;
      a->gRegs = gcRefsArray(a->gRegs);
      break;
    }

  case R_OBJECT:
    {
      Object *o      = (Object *) this;
      o->gRegs       = gcRefsArray(o->gRegs);
      o->cell        = (Cell*)o->cell->gc();
      o->fastMethods = o->fastMethods->gc();
      break;
    }

  case R_CELL:
    {
      Cell *c = (Cell *) this;
      c->clusterNode = gcBoardChain(c->clusterNode);
      gcTagged(c->val,c->val);
      break;
    }

  case R_BUILTIN:
    {
      Builtin *bi = (Builtin *) this;
      gcTagged(bi->suspHandler,bi->suspHandler);
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

  args = gcRefsArray(args);
}

inline void STuple::gcRecurse()
{
  gcTagged(label,label);
  gcTaggedBlockRecurse(getRef(),getSize());
}


inline void LTuple::gcRecurse()
{
  gcTaggedBlockRecurse(args,2);
}



void performCopying(void){
  while (!ptrStack.empty()) {
    TaggedPtr tptr    = ptrStack.pop();
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

#ifdef DEBUG_GC
#  define InitialBigGCLimit   (1 * (1<<20))
#  define InitialSmallGCLimit (1<<20)
#else
#  define InitialBigGCLimit   (9 * (1<<20))  // 9 * 2^10 = 9 MB
#  define InitialSmallGCLimit (1<<20)        // 1 * 2^10 = 1 MB
#endif

int bigGCLimit   = InitialBigGCLimit;
int smallGCLimit = InitialSmallGCLimit;

void AM::doGC()
{
  //  --> empty trail
  deinstallPath(Board::GetRoot());

  // do gc
  gc(gcVerbosity);

  // calc upper limits for next gc
  smallGCLimit  = getUsedMemory();
  bigGCLimit    = smallGCLimit + InitialBigGCLimit;
  smallGCLimit += InitialSmallGCLimit;
  unsetSFlag(StartGC);
}


// pre-condition: root node is installed
Bool AM::smallGC()
{
  // if machine is idle
  if (getUsedMemory() > smallGCLimit && gcFlag) {
    if (idleMessage)
      {
        statusMessage("doing gc during idle ");
      }
    int save = gcVerbosity;
    gcVerbosity = 0;
    doGC();
    gcVerbosity = save;
    return OK;
  }
  return NO;
}


#ifdef DEBUG_CHECK
void checkInToSpace(TaggedRef term)
{
  if (isRef (term) && term != makeTaggedNULL() &&
      !inChunkChain (heapGetStart (), (void *)term))
    error ("reference to 'from' space");
  if (!isRef (term)) {
    switch (tagTypeOf (term)) {
    case UVAR:
    case SVAR:
    case LTUPLE:
    case STUPLE:
    case CONST:
    case SRECORD:
      if (!inChunkChain (heapGetStart (), (void *)tagValueOf (term)))
        error ("reference to 'from' space");
      break;
    default:
      break;
    }
  }
}

void regsInToSpace(TaggedRef *regs, int size)
{
  for (int i=0; i<size; i++) {
    checkInToSpace(regs[i]);
  }
}

#endif


#ifdef OUTLINE
#undef inline
#endif
