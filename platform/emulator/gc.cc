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
// #define TURNED_OFF

#include <ctype.h>

#include "../include/config.h"

#include "gc.hh"
#include "builtins.hh"
#include "actor.hh"
#include "am.hh"
#include "board.hh"
#include "cell.hh"
#include "dllist.hh"
#include "fdgenvar.hh"
#include "fdhook.hh"
#include "io.hh"
#include "misc.hh"
#include "objects.hh"
#include "stack.hh"
#include "thread.hh"
#include "ozdebug.hh"
#include "verbose.hh"

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
Bool isInTree(Board *b);

//*****************************************************************************
//               Macros and Inlines
//*****************************************************************************

#define MEMCPY(N,P,S) memcpy(N, P, S); INITMEM(P, S, 0xff); MOVEMSG(P, N, S);

#define CHECKCOLLECTED(elem,type)  \
  if (GCISMARKED((int)elem)) return (type) GCUNMARK((int) elem);


//// switches for debug macros
#define CHECKSPACE  // check if object is really copied from heap (1 chunk) 
// #define INITFROM    // initialise copied object
// #define VERBOSE     // inform user about current state of gc
// Note: in VERBOSE modus big external file (gc-debug.txt) is produced.
// It contains very detailed debug (trace) information; 

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
           error("NOT taken from heap.");
#   define INTOSPACE(P)                                                       \
      if (opMode == IN_GC && P != NULL && inChunkChain(heapGetStart(), (void*)P)) \
                             error("Taken from TO-SPACE 0x%x %d", P, __LINE__);
#   define TOSPACE(P)                                                         \
      if (opMode == IN_GC && P != NULL && !inChunkChain(heapGetStart(), (void*)P)) \
                        error("Not taken from TO-SPACE 0x%x %d", P, __LINE__);

#   define TAGINTOSPACE(T) if(!(isRef(T) || isAnyVar(T) ||                   \
                                tagTypeOf(T) == ATOM || tagTypeOf(T) == SMALLINT || \
			        tagTypeOf(T) == FLOAT || tagTypeOf(T) == BIGINT))   \
                          if (opMode == IN_GC &&                              \
			      inChunkChain(heapGetStart(), tagValueOf(T)))    \
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
#  define PUTCHAR(C) putc(C, verbOut)
#  define GCMETHMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s this: 0x%p :%d\n",                          \
	    (ptrStack.getUsed ()), S,this,__LINE__);                          \
    fflush(verbOut);
#  define GCNEWADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) --> 0x%p :%d\n",(void *)A,__LINE__);                \
    fflush(verbOut);
#  define GCOLDADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) <-- 0x%p :%d\n",(void *)A,__LINE__);                \
    fflush(verbOut);
#  define GCPROCMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s :%d\n",                                     \
	    (ptrStack.getUsed ()),S,__LINE__);                                \
    fflush(verbOut);
# define MOVEMSG(F,T,S)                                                       \
     fprintf(verbOut,"(gc) \t%d bytes moved from 0x%p to 0x%p\n",S,F,T);      \
     fflush(verbOut) 
#else
#  define PUTCHAR(C)
#  define GCMETHMSG(S)
#  define GCPROCMSG(S)
#  define GCNEWADDRMSG(A)
#  define GCOLDADDRMSG(A)
#  define MOVEMSG(F,T,S)
#endif

/*
 *   Modes of working:
 */
typedef enum {IN_GC = 0, IN_TC} GcMode;

GcMode opMode;
static int varCount;   // not only variables, but names, cells & abstractions;
static Board* fromCopyBoard;
static Board* toCopyBoard;

inline int  GCMARK(int S)      { return (S | GCTAG); }
inline int  GCMARK(void *S)    { return GCMARK((int) S); }
inline int  GCUNMARK(int S)    { return S & ~GCTAG; }
inline Bool GCISMARKED(int S)  { return S &  GCTAG ? OK : NO; }

