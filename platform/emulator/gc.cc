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

/****************************************************************************
 *               Forward Declarations
 ****************************************************************************/

void processUpdateStack(void);
void performCopying(void);

/****************************************************************************
 *   Modes of working:
 *     IN_GC: garbage collecting
 *     IN_TC: term copying
 ****************************************************************************/

Bool gc_is_running = NO;

typedef enum {IN_GC = 0, IN_TC} GcMode;

static GcMode opMode;

#ifdef CS_PROFILE
int32 * cs_copy_start = NULL;
int32 * cs_orig_start = NULL;
int     cs_copy_size  = 0;
#endif


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


#ifdef VERBOSE

#define VERB_OUTPUT   "/tmp/verb-out-%d.txt"

void verbReopen ();

FILE *verbOut = (FILE *) NULL;

void verbReopen ()
{
  if (verbOut != (FILE *) NULL)
    (void) fclose (verbOut);

  char *filename = new char[100];
  sprintf(filename,VERB_OUTPUT,osgetpid());
  fprintf(stderr, "verbose output to file %s\n",filename);

  verbOut = fopen (filename, "w");
  delete [] filename;
}

#endif // VERBOSE


/*
 * CHECKSPACE -- check if object is really copied from heap
 *   has as set of macros:
 *    INITCHECKSPACE - save pointer to from-space & print from-space
 *    NOTINTOSPACE   - assert not in to-space
 *    INTOSPACE      - assert in to-space
 * NOTE: this works only for chunk
 */

static MemChunks *fromSpace;

Bool inToSpace(void *p)
{
  return (opMode==IN_TC || p==NULL || MemChunks::list->inChunkChain(p));
}

Bool notInToSpace(void *p)
{
  return (opMode==IN_TC || p==NULL || !MemChunks::list->inChunkChain(p));
}

Bool inFromSpace(void *p)
{
  return (opMode==IN_TC || p==NULL || fromSpace->inChunkChain(p));
}

void initCheckSpace()
{
  fromSpace = MemChunks::list;
  DebugGCT(printf("FROM-SPACE:\n");
	   fromSpace->print();)
}

void exitCheckSpace()		
{
  DebugGCT(printf("TO-SPACE:\n");
	   MemChunks::list->print();)
}

#ifdef CHECKSPACE

#define INFROMSPACE(P)  Assert(inFromSpace(P))
#define NOTINTOSPACE(P) Assert(notInToSpace(P))
#define INTOSPACE(P)    Assert(inToSpace(P))

#else

#define INFROMSPACE(P)
#define NOTINTOSPACE(P)
#define INTOSPACE(P)

#endif



/*
 * VERBOSE  --- print various debug information to the file verb-out.txt
 */

#ifdef VERBOSE
#  define GCMETHMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s this: 0x%p ",(ptrStack.getUsed ()),S,this); \
    WHERE(verbOut);							      \
    fprintf(verbOut,"\n");						      \
    fflush(verbOut);
#  define GCNEWADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) --> 0x%p ",(void *)A);		              \
    WHERE(verbOut);							      \
    fprintf(verbOut,"\n");						      \
    fflush(verbOut);
#  define GCOLDADDRMSG(A)                                                     \
    fprintf(verbOut,"(gc) <-- 0x%p ",(void *)A);		              \
    WHERE(verbOut);							      \
    fprintf(verbOut,"\n");						      \
    fflush(verbOut);
#  define GCPROCMSG(S)                                                        \
    fprintf(verbOut,"(gc) [%d] %s ",(ptrStack.getUsed ()),S);		      \
    WHERE(verbOut);							      \
    fprintf(verbOut,"\n");						      \
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
 * TC: groundness check needs to count the number of
 *    variables, names, cells & abstractions
 */

static Bool varCopied;

#define setVarCopied \
 varCopied = OK;


/*
 * TC: the copy board in from-space and to-space
 */
static Board* fromCopyBoard;
static Board* toCopyBoard;


/****************************************************************************
 * Copy from from-space to to-space
 ****************************************************************************/
inline
void fastmemcpy_static(int32 *to, int32 *frm, size_t sz)
{
#ifdef VERBOSE
  fprintf(verbOut,"(gc) \tcopy %d bytes from 0x%p to 0x%p\n",sz,frm,to);
#endif
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
}

inline
void fastmemcpy_dynamic(int32 *to, int32 *frm, size_t sz)
{
#ifdef VERBOSE
  fprintf(verbOut,"(gc) \tcopy %d bytes from 0x%p to 0x%p\n",sz,frm,to);
#endif 
  sz = sz >> 2;
  while (sz > 8) {
    to[7] = frm[7]; to[6] = frm[6]; to[5] = frm[5]; to[4] = frm[4];
    to[3] = frm[3]; to[2] = frm[2]; to[1] = frm[1]; to[0] = frm[0];
    to += 8; frm += 8; sz -= 8;
  }
  switch(sz) {
  case 8: to[7] = frm[7];
  case 7: to[6] = frm[6];
  case 6: to[5] = frm[5];
  case 5: to[4] = frm[4];
  case 4: to[3] = frm[3];
  case 3: to[2] = frm[2];
  case 2: to[1] = frm[1];
  case 1: to[0] = frm[0];
  }
}

inline
void *gcReallocStatic(void *ptr, size_t sz)
{
  void *ret = heapMalloc(sz);
  DebugCheck(sz%sizeof(int) != 0,
	     error("gcReallocStatic: can only handle word sized blocks"););
  fastmemcpy_static((int32*)ret,(int32*)ptr,sz);
  return ret;
}


inline
void *gcReallocDynamic(void *ptr, size_t sz)
{
  void *ret = heapMalloc(sz);
  DebugCheck(sz%sizeof(int) != 0,
	     error("gcReallocDynamic: can only handle word sized blocks"););
  fastmemcpy_dynamic((int32*)ret,(int32*)ptr,sz);
  return ret;
}


/*****************************************************************************
 * makeTaggedRef without consitency check
 *****************************************************************************/