inline
void fastmemcpy(int *to, int *frm, int sz)
{
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



inline
Bool needsNoCollection(TaggedRef t)
{
  Assert(t!=makeTaggedNULL());

  TypeOfTerm tag = tagTypeOf(t);
  return (tag == SMALLINT ||
	  (tag == ATOM && (tagged2Atom(t)->isDynXName ()) == NO))
         ? OK
	 : NO;
}

//*****************************************************************************
//                        Consistency checks of trails
//*****************************************************************************

inline
void RebindTrail::gc()
{
  Assert(empty());
}


// cursor points to next free position
inline
void Trail::gc()
{
  Assert(empty());
}


//*****************************************************************************
//               ADT  TypedPtr
//*****************************************************************************

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


typedef TaggedRef TaggedPtr;

inline
TaggedPtr makeTaggedPtr(void *ptr, TypeOfPtr tag)
{
  return makeTaggedRef((TypeOfTerm) tag, ptr);
}

inline
TypeOfPtr getType(TaggedPtr tp)
{
  return (TypeOfPtr) tagTypeOf(tp);
}

inline
void *getPtr(TaggedPtr tp)
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

DebugGCT(static int updateStackCount);


//*****************************************************************************
//                    Auxiliary stuff to collect TERMs
//*****************************************************************************


/*
 *  Destructive modification of term:
 *  If mode is IN_GC, store value in cell ptr only;
 *  else save cell at ptr and also store in this cell.
 *
 */
inline
void setHeapCell (int* ptr, int newValue)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(ptr, (int) *ptr);
  }
  DebugGC(opMode == IN_GC
	   && inChunkChain (heapGetStart (), (void *)ptr)
	   && GCISMARKED (newValue),
	   error ("storing marked value in 'TO' space"));
  DebugGC(opMode == IN_GC
	  && inChunkChain (from, (void *) GCUNMARK (newValue)),
	  error ("storing (marked) ref in to FROM-space"));
  *ptr = newValue;
}


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
void gcTaggedBlockRecurse(TaggedRef *block,int sz)
{
  for(int i = sz-1; i>=0; i--) {
    if (isRef(block[i]) || !isAnyVar(block[i])) {
      gcTagged(block[i],block[i]);
    }
  }
}


inline
Bool isLocalBoard (Board* b)
{
  return b->isPathMark() ? NO : OK;
}

inline 
Atom *Atom::gc()
{
  if (isDynXName() == NO) {
    return (this);
  }

  if (opMode == IN_GC || isLocalBoard (home) == OK) {
    GCMETHMSG("Atom::gc");
    CHECKCOLLECTED (printName, Atom *);
    varCount++;
    Name *aux = (Name *) gcRealloc (this,sizeof (*this));
    GCNEWADDRMSG (aux);
    ptrStack.push (aux, PTR_NAME);
    setHeapCell((int*) &printName, GCMARK(aux));
    return (aux);
  } else {
    return (this);
  }
}

inline 
void Atom::gcRecurse ()
{
  GCMETHMSG("Atom::gcRecurse");
  DebugGC((isDynXName () == NO),
	  error ("non-dynamic name is found in gcRecurse"));
  home = home->gcBoard ();
}

// WARNING: the value field of floats has no bit left for a gc mark
//   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
inline
Float *Float::gc()
{
  Float *ret =  new Float(value);
  return ret;
}

inline
BigInt *BigInt::gc()
{
  CHECKCOLLECTED(*(int *)&value.alloc, BigInt *);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  setHeapCell((int *)&value.alloc, GCMARK(ret));
  return ret;
}

inline
void ConsList::gc()
{
  GCMETHMSG("ConsList::gc");
  if(first){
    int sz = numbOfCons*sizeof(Equation);
    Equation *aux = (Equation*)gcRealloc(first,sz);
    GCNEWADDRMSG(aux);
    for(int i = 0; i < numbOfCons; i++){
#ifdef DEBUG_CHECK
      //  This is the very useful consistency check.
      //  'Equations' with non-variable at the left side are figured out;
      TaggedRef auxTerm = first[i].getLeft ();
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
	  error ("non-variable is found at left side in consList");
	} while (1);
      }
#endif
      gcTagged(*first[i].getLeftRef(),  *aux[i].getLeftRef()); 
      gcTagged(*first[i].getRightRef(), *aux[i].getRightRef());
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
  
  DebugGC(opMode == IN_TC && (!isLocalBoard(node->gcGetBoardDeref ()) ||
			      !isInTree(node->gcGetBoardDeref ())),
	  error ("non-local board in TC mode is being copied"));
  
  setHeapCell((int *)&pc, GCMARK(ret));
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

  INTOSPACE(r);

  CHECKCOLLECTED(r[-1], RefsArray);

  int sz = getRefsArraySize(r);

  RefsArray aux = allocateRefsArray(sz,NO);
  GCNEWADDRMSG(aux);

  if (isDirtyRefsArray(r)) {
    markDirtyRefsArray(aux);
  }

  DebugCheck(isFreedRefsArray(r),
	     markFreedRefsArray(aux););
  
  setHeapCell((int*) &r[-1], GCMARK(aux));

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
  node = node->gcBoard();
}

/* mm2: have to check for discarded node */
CFuncContinuation *CFuncContinuation::gcCont(void)
{
  GCMETHMSG("CFuncContinuation::gcCont");
  CHECKCOLLECTED(cFunc, CFuncContinuation *);
  
  CFuncContinuation *ret = (CFuncContinuation*) gcRealloc(this,sizeof(*this));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CFUNCONT);
  setHeapCell((int *)&cFunc, GCMARK(ret));
  return ret;
}


Continuation *Continuation::gc()
{
  GCMETHMSG("Continuation::gc");
  CHECKCOLLECTED(pc, Continuation *);
  
  Continuation *ret = (Continuation *) gcRealloc(this,sizeof(Continuation));
  GCNEWADDRMSG(ret);
  ptrStack.push(ret, PTR_CONT);
  setHeapCell((int *)&pc, GCMARK(ret));
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
  node=node->gcBoard();
  Continuation::gcRecurse();
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
  setHeapCell((int *)&label, GCMARK(ret));
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
  
  setHeapCell((int*) &args[0], GCMARK(ret));
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
    sz = sizeof(Abstraction);
    DebugGCT(if (opMode == IN_GC) INTOSPACE(((Abstraction *) this)->name););
    break;
  case R_OBJECT:
    sz = sizeof(Object);
    break;
  case R_CELL:
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
      if (((OneCallBuiltin *) this)->isSeen())
	((Builtin *) this)->gRegs = NULL;
      break;
    case BIsolved:
      sz = sizeof(SolvedBuiltin);
      break;
    default:
      sz = sizeof(Builtin);
    }
    break;
  default:
    error("SRecord::gc: unknown type");
  }

  SRecord *ret = (SRecord*) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_SRECORD);
  setHeapCell((int *)&u.type, GCMARK(ret));
  return ret;
}