inline
TaggedRef makeTaggedRefToFromSpace(TaggedRef *s)
{
  CHECK_POINTER_N(s);
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
  PTR_LTUPLE_TAIL,
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

inline
TypedPtr makeTypedPtr(void *ptr, TypeOfPtr tag)
{
  return makeTaggedRef2p((TypeOfTerm) tag, ptr);
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
  Bool locked;
  TypedPtrStack() : Stack(1000,Stack_WithMalloc), locked(NO) {}
  ~TypedPtrStack() {}
  
  void lock()   { locked = OK; }
  void unlock() { locked = NO; }

  void push(void *ptr, TypeOfPtr type) {
    Assert(!locked);
    Stack::push((StackEntry)makeTypedPtr(ptr,type));
  }
  void pushInt(int n) {
    Assert(!locked);
    Stack::push((StackEntry) n);
  }

  TypedPtr pop()  { 
    return (TypedPtr) ToInt32(Stack::pop()); 
  }
  TypedPtr popInt()  { 
    return (int) ToInt32(Stack::pop()); 
  }
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
inline int32  GCMARK(void *S)    { return makeTaggedRef2p(GCTAG,S); }
inline int32  GCMARK(int32 S)    { return makeTaggedRef2i(GCTAG,S); }

inline void *GCUNMARK(int32 S)   { return tagValueOf2(GCTAG,S); }
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
void storeFwd (int32* fromPtr, void *newValue, Bool domark=OK)
{
  if (opMode == IN_TC) {
    savedPtrStack.pushPtr(fromPtr);
  }
  Assert(inFromSpace(fromPtr));
  //  Assert(inToSpace(newValue));
  *fromPtr = domark ? GCMARK(newValue) : ToInt32(newValue);
}

inline
void storeFwd(void* fromPtr, void *newValue)
{
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
	OZ_updateHeapTerm(*ret->elem);
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


/****************************************************************************
 * The update stack contains REF's to not yet copied variables
 ****************************************************************************/



class UpdateStack: public Stack {
  Bool locked;
public:
  UpdateStack() : Stack(1000,Stack_WithMalloc), locked(NO) {}
  ~UpdateStack() {}
  void push(TaggedRef *t) {
    Assert(!locked);
    Stack::push((StackEntry)t);
  }
  TaggedRef *pop() { return (TaggedRef*) Stack::pop(); }

  void lock()   { locked = OK; } 
  void unlock() { locked = NO; } 
};

static UpdateStack updateStack;


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
  Assert(!bb->_isRoot());
  do {
    bb = bb->getParent();
    bb->setPathMark();
  } while (!bb->_isRoot());
}

/*
 * TC: after copying
 */

inline
void unsetPathMarks(Board *bb)
{
  Assert(!bb->_isRoot());
  do {
    bb = bb->getParent();
    bb->unsetPathMark();
  } while (!bb->_isRoot());
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

// This procedure derefences cluster chains and collects only the object at 
// the end of such a chain.


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

inline
Board * Board::gcDerefedBoard() {
  GCMETHMSG("Board::gcDerefedBoard");
  INFROMSPACE(this);

  Assert(!isCommitted());

  Board * bb = this;

  Assert(bb);

  if (gcIsMarked())
    return gcGetFwd();
  
  if (!bb->gcIsAlive()) 
    return 0;

  // kost@, TMUELLER
  // process.oz causes the assertion to fire!!! 
  // Presumably a register allocation problem
  // See the cure below:
  //  if (opMode == IN_TC && !isInTree(bb)) return 0; 

  // In an optimized machine the assertion is absent
  Assert(opMode != IN_TC || isInTree(bb)); 

  COUNT(board);

  Assert(opMode==IN_TC || !inToSpace(bb));

  Board *ret = (Board *) gcReallocStatic(bb, sizeof(Board));
      
  FDPROFILE_GC(cp_size_board, sizeof(Board));
	
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_BOARD);

  storeFwdField(ret);

  return ret;
}

Board * Board::gcDerefedBoardOutline() {
  
  return gcDerefedBoard();

}

Board * Board::gcBoard() {
  GCMETHMSG("Board::gcBoard");
  INFROMSPACE(this);

  if (!this) return 0;

  return this->derefBoard()->gcDerefedBoard();
}



/*
 * Literals:
 *   3 cases: atom, optimized name, dynamic name
 *   only dynamic names need to be copied
 */

void dogcGName(GName *gn)
{
  if (gn && opMode == IN_GC) {
    gcGName(gn);
  }
}

inline
Name *Name::gcName()
{
  CHECKCOLLECTED(homeOrGName, Name *);
  GName *gn = NULL;
  if (hasGName()) {
    gn = getGName();
  }
  if (opMode == IN_GC && isOnHeap() ||
      opMode == IN_TC && isLocalBoard(GETBOARD(this))) {
    GCMETHMSG("Name::gc");
    COUNT(literal);
    setVarCopied;
    Name *aux = (Name*) gcReallocStatic(this,sizeof(Name));
    GCNEWADDRMSG(aux);
    ptrStack.push(aux, PTR_NAME);
    storeFwd(&homeOrGName, aux);
    
    FDPROFILE_GC(cp_size_literal,sizeof(*this));
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
  GCMETHMSG("Name::gcRecurse");
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
SuspList * SuspList::gc()
{
  GCMETHMSG("SuspList::gc");

  SuspList *ret = NULL;

  for (SuspList* help = this; help != NULL; help = help->next) {
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
void OZ_FiniteDomainImpl::gc(void)
{
  FDPROFILE_GC(cp_size_fdvar, getDescrSize());

  copyExtension();
}

inline
void GenFDVariable::gc(GenFDVariable * frm)
{
  GCMETHMSG("GenFDVariable::gc");

  finiteDomain = frm->finiteDomain;
  ((OZ_FiniteDomainImpl *) &finiteDomain)->gc();
  
  int i;
  for (i = fd_prop_any; i--; )
    fdSuspList[i] = frm->fdSuspList[i]->gc();
}

inline
void GenBoolVariable::gc(GenBoolVariable * frm)
{
  GCMETHMSG("GenFDVariable::gc");

  store_patch = frm->store_patch;
  
}

inline
void GenFSetVariable::gc()
{
  GCMETHMSG("GenFSetVariable::gc");

  int i;
  for (i = fs_prop_any; i--; )
    fsSuspList[i] = fsSuspList[i]->gc();
}

inline
FSetValue * FSetValue::gc(void) 
{
  return (FSetValue *) gcReallocDynamic(this, sizeof(FSetValue));
}

inline
void GenLazyVariable::gc(void)
{
  GCMETHMSG("GenLazyVariable::gc");
  if (function!=0) {
    OZ_updateHeapTerm(function);
    OZ_updateHeapTerm(result);
  }
}

inline
void GenMetaVariable::gc(void)
{
  GCMETHMSG("GenMetaVariable::gc");
  OZ_updateHeapTerm(data);
}

inline
void AVar::gc(void)
{ 
  GCMETHMSG("AVar::gc");
  OZ_updateHeapTerm(value);
}

inline
DynamicTable* DynamicTable::gc(void)
{
  GCMETHMSG("DynamicTable::gc");

  Assert(isPwrTwo(size));
  // Copy the table:
  COUNT(dynamicTable);
  COUNT1(dynamicTableLen,size);
  size_t len = (size-1)*sizeof(HashElement)+sizeof(DynamicTable);
  DynamicTable* ret = (DynamicTable*) gcReallocDynamic(this,len);
  
  FDPROFILE_GC(cp_size_ofsvar, len);
  
  GCNEWADDRMSG(ret);
  // Take care of all TaggedRefs in the table:
  ptrStack.push(ret,PTR_DYNTAB);
  // (no storeFwd needed since only one place points to the dynamictable)

#ifdef DEBUG_CHECK
  for (dt_index i=size; i--; ) {
    if (table[i].ident != makeTaggedNULL()) {
      Assert(!isDirectVar(table[i].ident));
      Assert(!isDirectVar(table[i].value));
    }
  }
#endif
  
  return ret;
}

inline
void DynamicTable::gcRecurse() {
  for (dt_index i=size; i--; ) {
    if (table[i].ident != makeTaggedNULL()) {
      OZ_updateHeapTerm(table[i].ident);
      OZ_updateHeapTerm(table[i].value);
    }
  }
}


inline
void GenOFSVariable::gc(void)
{
    GCMETHMSG("GenOFSVariable::gc");
    OZ_updateHeapTerm(label);
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
Bool GenCVariable::gcNeeded(void) {

  if (this->gcIsMarked()) {
    Assert(!home->isCommitted());
    return OK;
  }
  
  Board * bb = home->derefBoard();

  home = bb;

  return ((opMode == IN_GC) || isLocalBoard(bb));
}
  
inline
GenCVariable * GenCVariable::gc(void) {
  INFROMSPACE(this);

  Assert(!home->isCommitted());

  Assert(gcNeeded());

  if (gcIsMarked())
    return gcGetFwd();
  
  Board * bb = home->gcDerefedBoard();
  
  Assert(bb);
  
  SuspList * sl = suspList;
  
  setVarCopied;

  GenCVariable * to;
    
  switch (getType()){
  case FDVariable:
    to = (GenCVariable *) heapMalloc(sizeof(GenFDVariable));
    to->u = this->u;
    storeFwdField(to);
    ((GenFDVariable *) to)->gc((GenFDVariable *) this);
    FDPROFILE_GC(cp_size_fdvar, sizeof(GenFDVariable));
    break;
  case BoolVariable:
    to = (GenCVariable *) heapMalloc(sizeof(GenBoolVariable));
    to->u = this->u;
    storeFwdField(to);
    ((GenBoolVariable *) to)->gc((GenBoolVariable *) this);
    FDPROFILE_GC(cp_size_boolvar, sizeof(GenBoolVariable));
    break;
  case OFSVariable:
    to = (GenCVariable *) gcReallocStatic(this, sizeof(GenOFSVariable));
    storeFwdField(to);
    ((GenOFSVariable *) to)->gc();
    FDPROFILE_GC(cp_size_ofsvar, sizeof(GenOFSVariable));
    break;
  case MetaVariable:
    to = (GenCVariable *) gcReallocStatic(this, sizeof(GenMetaVariable));
    storeFwdField(to);
    ((GenMetaVariable *) to)->gc();
    FDPROFILE_GC(cp_size_metavar, sizeof(GenMetaVariable));
    break;
  case AVAR:
    to = (GenCVariable *) gcReallocStatic(this, sizeof(AVar));
    storeFwdField(to);
    ((AVar *) to)->gc();
    break;
  case PerdioVariable:
    to = (GenCVariable *) gcReallocStatic(this, sizeof(PerdioVar));
    storeFwdField(to);
    ((PerdioVar *) to)->gc();
    break;
  case FSetVariable:
    // TMUELLER: must be reset to `gcReallocStatic'
    to = (GenCVariable *) gcReallocDynamic(this, sizeof(GenFSetVariable));
    storeFwdField(to);
    ((GenFSetVariable *) to)->gc();
    break;
  case LazyVariable:
    to = (GenCVariable *) gcReallocStatic(this, sizeof(GenLazyVariable));
    storeFwdField(to);
    ((GenLazyVariable*) to)->gc();
    break;
  default:
    Assert(0);
  }

  Assert(opMode != IN_GC || this->home != bb);
    
  to->suspList = sl->gc();
  to->home     = bb;
    
  return to;
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
Bool SVariable::gcNeeded(void) {

  if (this->gcIsMarked()) {
    Assert(!home->isCommitted());
    return OK;
  }
  
  Board * bb = home->derefBoard();

  home = bb;

  return ((opMode == IN_GC) || isLocalBoard(bb));
}
  
inline
SVariable * SVariable::gc() {
  Assert(!home->isCommitted());

  Assert(gcNeeded());

  if (gcIsMarked()) 
    return gcGetFwd();
  
  Board * bb = home->gcDerefedBoard();
    
  Assert(bb);
  
  setVarCopied;

  SVariable * to = (SVariable *) heapMalloc(sizeof(SVariable));
  
  Assert(opMode != IN_GC || to->home != bb);

  to->suspList = suspList->gc();
  to->home     = bb;

  storeFwdField(to);
  

  return to;
}


inline
Bool gcUVarNeeded(TaggedRef var) {
  Assert(isUVar(var));

  Board * bb = tagged2VarHome(var)->derefBoard();

  return ((opMode == IN_GC) || isLocalBoard(bb));
}


inline
TaggedRef gcUVar(TaggedRef var) {
  GCPROCMSG("gcUVar");
  
  Assert(isUVar(var));

  Board * bb = tagged2VarHome(var)->derefBoard();

  Assert(((opMode == IN_GC) || isLocalBoard(bb)));

  bb = bb->gcDerefedBoard();

  Assert(bb);
    
  setVarCopied;

  INTOSPACE(bb);
  
  return makeTaggedUVar(bb);
}


void gcDirectVar(TaggedRef * frm, TaggedRef * to) {
  GCPROCMSG("gcDirectVar");
  TaggedRef var = * frm;

  Assert(isDirectVar(var));
  
  switch (tagTypeOf(var)) {
  case CVAR:
    {
      GenCVariable * cv = tagged2CVar(var);

      if (cv->gcNeeded()) { 
	*to = makeTaggedCVar(cv->gc());
      } else {
	// We cannot copy the variable, but we have already copied
	// their taggedref, so we change the original variable to a ref 
	// of the copy.
	// After pushing on the update stack the
	// the original variable is replaced by a reference!
	*to  = *frm;
	*frm = makeTaggedRef(to);
      }

      break;
    }
  case SVAR:
    {
      SVariable * sv = tagged2SVar(var);

      if (sv->gcNeeded()) { 
	*to = makeTaggedSVar(sv->gc());
      } else {
	// See above
	*to  = *frm;
	*frm = makeTaggedRef(to);
      }

      break;
    }

  case UVAR:
    
    if (gcUVarNeeded(var)) {
      *to = gcUVar(var);
    } else {
      // See above
      *to  = *frm;
      *frm = makeTaggedRef(to);
    }
    break;
    
  default:
    Assert(0);
    break;
  }
}

/*
 * Float
 * WARNING: the value field of floats has no bit left for a gc mark
 *   --> copy every float !! so that X=Y=1.0 --> X=1.0, Y=1.0
 */
inline
Float *Float::gc() {
  Assert(opMode == IN_GC);

  COUNT(ozfloat);

  Float *ret =  newFloat(value);
  return ret;
}

inline
BigInt * BigInt::gc() {
  Assert(opMode == IN_GC);

  CHECKCOLLECTED(*(int *)&value, BigInt *);
  COUNT(bigInt);

  BigInt *ret = new BigInt();
  mpz_set(&ret->value,&value);
  storeFwd((int *)&value, ret);
  return ret;
}

inline
void Script::gc()
{
  GCMETHMSG("Script::gc");
  if (first){
    int sz = numbOfCons*sizeof(Equation);
    COUNT1(scriptLen,sz);
    Equation *aux = (Equation*) heapMalloc(sz);
    GCNEWADDRMSG(aux);
    
    FDPROFILE_GC(cp_size_script, sz);

    for (int i = numbOfCons; i--; ){
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
      aux[i] = first[i];
      Assert(!isDirectVar(first[i].left));
      Assert(!isDirectVar(first[i].right));
      OZ_updateHeapTerm(aux[i].left); 
      OZ_updateHeapTerm(aux[i].right);
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
  storeFwd((int32*)&r[-1],ToPointer(ToInt32(ptr)|RAGCTag),NO);
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

  for (int i = sz; i--; ) {
    aux[i] = r[i];
    Assert(!isDirectVar(r[i]));
    OZ_updateHeapTerm(aux[i]);
  }
  
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
    (RunnableThreadBody *) gcReallocStatic(this, sizeof(RunnableThreadBody));
  GCNEWADDRMSG (ret);
  taskStack.gc(&ret->taskStack);

  return (ret);
}

OZ_Propagator * OZ_Propagator::gc(void)
{
  GCMETHMSG("OZ_Propagator::gc");
  GCOLDADDRMSG(this);

  OZ_Propagator * p = (OZ_Propagator *) gcReallocDynamic(this, sizeOf());

  GCNEWADDRMSG (p);

  ptrStack.push(p, PTR_PROPAGATOR);
  
  return p;
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

Continuation *Continuation::gc()
{
  GCMETHMSG ("Continuation::gc");

  if (gcIsMarked())
    return gcGetFwd();
  
  COUNT(continuation);
  Continuation *ret =
    (Continuation *) gcReallocStatic(this, sizeof(Continuation));
  GCNEWADDRMSG (ret);
  ptrStack.push (ret, PTR_CONT);

  storeFwdField(ret);

  FDPROFILE_GC(cp_size_cont, sizeof(Continuation));

  return (ret);
}

/* collect LTuple, SRecord */

inline
LTuple * LTuple::gc() {
  GCMETHMSG("LTuple::gc");

  LTuple * l = this;

  TaggedRef start;
  TaggedRef * p = &start;

  int n = 0;

  while (1) {
    TaggedRef h = l->args[0];

    // Check wether already collected
    if (GCISMARKED(h)) {
      if (n > 0) {
	*p = makeTaggedLTuple((LTuple *) GCUNMARK(h));

	LTuple * ret = tagged2LTuple(start);

	ptrStack.pushInt(n);
	ptrStack.push(ret, PTR_LTUPLE);

	return ret;
      } else {
	return ((LTuple *) GCUNMARK(h));
      }
    }

    n++;

    LTuple * c = (LTuple *) heapMalloc(sizeof(LTuple));

    *p = makeTaggedLTuple(c);
    p  = &(c->args[1]);
    
    // Process list element
    if (isDirectVar(h)) {
      gcDirectVar(&(l->args[0]), &(c->args[0]));
    } else {
      c->args[0] = h;
    }

    // Store forward
    storeFwd(&(l->args[0]), &(c->args[0]));
    
    TaggedRef t = l->args[1];

    // Process list tail
    if (isDirectVar(t)) {
      gcDirectVar(&(l->args[1]),&(c->args[1]));
      storeFwd(&(l->args[1]), &(c->args[1]));
      
      LTuple * ret = tagged2LTuple(start);

      ptrStack.pushInt(n);
      ptrStack.push(ret, PTR_LTUPLE);
      
      return ret;
    }

    TaggedRef not_deref = t;
    
    DEREF(t, t_ptr, t_tag);

    if (!isLTuple(t_tag)) {
      LTuple * ret = tagged2LTuple(start);

      ptrStack.pushInt(n);

      if (isNil(t)) {
	c->args[1] = t;
	ptrStack.push(ret, PTR_LTUPLE);
      } else {
	c->args[1] = not_deref;
	ptrStack.push(ret, PTR_LTUPLE_TAIL);
      }
      return ret;
    }
    
    l = tagged2LTuple(t);

  } 
  
  Assert(0);
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

  SRecord *ret = (SRecord*) heapMalloc(len);
  GCNEWADDRMSG(ret);
  
  ret->label       = label;
  ret->recordArity = recordArity;

  Bool isInGc = (opMode != IN_TC);
  
  register TaggedRef * a = getRef();
  register TaggedRef * c = ret->getRef();

  for (int i = getWidth(); i--; ) {
    TaggedRef from = a[i];

    if (isDirectVar(from)) {
      gcDirectVar(a + i, c + i);
      storeFwd((int *) a + i, c + i);
    } else {
      c[i] = from;
    }
    
  }

  ptrStack.push(ret,PTR_SRECORD);

  storeFwd((int32*)&label, ret);

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

Thread *Thread::gcThread ()
{
  GCMETHMSG ("Thread::gcThread");

  if (this == (Thread *) NULL) return ((Thread *) NULL);
  Thread *ret = isCollected(this,item.threadBody);
  if (ret) return ret;

  if (isDeadThread()) return ((Thread *) NULL);

  //  Some invariants:
  // nothing can be copied (IN_TC) until stability;

  // first class threads: must only copy thread when local to solve!!!
  if (opMode == IN_TC && !isLocalBoard(GETBOARD(this))) {
    return this;
  }

  Assert(opMode == IN_GC || !isRunnable());

  // 
  //  Note that runnable threads can be also counted 
  // in solve actors (for stability check), and, therefore, 
  // might not just dissappear!
  if (isSuspended() && !GETBOARD(this)->gcIsAlive()) {
    return ((Thread *) NULL);
  }

  COUNT(thread);
  Thread *newThread = (Thread *) gcReallocStatic(this, sizeof(Thread));
  GCNEWADDRMSG(newThread);

  if (isRunnable() || hasStack()) {
    ThreadList::add(newThread);
  }

  ptrStack.push(newThread, PTR_THREAD);

  FDPROFILE_GC(cp_size_susp, sizeof(*this));

  storeFwd(&item.threadBody, newThread);

  return newThread;
}

inline
Abstraction *gcAbstraction(Abstraction *a)
{
  return (Abstraction *) a->gcConstTerm();
}


Thread *Thread::gcDeadThread()
{
  Assert(isDeadThread());

  COUNT(thread);
  Thread *newThread = (Thread *) gcReallocStatic(this,sizeof(Thread));
  GCNEWADDRMSG(newThread);

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
void Thread::gcRecurse ()
{
  GCMETHMSG("Thread::gcRecurse");

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
ForeignPointer * ForeignPointer::gc(void)
{
  GCMETHMSG("ForeignPointer::gc");
  ForeignPointer * ret =
    (ForeignPointer*) gcReallocStatic(this,sizeof(ForeignPointer));
  ret->ptr = ptr;
  GCNEWADDRMSG(ret);
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
    OZ_updateHeapTerm(*t->getRefHead());
  }
  for (OZ_Term l1=finalize_list;!isNil(l1);l1=tail(l1)) {
    LTuple *t = tagged2LTuple(l1);
    OZ_updateHeapTerm(*t->getRefHead());
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
void OZ_updateHeapTerm(TaggedRef &to) {
  GCPROCMSG("OZ_updateHeapTerm");
  register TaggedRef aux = to;

  Assert(opMode==IN_TC || !fromSpace->inChunkChain(&to));

  register TaggedRef * aux_ptr = NULL;

update:
  
  switch (tagTypeOf(aux)) {
    
  case REFTAG1:
    /* initalized but unused cell in register array */
    if (aux == makeTaggedNULL())
      return;

  case REFTAG2:
  case REFTAG3:
  case REFTAG4:
    
    do {
      aux_ptr = tagged2Ref(aux);
      aux     = *aux_ptr;
    } while (IsRef(aux));
  
    to = aux;
    
    goto update;

  case GCTAG:
    to = makeTaggedRef((TaggedRef*) GCUNMARK(aux));
    // mm2: this can lead to not shortened ref chains together with
    // the CONS forwarding: if a CONS cell is collected then every
    // reference to the first element becomes a ref. May try this:
    // if (!isVar(*to)) to=deref(to);
    break;
    
  case SMALLINT: 
    break;

  case FSETVALUE:
    if (opMode == IN_GC)
      to = makeTaggedFSetValue(((FSetValue *) tagged2FSetValue(aux))->gc()); 
    break;
    
  case LITERAL:  
    { 
      Literal * l = tagged2Literal(aux);

      if (needsCollection(l)) 
	to = makeTaggedLiteral(l->gc()); 
      break;
    }
    
  case LTUPLE:   
    to = makeTaggedLTuple(tagged2LTuple(aux)->gc()); 
    break;

  case SRECORD:  
    to = makeTaggedSRecord(tagged2SRecord(aux)->gcSRecord()); 
    break;

  case BIGINT: 
    if (opMode == IN_GC) 
      to = makeTaggedBigInt(tagged2BigInt(aux)->gc()); 
    break;

  case OZFLOAT:  
    if (opMode == IN_GC) 
      to = makeTaggedFloat(tagged2Float(aux)->gc());   
    break;

  case OZCONST:
    to = makeTaggedConst(tagged2Const(aux)->gcConstTerm());
    break;

  case SVAR:
    {
      SVariable * sv = tagged2SVar(aux);

      if (sv->gcNeeded()) {
	(void) sv->gc();
	updateStack.push(&to);
	to = makeTaggedRefToFromSpace(aux_ptr);
      } else {
	to = makeTaggedRef(aux_ptr);
      }
      
      break;
    }

  case CVAR:
    {
      GenCVariable * cv = tagged2CVar(aux);

      if (cv->gcNeeded()) {
	(void) cv->gc();
	updateStack.push(&to);
	to = makeTaggedRefToFromSpace(aux_ptr);
      } else {
	to = makeTaggedRef(aux_ptr);
      }

      break;
    }
    
  case UVAR: 
    {
      // put address of ref cell to be updated onto update stack    
      Assert(aux_ptr);
      
      Board * bb = tagged2VarHome(aux)->derefBoard();
      
      Assert(bb);

      if (opMode == IN_TC && !isLocalBoard(bb)) {
	to = makeTaggedRef(aux_ptr);
      } else {
	(void) bb->gcDerefedBoard();
      
	Assert(bb->gcIsAlive());

	updateStack.push(&to);
      
	to = makeTaggedRefToFromSpace(aux_ptr);
      }
      
      break;    
    }
  }
}

void OZ_collectHeapBlock(TaggedRef * frm, TaggedRef * to, int sz) {

  for (int i=sz; i--; ) {
    to[i] = frm[i];
    OZ_updateHeapTerm(to[i]);
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

  gcFrameToProxy();

  GCMETHMSG(" ********** AM::gc **********");
  opMode = IN_GC;
  gcing = 0;
  gc_is_running = OK;

  ozstat.initGcMsg(msgLevel);
  
  ptrStack.unlock();

  MemChunks *oldChain = MemChunks::list;

  VariableNamer::cleanup();  /* drop bound variables */
  cacheList->cacheListGC();  /* invalidate inline caches */

  initCheckSpace();
  initMemoryManagement();
  //  ProfileCode(ozstat.initCount());

  { /* initialize X regs; this IS necessary ! */
    for (int j=NumberOfXRegisters; j--; ) {
      xRegs[j] = 0;
    }
  }
  
//                 actual garbage collection starts here
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

// colouring root pointers grey
//-----------------------------------------------------------------------------

  Assert(updateStack.isEmpty());
  Assert(trail.isEmpty());
  Assert(cachedSelf==0);
  Assert(ozstat.currAbstr==NULL);
  Assert(shallowHeapTop==0);
  Assert(_rootBoard);

  _rootBoard = _rootBoard->gcBoard();   // must go first!
  setCurrent(_currentBoard->gcBoard(),NO);

  GCPROCMSG("Predicate table");
  CodeArea::gc();

  aritytable.gc ();
  ThreadsPool::doGC ();

#ifdef DEBUG_STABLE
  board_constraints = board_constraints->gc ();
#endif
  
  suspendVarList=makeTaggedNULL(); /* no valid data */

  OZ_updateHeapTerm(aVarUnifyHandler);
  OZ_updateHeapTerm(aVarBindHandler);

  OZ_updateHeapTerm(defaultExceptionHdl);
  OZ_updateHeapTerm(opiCompiler);
  OZ_updateHeapTerm(debugStreamTail);

  gc_tcl_sessions();

  toplevelVars = gcRefsArray(toplevelVars);

  extRefs = extRefs->gc();
  
  PROFILE_CODE1(FDProfiles.gc());

#ifdef FINALIZATION
  OZ_updateHeapTerm(finalize_handler);
  performCopying();
  gc_finalize();
#endif

  gcOwnerTable();  
  gcBorrowTable1();
  performCopying();
  gcBorrowTable2();


  performCopying();

  ptrStack.lock();

  gcGNameTable();
  performCopying();


// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to checked now
  GCPROCMSG("updating references");
  processUpdateStack();
  
  Assert(ptrStack.isEmpty());

  gcBorrowTable3();
  gcSiteTable();

  exitCheckSpace();

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  cachedStack = NULL;

  ozstat.printGcMsg(msgLevel);
  
  gc_is_running = NO;
  gcing = 1;
} // AM::gc


/*
 *   Process updateStack -
 *
 */
void processUpdateStack(void) {
  updateStack.lock();

  while (!updateStack.isEmpty()) {
    TaggedRef * to = updateStack.pop();

    Assert(isRef(*to));

    TaggedRef * aux_ptr = tagged2Ref(*to);
    TaggedRef   aux     = *aux_ptr;
    
    Assert(!isRef(aux));
    
    switch (tagTypeOf(aux)) {

    case UVAR:
      {
	Assert(gcUVarNeeded(aux));

	TaggedRef uv = gcUVar(aux);
	
	Assert(uv);

	*to = makeTaggedRef(newTaggedUVar(tagged2VarHome(uv)));
	
	COUNT(uvar);
	break;
      }
    
    case SVAR:
      { 
	SVariable *sv = tagged2SVar(aux);
	
	Assert(sv->gcNeeded());

	INFROMSPACE(sv);
  
	Assert(sv->gcIsMarked());
	  
	*to = makeTaggedRef(newTaggedSVar(sv->gcGetFwd()));
      
	COUNT(svar);
	break;
      }

    case CVAR:
      {
	GenCVariable *cv = tagged2CVar(aux);
	
	Assert(cv->gcNeeded());

	INFROMSPACE(cv);

	Assert(cv->gcIsMarked());
	
	*to = makeTaggedRef(newTaggedCVar(cv->gcGetFwd()));
    
	COUNT(cvar);
	break;
      }

    case GCTAG:
      *to = makeTaggedRef((TaggedRef *) GCUNMARK(aux));
      break;
    
    default:
      Assert(NO);
    }
    
    INFROMSPACE(aux_ptr);
    
    storeFwd((int *) aux_ptr, ToPointer(*to));
    
  } // while

  Assert(updateStack.isEmpty());
  updateStack.unlock();
}


/*
 *   AM::copyTree () routine (for search capabilities of the machine)
 *
 */

#ifdef CS_PROFILE
static Bool across_redid = NO;
#endif

Board* AM::copyTree(Board* bb, Bool *isGround)
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
  
  if (isGround == (Bool *) NULL) {
    GCMETHMSG(" ********** AM::copyTree **********");
  } else {
    GCMETHMSG(" ********** AM::copyTree (groundness) **********");
  }
  opMode = IN_TC;
  gcing = 0;

  varCopied = NO;

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

  ptrStack.unlock();

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
  // Note that parent, right&leftSibling must be set in this subtree -
  // for instance, with "setParent"

  if (isGround != (Bool *) NULL)
    *isGround = (varCopied == NO) ? OK : NO;

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
      for (int i = aux->getSize(); i--; ) {
	if (aux->table[i].key) {
	  OZ_updateHeapTerm(aux->table[i].key);
	}
      }
    }
    OZ_updateHeapTerm(aux->list);
    aux = aux->next;
  }
}

void ArityTable::gc()
{
  GCMETHMSG("ArityTable::gc");

  for (int i = size; i--; ) {
    if (table[i] != NULL) {
      (table[i])->gc();
    }
  }
}

void PrTabEntry::gcPrTabEntry()
{
  if (this == NULL) return;

  OZ_updateHeapTerm(info);
  OZ_updateHeapTerm(names);
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

void CodeArea::gc()
{
  abstractionTab.gcAbstractionTable();
}

void ThreadsPool::doGC ()
{
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

// Note: the order of the list must be maintained
inline
OrderedSuspList * OrderedSuspList::gc()
{
  GCMETHMSG("OrderedSuspList::gc");

  OrderedSuspList * ret = NULL, * help = this, ** p = & ret;

  while (help != NULL) {

    Thread * aux = help->t->gcThread();

    if (aux) {
      *p = new OrderedSuspList(aux, NULL);
      p = & (*p)->n;
    }

    help = help->n;
  }
  GCNEWADDRMSG (ret);
  return (ret);
}

void TaskStack::gc(TaskStack *newstack)
{
  COUNT(taskStack);
  // mm2 COUNT1(taskStackLen,getMaxSize());

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
      if (opMode == IN_TC)
	(void)CodeArea::livenessX(pc,Y,getRefsArraySize(Y));
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
      OZ_updateHeapTerm(*tt);
      Y = (RefsArray) ToPointer(*tt);
      G = gcRefsArray(G);
    } else if (PC == C_CFUNC_CONT_Ptr) {
      G = gcRefsArray(G);
    } else { // usual continuation
      COUNT(cCont);
      Y = gcRefsArray(Y);
      G = gcRefsArray(G);
    }

    *(--newtop) = PC;
    *(--newtop) = Y;
    *(--newtop) = G;
  } // while not task stack is empty
}


//*********************************************************************
//                           NODEs
//*********************************************************************


inline  
Bool ConstTerm::gcIsMarked(void) {
  return GCISMARKED(ctu.tagged);
}

inline
void ConstTerm::gcMark(ConstTerm * fwd) {
  ctu.tagged = GCMARK(fwd);
}

inline
void ** ConstTerm::gcGetMarkField(void) {
  return (void **) &ctu.tagged;
}

inline
ConstTerm * ConstTerm::gcGetFwd(void) {
  Assert(gcIsMarked());
  return (ConstTerm *) GCUNMARK((int) ctu.tagged);
}

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
  setVarCopied;
  switch(getType()) {
  case Co_Object:
    {
      Object *o = (Object *) this;
      o->gcEntityInfo();

      switch(o->getTertType()) {
      case Te_Local:   o->setBoard(GETBOARD(o)->gcDerefedBoardOutline()); break;
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
	cl->setBoard(GETBOARD(cl)->gcDerefedBoardOutline()); 
	OZ_updateHeapTerm(cl->val);
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
	p->setBoard(GETBOARD(p)->gcDerefedBoardOutline()); /* ATTENTION */
	PortWithStream *pws = (PortWithStream *) this;
	OZ_updateHeapTerm(pws->strm);
	break;}
      case Te_Proxy:{
	p->gcProxy();
	break;}
      case Te_Manager:{
	p->gcManager();
	PortWithStream *pws = (PortWithStream *) this;
	OZ_updateHeapTerm(pws->strm);
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
	  s->setBoard(GETBOARD(s)->gcDerefedBoardOutline());
	}
      }
      break;
    }
    
  case Co_Chunk:
    {
      SChunk *c = (SChunk *) this;
      OZ_updateHeapTerm(c->value);
      c->gcConstTermWithHome();
      break;
    }
    
  case Co_Array:
    {
      OzArray *a = (OzArray*) this;

      a->gcConstTermWithHome();

      int aw = a->getWidth();

      if (aw > 0) {
	TaggedRef *oldargs = a->getArgs();
	TaggedRef *newargs = 
	  (TaggedRef*) heapMalloc(sizeof(TaggedRef) * aw);
	for (int i=aw; i--; ) {
	  newargs[i] = oldargs[i];
	  Assert(!isDirectVar(oldargs[i]));
	  OZ_updateHeapTerm(newargs[i]);
	}
	
	a->setPtr(newargs);
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
	ll->setBoard(GETBOARD(ll)->gcDerefedBoardOutline());  /* maybe getBoardInternal() */
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

#define CheckLocal(CONST)					\
{								\
   Board *bb=GETBOARD(CONST);					\
   if (!bb->gcIsAlive()) return NULL;				\
   if (opMode == IN_TC && !isLocalBoard(bb)) return this;	\
}

inline void EntityInfo::gcWatchers(){
  Watcher **base=&watchers;
  Watcher *w=*base;
  while(w!=NULL){
    Watcher* newW=(Watcher*) gcReallocStatic(w,sizeof(Watcher));
    *base=newW;
    newW->thread=newW->thread->gcThread();
    OZ_updateHeapTerm(newW->proc);
    base= &(newW->next);
    w=*base;}}

void Tertiary::gcEntityInfo(){
  if(info==NULL) return;
  EntityInfo *newInfo = (EntityInfo *) gcReallocStatic(info,sizeof(EntityInfo));
  info=newInfo;
  info->gcWatchers();}

ConstTerm *ConstTerm::gcConstTerm()
{
  GCMETHMSG("ConstTerm::gcConstTerm");

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
      COUNT(abstraction);
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
      COUNT(cell);   
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
      COUNT(space);
      break;
    }

  case Co_Chunk:
    {
      SChunk *sc = (SChunk *) this;
      CheckLocal(sc);
      COUNT(chunk);
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

  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_CONSTTERM);
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


ConstTerm* ConstTerm::gcConstTermSpec()
{
  GCMETHMSG("ConstTerm::gcConstTerm");

  if (gcIsMarked())
    return gcGetFwd();

  Tertiary *t=(Tertiary*)this;
  Assert((t->getType()==Co_Cell) || (t->getType()==Co_Lock));
  Assert(t->getTertType()==Te_Frame);
  ConstTerm *ret;
  if(t->getType()==Co_Cell){
    CellFrame *cf=(CellFrame*)t;
    cf->setAccessBit();
    COUNT(cell);   
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(CellFrame));
    cf->myStoreForward(ret);}
  else{
    Assert(getType()==Co_Lock);
    LockFrame *lf=(LockFrame*)t;
    lf->setAccessBit();
    ret = (ConstTerm *) gcReallocStatic(this,sizeof(LockFrame));
    lf->myStoreForward(ret);}
  GCNEWADDRMSG(ret);
  ptrStack.push(ret,PTR_CONSTTERM);
  return ret;
}

HeapChunk * HeapChunk::gc(void)
{
  GCMETHMSG("HeapChunk::gc");

  COUNT(heapChunk);
  HeapChunk * ret = (HeapChunk *) gcReallocStatic(this, sizeof(HeapChunk));

  ret->chunk_data = copyChunkData();
  
  GCNEWADDRMSG(ret);
  storeFwdField(ret);
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
void Board::gcRecurse()
{
  GCMETHMSG("Board::gcRecurse");
  Assert(!isCommitted() && !isFailed());
  body.gcRecurse();
  u.actor=u.actor->gcActor();

  script.Script::gc();
}


inline  
Bool Actor::gcIsMarked(void) {
  return (gcField != 0);
}

inline
void Actor::gcMark(Actor * fwd) {
  gcField = fwd;
}

inline
void ** Actor::gcGetMarkField(void) {
  return (void **) &gcField;
}

inline
Actor * Actor::gcGetFwd(void) {
  Assert(gcIsMarked());
  return gcField;
}

Actor *Actor::gcActor()
{
  if (this==0) return 0;

  Assert(board->derefBoard()->gcIsAlive());

  GCMETHMSG("Actor::gc");

  if (gcIsMarked())
    return gcGetFwd();
  
  Actor *ret;

  if (isWait()) {
    COUNT(waitActor);
    ret = (Actor *) gcReallocStatic(this,sizeof(WaitActor));
  } else if (isAsk () == OK) {
    COUNT(askActor);
    ret = (Actor *) gcReallocStatic(this,sizeof(AskActor));
  } else {
    COUNT(solveActor);
    ret = (Actor *) gcReallocStatic(this,sizeof(SolveActor));
  }
      
  FDPROFILE_GC(isWait() ? cp_size_waitactor
	       : (isAsk() ? cp_size_askactor
		  : cp_size_solveactor), sz);
	
  GCNEWADDRMSG(ret);

  ptrStack.push(ret,PTR_ACTOR);

  storeFwdField(ret);

  return ret;
}

inline
void AWActor::gcRecurse()
{
  thread = thread->gcThread();
}

inline
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
  GCMETHMSG("AskActor::gcRecurse");
  next.gcRecurse ();
  board = board->gcBoard ();
  Assert(board);
}

inline
void SolveActor::gcRecurse () {
  GCMETHMSG("SolveActor::gcRecurse");
  if (opMode == IN_GC || solveBoard != fromCopyBoard) {
    board = board->gcBoard();
    Assert(board);
  }
  solveBoard = solveBoard->gcBoard();
  Assert(solveBoard);

  if (opMode == IN_GC || !isGround())
    OZ_updateHeapTerm(solveVar);
  
  OZ_updateHeapTerm(result);
  suspList         = suspList->gc();
  cpb              = cpb->gc();
  localThreadQueue = localThreadQueue->gc();
  nonMonoSuspList  = nonMonoSuspList->gc();
#ifdef CS_PROFILE
  if((copy_size>0) && copy_start && (opMode == IN_GC)) {
    free(copy_start);
  }
  orig_start = (int32 *) NULL;
  copy_start = (int32 *) NULL;
  copy_size  = 0;
#endif
}

inline
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

CpBag * CpBag::gc(void) {
  GCMETHMSG("CpBag::gc");
  
  CpBag *  copy = (CpBag *) 0;
  CpBag ** cur  = &copy;
  CpBag *  old  = this;
  
  while (old) {
    WaitActor * wa = old->choice;
    
    if (wa && wa->isAliveUpToSolve() && GETBOARD(wa)->gcIsAlive()) {
      Assert(!(opMode == IN_TC && !isInTree(GETBOARD(wa))));
      
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
  GCMETHMSG("SRecord::gcRecurse");
  OZ_updateHeapTerm(label);

  TaggedRef * a = getRef();
  
  for (int i = getWidth(); i--; ) {
    if (!isDirectVar(a[i]))
      OZ_updateHeapTerm(a[i]);
  }
}


inline
void LTuple::gcRecurse(int n, Bool tail) {
  GCMETHMSG("LTuple::gcRecurse");
  
  LTuple * l = this;

  while (n > 1) {
    if (!isDirectVar(l->args[0]))
      OZ_updateHeapTerm(l->args[0]);
    l = tagged2LTuple(l->args[1]);
    n--;
  }

  if (!isDirectVar(l->args[0]))
    OZ_updateHeapTerm(l->args[0]);

  if (tail) {
    Assert(!isDirectVar(l->args[1]));
    OZ_updateHeapTerm(l->args[1]);
  }

}



void performCopying(void)
{
  while (!ptrStack.isEmpty()) {
    TypedPtr tptr     = ptrStack.pop();  
    void *ptr         = getPtr(tptr);
    TypeOfPtr ptrType = getType(tptr);
    
    switch(ptrType) {
      
    case PTR_LTUPLE:    
      ((LTuple *) ptr)->gcRecurse(ptrStack.popInt(), NO);
      break;
    case PTR_LTUPLE_TAIL:    
      ((LTuple *) ptr)->gcRecurse(ptrStack.popInt(), OK);
      break;
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

OzDebug *OzDebug::gcOzDebug()
{
  OzDebug *ret = (OzDebug*) gcReallocStatic(this,sizeof(OzDebug));

  ret->Y = gcRefsArray(ret->Y);
  ret->G = gcRefsArray(ret->G);
  OZ_updateHeapTerm(ret->data);
  ret->arguments = gcRefsArray(ret->arguments);

  return ret;
}

// special purpose to gc borrowtable entry which is a variable
TaggedRef gcTagged1(TaggedRef in) {
  TaggedRef x=deref(in);
  Assert(GCISMARKED(x));
  return makeTaggedRef((TaggedRef*)GCUNMARK(x));
}