// mm2: what shall we check here ???
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
    DebugCheck((b == am.rootBoard),
	       error ("isInTree (Board *): root Board is reached"));
    if (b == fromCopyBoard)
      return OK;
    b = b->getParentBoard();
    if (b != (Board *) NULL) 
      b = b->gcGetBoardDeref();
  }
  return NO;
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
  
  Board *el = getNode()->gcGetBoardDeref();

  if (isDead () || el == (Board *) NULL) {
    return ((Suspension *) NULL);
  }

  // mm2: el may be a GCMARK'ed board
  if (tcFlag == OK && isInTree(el) == NO) {
    return ((Suspension *) NULL);
  }
  
  Suspension *newSusp = (Suspension *) gcRealloc(this, sizeof(*this));
  GCNEWADDRMSG(newSusp);

  switch (flag & (S_cont|S_cfun)){
  case S_null:
    newSusp->item.node = item.node->gcBoard();
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
  } // for

  return ret;
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
      Board *home = tagged2VarHome(var);
      INFROMSPACE(home);
      home = home->gcBoard();
      TOSPACE (home);
      GCNEWADDRMSG((home ? makeTaggedUVar(home) : makeTaggedNULL()));
      return home ? makeTaggedUVar(home) : makeTaggedNULL();
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
	GCNEWADDRMSG(makeTaggedNULL());
	return makeTaggedNULL();
      }

      int cv_size;
      cv_size = sizeof(SVariable);
      
      SVariable *new_cv = (SVariable*)gcRealloc(cv,cv_size);
      
      setHeapCell((int *)cv, GCMARK(new_cv));
      
      if (opMode == IN_TC && new_cv->getHome () == fromCopyBoard)
	new_cv->suspList = new_cv->suspList->gc(OK);
      else
	new_cv->suspList = new_cv->suspList->gc(NO);
      
      DebugGC(new_cv->home == newBoard,
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
	GCNEWADDRMSG(makeTaggedNULL());
	return makeTaggedNULL();
      }

      int gv_size = gv->getSize();
      
      GenCVariable *new_gv = (GenCVariable*)gcRealloc(gv, gv_size);
      
      setHeapCell((int *)gv, GCMARK(new_gv));
      
      if (opMode == IN_TC && new_gv->getHome () == fromCopyBoard)
	new_gv->suspList = new_gv->suspList->gc(OK);
      else
	new_gv->suspList = new_gv->suspList->gc(NO);
      new_gv->gc();

      DebugGC(new_gv->home == newBoard,
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
  GCMETHMSG("GenFDVariable::gc");
  finiteDomain.gc();
  
  int i;
  if (opMode == IN_TC && getHome() == fromCopyBoard)
    for (i = 0; i < any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(OK);
  else
    for (i = 0; i < any; i++)
      fdSuspList[i] = fdSuspList[i]->gc(NO);
}


inline
Bool updateVar(TaggedRef var)
{
  GCPROCMSG("updateVar");
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
  GCPROCMSG("gcTagged");
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

  DebugGCT(INTOSPACE(auxTermPtr));

  switch (auxTermTag) {

  case SMALLINT:
    toTerm = auxTerm;
    return;

  case ATOM:
    toTerm = makeTaggedAtom (tagged2Atom (auxTerm)->gc ());
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
      DebugGCT(updateStackCount++);
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
  DebugGCT(updateStackCount = 0);
#ifdef TURNED_OFF
  fprintf(stdout, "I'm terribly sorry, but gc is currently turned off.\n"); 
  fflush(stdout);
  return;
#endif
  static int gcSoFar = 0;
  unsigned int utime = usertime();
  int i;
  // print initial message
  if (msgLevel>0) {
    message("\nHeap garbage collection in progress.\n");
  }

  char *oldChain = heapGetStart();
  unsigned int usedMem = getUsedMemory();
  unsigned int allocMem = getAllocatedMemory();
  INITCHECKSPACE;
  _arityTableOnce = 0;
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

  GCPROCMSG("Predicate table");
  CodeArea::gc();

  rootBoard=rootBoard->gcBoard();
  setCurrent(currentBoard->gcBoard(),NO);

  GCREF(currentThread);
  GCREF(rootThread);

  if (currentThread && currentThread->isNormal()) {
    am.currentTaskStack = currentThread->getTaskStack();
  } else {
    am.currentTaskStack = (TaskStack *) NULL;
  }

  if (FDcurrentTaskSusp != (Suspension *) NULL)
    FDcurrentTaskSusp = FDcurrentTaskSusp->gcSuspension (NO);
  Thread::GC();

  GCPROCMSG("ioNodes");
  for(i = 0; i < FD_SETSIZE; i++)
    if(FD_ISSET(i,&IO::globalReadFDs)) {
      if (i != fileno(IO::QueryFILE)) {
	IO::ioNodes[i] = IO::ioNodes[i]->gcBoard();
      }
    } 
  performCopying();

  GCPROCMSG("toplevelVars");
  am.toplevelVars = gcRefsArray(am.toplevelVars);

  GCPROCMSG("updating external references to terms into heap");
  ExtRefNode::gc();

  performCopying();

  // X regs initialization;
  for(i = 0; i < NumberOfXRegisters; i++)
    xRegs[i] = (TaggedRef) NULL;

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
  gcSoFar += (usedMem - getUsedMemory())/MB;
  utime = usertime() - utime;
  stat.timeForGC += utime;
  stat.heapAllocated += (usedMem - getUsedMemory());

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
      DebugGCT(updateStackCount--;);
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

      if (newVar == makeTaggedNULL()) {
	*Term = newVar;
	setHeapCell((int*) auxTermPtr,newVar);
      } else {
	switch(tagTypeOf(auxTerm)){
	case UVAR:
	  *Term = makeTaggedRef(newTaggedUVar(tagged2VarHome(newVar)));
	  break;
	case SVAR:
	  *Term = makeTaggedRef(newTaggedSVar(tagged2SVar(newVar)));
	  break;
	case CVAR:
	  *Term = makeTaggedRef(newTaggedCVar(tagged2CVar(newVar)));
	  break;
	default:
	  error("processUpdateStack: variable expected here.");
	} // switch
	INFROMSPACE(auxTermPtr);
	setHeapCell((int*) auxTermPtr,
		    GCMARK((int) *Term));
      }
    } // while

  DebugGC(updateStackCount != 0,
	  error ("updateStackCount != 0"));
}

// all nodes but node self
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
  error ("(gc) getPathMarks");
}

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
  error ("(gc) getPathMarks");
}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */
Board* AM::copyTree (Board* bb, Bool *isGround)
{
  GCMETHMSG(" ********** AM::copyTree **********");
  opMode = IN_TC;
  gcing = 0;
  varCount=0;
  stat.timeForCopy -= usertime();

  DebugGCT(updateStackCount = 0);
  DebugGC ((bb->isCommitted () == OK), error ("committed board to be copied"));
  fromCopyBoard = bb;
  setPathMarks(fromCopyBoard);
  toCopyBoard = fromCopyBoard->gcBoard();
  // kost@ : FDcurrentTaskSusp ???

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

void AbstractionTable::gc()
{
  GCMETHMSG("AbstractionTable::gc");
  AbstractionTable *aux = this;
  
  while(aux != NULL)
    {
      // there may be NULL entries in the table during gc
      aux->pred = (Abstraction *) aux->pred->gcSRecord();
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
      newStack->gcQueue(pop()); 			  // PC
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // Y
      newStack->gcQueue(gcRefsArray((RefsArray) pop()));  // G
      break;
      
    case C_XCONT:
      newStack->gcQueue(pop()); 			  // PC
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
    return ((Board *) this)->gcBoard();
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
  setHeapCell((int *)&flags, GCMARK(ret));
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
  notificationBoard = notificationBoard->gcBoard ();
}


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
  //         could be reached.
  if (opMode == IN_TC && isInTree (this) == NO) {
    setHeapCell ((int *) &suspCount, GCMARK(this));
    return (this);
  }
  size_t sz = sizeof(Board);
  Board *ret = (Board *) gcRealloc(this,sz);
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_BOARD);
  setHeapCell((int *)&suspCount, GCMARK(ret));
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
  script.gc();
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
  setHeapCell((int *)&priority, GCMARK(ret));
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

inline
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
      // "Atom *printName" needs no collection
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

    case PTR_NAME:
      ((Atom *) ptr)->gcRecurse ();
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

void checkGC() {
  if (getUsedMemory() > bigGCLimit && am.conf.gcFlag) {
    am.setSFlag(StartGC);
  }
}

void AM::doGC()
{
  //  --> empty trail
  deinstallPath(rootBoard);

  // do gc
  gc(am.conf.gcVerbosity);

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
  if (getUsedMemory() > smallGCLimit && am.conf.gcFlag) {
    if (am.conf.showIdleMessage) {
	message("gc ... ");
      }
    int save = am.conf.gcVerbosity;
    am.conf.gcVerbosity = 0;
    doGC();
    am.conf.gcVerbosity = save;
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

void regsInToSpace(TaggedRef *regs, int sz)
{
  for (int i=0; i<sz; i++) {
    checkInToSpace(regs[i]);
  }
}

#endif

OzDebug *OzDebug::gcOzDebug()
{
  pred = pred->gcSRecord();
  args = gcRefsArray(args);
  return this;
}


#ifdef OUTLINE
#undef inline
#endif
