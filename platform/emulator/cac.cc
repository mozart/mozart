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
 *    Kevin Glynn (glynn@info.ucl.ac.be)
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
#include "var_failed.hh"
#include "var_readonly.hh"
#include "var_simple.hh"
#include "var_opt.hh"
#include "var_ext.hh"
#include "thr_int.hh"
#include "debug.hh"
#include "pointer-marks.hh"
#include "dpInterface.hh"
#include "gname.hh"
#include "interFault.hh"
#include "weakdict.hh"
#include "mozart_cpi.hh"

#if defined(DEBUG_LIVECALC)
// Whenever liveness zeroes the contents of a register we actually
// place a fresh int (maintained by deadMarker) there. This gives some
// debugging info when tracking down buggy liveness code ...
static int deadMarker = 300000;
   #define VOID_GREG(to,i) to->initG(i,makeTaggedSmallInt(deadMarker++))
#else
   #define VOID_GREG(to,i) to->initG(i,makeTaggedNULL())
#endif
#if defined(DEBUG_PRINTLIVECALC)
static int deadMarker = makeTaggedNULL();
#endif

#ifdef OUTLINE
#define inline
#endif


/* We allocate a static usage block for G usage vectors.  This
** saves a lot of malloc/free and a lot of time for some benchmarks
*/
#define StaticGUsageVectorSize 100

static int gUsageVector[StaticGUsageVectorSize];

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

#define _cacPendThreadEmul       gCollectPendThreadEmul

#define _cacSuspList             gCollectSuspList
#define _cacLocalSuspList        gCollectLocalSuspList

#define _cacDictEntry           gCollectDictEntry

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

#define _cacPendThreadEmul       sClonePendThreadEmul

#define _cacSuspList             sCloneSuspList
#define _cacLocalSuspList        sCloneLocalSuspList

#define _cacDictEntry           sCloneDictEntry

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
#define CAC_MALLOC(sz) oz_heapMalloc((sz))
#else
#define CAC_MALLOC(sz) oz_freeListMalloc((sz))
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
  case 48:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];_t[3]=_f[3];_t[4]=_f[4];_t[5]=_f[5]; \
    _t[6]=_f[6];_t[7]=_f[7];_t[8]=_f[8];_t[9]=_f[9];_t[10]=_f[10];           \
    _t[11]=_f[11];                                                           \
    break;                                                                   \
  case 36:                                                                   \
    _t[0]=_f[0];_t[1]=_f[1];_t[2]=_f[2];_t[3]=_f[3];_t[4]=_f[4];_t[5]=_f[5]; \
    _t[6]=_f[6];_t[7]=_f[7];_t[8]=_f[8];                                     \
    break;                                                                   \
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
#define STOREFWDNOMARK(fromPtr, newValue)               \
  CPTRAIL(fromPtr);                                     \
  *((int32 *) fromPtr) = ToInt32(newValue);

#define STOREFWDMARK(fromPtr, newValue)                 \
  Assert(isSTAligned(newValue));                        \
  CPTRAIL(fromPtr);                                     \
  *((int32 *) fromPtr) = makeTaggedMarkPtr(newValue);

#define STOREPSEUDOFWDMARK(fromPtr, newValue)           \
  Assert(!isSTAligned(newValue));                       \
  CPTRAIL(fromPtr);                                     \
  *((int32 *) fromPtr) = makeTaggedRef(newValue);

#define STOREFWDFIELD(d,t)                              \
  CPTRAIL(d->cacGetMarkField());                        \
  d->cacMark(t);



/*
 * RefsArrays
 *
 */

inline
RefsArray * RefsArray::_cac(void) {
  if (!this)
    return this;
  if (cacIsMarked())
    return cacGetFwd();

  int l = getLen();

  RefsArray * t = RefsArray::allocate(l,NO);

  OZ_cacBlock(getArgsRef(), t->getArgsRef(), l);

  STOREFWDFIELD(this,t);

  return t;
}


/*
 *   Collection of terms
 *
 */

#ifdef G_COLLECT

#define NEEDSCOPYING(bb) (OK)

#define IsToSpaceBoard(b) (b->isEqGCStep(oz_getGCStep()))

#else

#define NEEDSCOPYING(bb) (!(bb)->hasMark())

#define IsToSpaceBoard(b) (b->getCopyStep() == oz_getCopyStep())

#endif


#ifdef G_COLLECT
/*
 * Abstraction
 * Argument vector describes liveness of G registers. If
 * it is NULL then all registers are live.
 */
Abstraction * Abstraction::gCollect(int gUsageLen, int * gUsage) {

  Assert(this);
  if (cacIsMarked())
    return (Abstraction *) cacGetFwd();

  Abstraction *to;
  TaggedRef *gReg;
  int gFinished = 1;

  if (cacIsCopied()) {
    // Repeat customer, update and gc any newly live fields in the copy.
    to = cacGetCopy();
    gReg = to->getGRef();
    if (!gUsage) {
      // All previously dead registers are live
      for (int i=to->pred->getGSize(); i--;) {
        // Only gc if this slot was previously void
        // to avoid nasty cycles.
#if defined(DEBUG_LIVECALC)
        if ( oz_isSmallInt(to->getG(i)) )
#else
        if ( to->getG(i)==makeTaggedNULL() )
#endif
        {
          to->initG(i,getG(i));
          oz_cacTerm(gReg[i], gReg[i]);
        }
      }
    }
    else {
      for (int i=to->pred->getGSize(); i--;) {
#if defined(DEBUG_LIVECALC)
        if ( oz_isSmallInt(to->getG(i)) )
#else
        if ( to->getG(i)==makeTaggedNULL() )
#endif
        { // Was dead
          if (gUsage[i]) {
            // Live. Only gc if this slot was previously void
            // to avoid nasty cycles.
            to->initG(i,getG(i));
            oz_cacTerm(gReg[i], gReg[i]);
          }
          else
            gFinished = 0;
        }
      }
    }
    if (gFinished) {
      // Can skip this abstraction in future
      STOREFWDFIELD(this, to);
    }
    return to;
  }
  // First time seen
  to = (Abstraction *) oz_hrealloc(this, getAllocSize());
  cacCopy(to);

  gReg = to->getGRef();
  int gSize= to->pred->getGSize();

  // Void dead registers
  if (gUsage) {
    // *IMPORTANT*: must zero all dead regs before we do *any* gc
    for (int i=gSize; i--; ) {
      if (!(gUsage[i])) {
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
        printf("Abstraction <%s> G[%d] => %d\n", to->getPrintName(), i, deadMarker);
#endif
        VOID_GREG(to,i);
        // Must still consider other paths to this abstraction
        gFinished = 0;
      }
    }
  }

  if (gFinished) {
    // Can skip this abstraction in future
    STOREFWDFIELD(this, to);
  }

  if (to->hasGName())
    gCollectGName(to->getGName1());
  else
    to->setBoard(to->getSubBoardInternal()->gCollectBoard());

  // We must be careful to only collect the non-voided entries.
  // Originally I fell through to the OZ_cacBlock call but that
  // caused a (hard to track down) bug whereby a register which is currently
  // void could already have a collected entry in it by the time we get to it
  // If gc'ing one of the registers causes this closure to be gc'd again the
  // code in the cacIsCopied() branch ensures it doesn't touch
  // any of the registers we are gc'ing here.
  if (!gFinished) {
    for (int i=gSize; i--; ) {
      if (gUsage[i]) {
        oz_cacTerm(gReg[i], gReg[i]);
      }
    }
  }
  else // All registers still live.
    OZ_cacBlock(gReg,gReg,gSize);

  //cause the rest of the closure to be gc'd.
  cacStack.push(to, PTR_CONSTTERM);
  return to;
}

#endif /* G_COLLECT */

/*
 * Boards:
 *
 */

inline
Board * Board::_cacBoard(void) {
  GCDBG_INFROMSPACE(this);

  // Do not clone a space above or collect a space above root ;-(
  Assert(this && !hasMark());

  return cacIsMarked() ? cacGetFwd() : _cacBoardDo();
}

Board * Board::_cacBoardDo(void) {
  Board * bb = derefBoard();

  Assert(bb->cacIsAlive());

  if (bb->cacIsMarked())
    return bb->cacGetFwd();

  Board * ret;
  cacReallocStatic(Board,bb,ret,sizeof(Board));

  // kost@ : the OptVar template has to be already there since it is
  // needed when collecting OptVar"s;
  ret->optVar = makeTaggedVar(new OptVar(ret));
#ifdef G_COLLECT
  ret->nextGCStep();
  // an alive board must be copied at every GC step exactly once:
  Assert(ret->isEqGCStep(oz_getGCStep()));
#endif
  ret->setCopyStep(oz_getCopyStep());

  cacStack.push(ret, PTR_BOARD);

  STOREFWDFIELD(bb, ret);

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

  if (
#ifdef G_COLLECT
      isOnHeap()
#else
      !getBoardInternal()->hasMark()
#endif
      ) {
    Name * aux = (Name *) memcpy(oz_heapDoubleMalloc(sizeof(Name)),
                                 this, sizeof(Name));

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
  DynamicTable * to = (DynamicTable *)
    oz_heapMalloc((size-1)*sizeof(HashElement) + sizeof(DynamicTable));
  to->numelem = numelem;
  to->size    = size;

  OZ_cacBlock((TaggedRef *) table, (TaggedRef *) to->table, 2 * size);

  return to;
}

//
// OzDictionary"s tables;
//

// replicated from dictionary.cc;
static const double GDT_MAXENTRIES      = 0.9;
static const double GDT_IDEALENTRIES    = 0.7;
static const int GDT_MINFULL            = 4;   //

// This one does not change the 'entries' counter (nor calls
// 'resize()', of course);
void DictHashTable::_cacDictEntry(DictNode *n)
{
  DictNode *np = &table[hash(featureHash(n->getKey()))];

  //
  if (np->isEmpty()) {
    (void) new (np) DictNode(*n);
    OZ_cacBlock((TaggedRef *) n, (TaggedRef *) np, 2);
    return;

    //
  } else {
    if (!np->isPointer()) {
      Assert(!featureEq(np->getKey(), n->getKey())); // unique keys;
      // a fresh collision - allocate a new block of DictNode"s;
      DictNode *newA =
        (DictNode *) CAC_MALLOC(2 * sizeof(DictNode));

      // .. this one must be already GC"ed;
      (void) new (newA) DictNode(*np);
      np->setSPtr(newA++);
      (void) new (newA) DictNode(*n);
      OZ_cacBlock((TaggedRef *) n, (TaggedRef *) newA, 2);
      //
      newA++;
      np->setEPtr(newA);
      return;

      //
    } else {
      DictNode *sptr = np->getDictNodeSPtr();
      DictNode *eptr = np->getDictNodeEPtr();
      int bytes = ((char *) eptr) - ((char *) sptr) + sizeof(DictNode);
      DictNode *newA = (DictNode *) CAC_MALLOC(bytes);

      //
      np->setSPtr(newA);
      do {
        (void) new (newA++) DictNode(*sptr++);
      } while (sptr < eptr);
      (void) new (newA) DictNode(*n);
      OZ_cacBlock((TaggedRef *) n, (TaggedRef *) newA, 2);
      //
      newA++;
      np->setEPtr(newA);
      return;
    }
  }
  Assert(0);
}

inline
DictHashTable* DictHashTable::_cac(void)
{
  int tableSize = dictHTSizes[sizeIndex];
  DictNode *an;

  //
  Assert(entries >= 0);
  if (entries >= (tableSize / GDT_MINFULL)) {
    // no compactification - reconstruct it isomorphically;
    an = (DictNode *) CAC_MALLOC(tableSize * sizeof(DictNode));
    DebugCode(int eS = 0;);
    DebugCode(int oS = 0;);
    DebugCode(int pS = 0;);

    //
    for (int i = tableSize; i--; ) {
      DictNode *n = &table[i];
      if (!n->isEmpty()) {
        if (!n->isPointer()) {
          DebugCode(oS++);
          DictNode *nn = &an[i];
          (void) new (nn) DictNode(*n);
          OZ_cacBlock((TaggedRef *) n, (TaggedRef *) nn, 2);
        } else {
          DebugCode(pS++);
          DictNode *sptr = n->getDictNodeSPtr();
          DictNode *eptr = n->getDictNodeEPtr();
          int bytes = ((char *) eptr) - ((char *) sptr);
          DictNode *newA =
            (DictNode *) memcpy(CAC_MALLOC(bytes), sptr, bytes);
          int size = bytes / sizeof(DictNode);
          OZ_cacBlock((TaggedRef *) sptr, (TaggedRef *) newA, 2 * size);
          DictNode *nn = &an[i];
          nn->setSPtr(newA);
          nn->setEPtr((DictNode *) (((char *) newA) + bytes));
        }
      } else {
        (void) new (&an[i]) DictNode; // empty;
        DebugCode(eS++);
      }
    }

    //
#if defined(DEBUG_CHECK)
//      if (tableSize >= 10000) {
//        fprintf(stdout,
//            "=(pid=%d)tabSize=%d, entries=%d, empty=%d, ord=%d, pts=%d\n",
//            osgetpid(), tableSize, entries, eS, oS, pS);
//      }
#endif
    DictHashTable *dht = new DictHashTable(*this);
    dht->table = an;
    return (dht);

    //
  } else {
    // construct anew, GC"ing keys/values along;
    //
    const int tableSize = dictHTSizes[sizeIndex];

    // can be zero too:
    int newTableSize = (int) ((double) entries / GDT_IDEALENTRIES);
    int newSizeIndex = sizeIndex - 1;
    DictNode* old = table;

    //
    Assert(newTableSize < tableSize);
    while (newSizeIndex >= 0 && dictHTSizes[newSizeIndex] >= newTableSize)
      newSizeIndex--;
    Assert(newSizeIndex < 0 || dictHTSizes[newSizeIndex] < newTableSize);
    newSizeIndex++;
    Assert(newSizeIndex >= 0 && dictHTSizes[newSizeIndex] >= newTableSize);
    // Must not oscillate:
    Assert(dictHTSizes[newSizeIndex] < tableSize);
    // Next GC should not attempt compactification:
    Assert(entries >= (dictHTSizes[newSizeIndex] / GDT_MINFULL));

    // construct the table anew (keep the 'entries' counter);
    newTableSize = dictHTSizes[newSizeIndex];

    //
    DictHashTable *dht = new DictHashTable(*this);
    // 'entries' copied;
    an = (DictNode *) oz_heapMalloc(newTableSize * sizeof(DictNode));
    for (int i = newTableSize; i--; )
      (void) new (&an[i]) DictNode;
    dht->table = an;
    dht->sizeIndex = newSizeIndex;
    dht->maxEntries = (int) (GDT_MAXENTRIES * newTableSize);

    //
    for (int i = tableSize; i--; old++) {
      if (!old->isEmpty()) {
        if (!old->isPointer()) {
          dht->_cacDictEntry(old);
        } else {
          DictNode *sptr = old->getDictNodeSPtr();
          DictNode *eptr = old->getDictNodeEPtr();
          do {
            dht->_cacDictEntry(sptr++);
          } while (sptr < eptr);
        }
      }
    }

    //
    return (dht);
  }
  Assert(0);
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
    oz_heapMalloc(sizeof(SuspList *) * noOfSuspLists);
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
  case OZ_VAR_OPT:
    Assert(0);
  case OZ_VAR_SIMPLE_QUIET:
  case OZ_VAR_SIMPLE:
    cacReallocStatic(OzVariable,this,to,sizeof(SimpleVar));
    break;
  case OZ_VAR_EXT:
    to = extVar2Var(var2ExtVar(this)->_cacV());
    memcpy(to,this,sizeof(OzVariable));
    GCDBG_INTOSPACE(to);
    cacStack.push(to, PTR_VAR);
    break;
  case OZ_VAR_READONLY_QUIET:
  case OZ_VAR_READONLY:
    cacReallocStatic(OzVariable,this,to,sizeof(ReadOnly));
    break;
  case OZ_VAR_FAILED:
    cacReallocStatic(OzVariable,this,to,sizeof(Failed));
    cacStack.push(to, PTR_VAR);
    break;
  case OZ_VAR_BOOL:
    cacReallocStatic(OzVariable,this,to,sizeof(OzBoolVariable));
    break;
  case OZ_VAR_OF:
    cacReallocStatic(OzVariable,this,to,sizeof(OzOFVariable));
    cacStack.push(to, PTR_VAR);
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
    cacStack.push(to, PTR_VAR);
    break;
  }

  to->setHome(bb);
  cacStack.pushSuspList(&(to->suspList));

  return (to);
}


inline
void OzOFVariable::_cacRecurse(void) {
  oz_cacTerm(label,label);
  // Update the pointer in the copied block:
  dynamictable=dynamictable->_cac();
}

inline
void Failed::_cacRecurse(void) {
  oz_cacTerm(exception,exception);
}

inline
void OzVariable::_cacVarRecurse(void) {

  switch (getType()) {
  case OZ_VAR_FAILED:
    ((Failed *)      this)->_cacRecurse();
    break;
  case OZ_VAR_OF:
    ((OzOFVariable*) this)->_cacRecurse();
    break;
  case OZ_VAR_CT:
    ((OzCtVariable*) this)->_cacRecurse();
    break;
  case OZ_VAR_EXT:
    var2ExtVar(this)->_cacRecurseV();
    break;
  default:
    Assert(0);
  }

}


#ifdef G_COLLECT

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
ConstFSetValue * ConstFSetValue::gCollect(void) {
  return new ConstFSetValue(((FSetValue *) _fsv)->gCollect());
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

  // Do not record the forward pointer! Recurse takes care of this!
  // kost@ : observe that the forward pointer here handles *both* the
  // tuple itself and its 'car' field, which can be e.g. a variable!
  cacMark(to);

  cacStack.push(this, PTR_LTUPLE);

  return to;
}

inline
SRecord *SRecord::_cacSRecord(void)
{
  Assert(this);

  GCDBG_INFROMSPACE(this);

  if (cacIsMarked())
    return cacGetFwd();

  SRecord * to =
    (SRecord*) CAC_MALLOC((getWidth()-1)*sizeof(TaggedRef)+sizeof(SRecord));

  to->recordArity = recordArity;

  STOREFWDFIELD(this, to);

  cacStack.push(this, PTR_SRECORD);

  return to;
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
    DEREF(fut,ptr);
    oz_bindReadOnly(ptr,val);
  }
}

//
Bool isGCMarkedTerm(OZ_Term t)
{
 redo:
  switch (tagged2ltag(t)) {
  case LTAG_REF00:
  case LTAG_REF01:
  case LTAG_REF10:
  case LTAG_REF11:
    {
      TaggedRef * ptr;
      do {
        ptr = tagged2Ref(t);
        t   = *ptr;
      } while (oz_isRef(t));
      goto redo;
    }
  case LTAG_LITERAL:
    {
      Literal * lit = tagged2Literal(t);
      if (lit->isAtom())
        return 1;
      else
        return ((Name*)lit)->cacIsMarked();
    }
  case LTAG_CONST0:
  case LTAG_CONST1:
    return (tagged2Const(t)->cacIsMarked());
  case LTAG_VAR0:
  case LTAG_VAR1:
    {
      OzVariable *cv = tagged2Var(t);
      if (cv->getType() == OZ_VAR_OPT)
        return (IsToSpaceBoard(cv->getBoardInternal()));
      else
        return (cv->cacIsMarked());
    }

  case LTAG_MARK0:
  case LTAG_MARK1:
    return OK;
  case LTAG_SRECORD0:
  case LTAG_SRECORD1:
  case LTAG_LTUPLE0:
  case LTAG_LTUPLE1:
  case LTAG_SMALLINT:
    break;
  }
  return NO;
}

void WeakDictionary::gCollectRecurseV(void) {
  oz_cacTerm(stream, stream);
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
    if (t!=0 && !isGCMarkedTerm(t)) {
      numelem--;
      if (stream) {
        if (!list) newstream=list=oz_newReadOnly(oz_rootBoard());
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
void VarFixStack::_cacFix(void)
{
  if (isEmpty())
    return;

  do {
    StackEntry e;
    pop1(e);
    TaggedRef * to = (TaggedRef *) e;

    Assert(oz_isRef(*to));

    TaggedRef *aux_ptr = tagged2Ref(*to);
    TaggedRef  aux     = *aux_ptr;
    TaggedRef *to_ptr;

    if (oz_isVar(aux)) {
      // not yet collected.
      OzVariable *ov = tagged2Var(aux);
      Assert(ov->getType() == OZ_VAR_OPT);
      Board *bb = ov->getBoardInternal()->derefBoard()->cacGetFwd();
      to_ptr = newTaggedOptVar(bb->getOptVar());

      // Now, 'to_ptr' is either double-word aligned (isSTAligned()),
      // or it is not.  Depending on that, either a usual "GC forward"
      // is stored, or the REF2 is used;
      if (isSTAligned(to_ptr)) {
        STOREFWDMARK(aux_ptr, to_ptr);
      } else {
        STOREPSEUDOFWDMARK(aux_ptr, to_ptr);
      }
    } else {
      // already there (either due to another "var fix" entry, or was
      // reached directly);
      if (oz_isMark(aux)) {
        to_ptr = (TaggedRef *) tagged2UnmarkedPtr(aux);
      } else {
        Assert(oz_isRef(aux));
        to_ptr = tagged2Ref(aux);
      }
    }

    //
    *to = makeTaggedRef(to_ptr);
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
#ifdef G_COLLECT
      GName * gn = cl->getGName1();
      if (gn)
        gCollectGName(gn);
#endif
      OZ_cacBlock(&(cl->features), &(cl->features), 4);
      break;
    }

  case Co_Abstraction:
    {
      Abstraction *a = (Abstraction *) this;
#ifdef G_COLLECT
      gCollectCode(a->getPred()->getCodeBlock());
      GName * gn = a->getGName1();
      if (gn)
        gCollectGName(gn);
#endif
#ifdef S_CLONE
            OZ_cacBlock(a->getGRef(),a->getGRef(),
                  a->getPred()->getGSize());
#endif
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
#ifdef G_COLLECT
      GName * gn = a->getGName1();
      if (gn)
        gCollectGName(gn);
#endif
      int aw = a->getWidth();
      if (aw > 0) {
        TaggedRef * newargs =
          (TaggedRef*) oz_heapMalloc(sizeof(TaggedRef) * aw);
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
ConstTerm * ConstTerm::gCollectConstTermInline(void) {
  Assert(this);

  if (cacIsMarked())
    return (ConstTerm *) cacGetFwd();

  int sz;

  switch (getType()) {

    /*
     * Unsituated types
     *
     */

  case Co_Extension: {
    OZ_Extension * ex = const2Extension(this);
    Assert(ex);
    GCDBG_INFROMSPACE(ex);

    Board * bb = (Board *) ex->__getSpaceInternal();

    OZ_Extension * ret = ex->gCollectV();
    Assert(extension2Const(ret)->getType() == Co_Extension);
    GCDBG_INTOSPACE(ret);

    if (bb) {
      Assert(bb->cacIsAlive());
      ret->__setSpaceInternal(bb->gCollectBoard());
    }

    ConstTerm* cret = extension2Const(ret);
    cacStack.push(cret,PTR_EXTENSION);
    STOREFWDFIELD(this,(OZ_Container*)cret);
    return cret;
  }

  case Co_Float: {
    ConstTerm * ret = new Float(((Float *) this)->getValue());
    STOREFWDFIELD(this, ret);
    return ret;
  }

  case Co_BigInt: {
    ConstTerm * ret = ((BigInt *) this)->gCollect();
    STOREFWDFIELD(this, ret);
    return ret;
  }

  case Co_FSetValue: {
      ConstTerm * ret = ((ConstFSetValue *) this)->gCollect();
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

  case Co_Abstraction: {
    Abstraction * ret = ((Abstraction *) this)->gCollect();
    return ret;
  }

    /*
     * ConstTermWithHome
     *
     */

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

//
ConstTerm *ConstTerm::gCollectConstTerm(void)
{
  return (gCollectConstTermInline());
}

#else

inline
ConstTerm *ConstTerm::sCloneConstTermInline(void) {
  Assert(this);

  if (cacIsMarked())
    return (ConstTerm *) cacGetFwd();

  int sz;

  switch (getType()) {

    /*
     * Unsituated types
     *
     */

  case Co_Extension: {
    // This is in fact situated
    OZ_Extension * ex = const2Extension(this);
    Assert(ex);
    Board * bb = (Board *) ex->__getSpaceInternal();

    if (bb) {
      Assert(bb->cacIsAlive());
      if (!NEEDSCOPYING(bb))
        return this;
    }

    OZ_Extension * ret = ex->sCloneV();

    if (bb) {
      ret->__setSpaceInternal(bb->sCloneBoard());
    }

    ConstTerm* cret = extension2Const(ret);
    cacStack.push(cret,PTR_EXTENSION);
    STOREFWDFIELD(this, (OZ_Container *) cret);
    return cret;
  }

  case Co_Float:
  case Co_BigInt:
  case Co_FSetValue:
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

  ret->Y = Y->gCollect();

  OZ_gCollectBlock(&(ret->CAP),&(ret->CAP),2);

  if (ret->arity > 0) {
    ret->arguments = (TaggedRef *)
      oz_heapMalloc(ret->arity * sizeof(TaggedRef));

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
    ProgramCounter   PC  = (ProgramCounter) *(--newtop);
    RefsArray     ** Y   = (RefsArray **)     --newtop;
    void          ** CAP = (void **)          --newtop;

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
      (void) CodeArea::livenessX(pc,*Y);
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
      printf("Remapped XCONT's X 0x%x --> ", *Y);
#endif
      *Y = (*Y)->_cac();
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
      printf("0x%x\n", *Y);
#endif
    } else if (PC == C_LOCK_Ptr) {
      TaggedRef ct = makeTaggedConst((ConstTerm *) *CAP);
      oz_cacTerm(ct, ct);
      *CAP = tagged2Const(ct);
    } else if (PC == C_SET_SELF_Ptr) {
      ConstTerm *ct = (ConstTerm *) *CAP;
      if (ct) {
        TaggedRef ctt = makeTaggedConst(ct);
        oz_cacTerm(ctt, ctt);
        *CAP = tagged2Const(ctt);
      }
    } else if (PC == C_SET_ABSTR_Ptr) {
      ;
    } else if (PC == C_DEBUG_CONT_Ptr) {
#ifdef G_COLLECT
      *Y = (RefsArray *) ((OzDebug *) *Y)->gCollectOzDebug();
#endif
      Literal *l = (Literal *) *CAP;
      if (l) {
        TaggedRef lt = makeTaggedLiteral(l);
        oz_cacTerm(lt, lt);
        *CAP = tagged2Literal(lt);
      }
    } else if (PC == C_CALL_CONT_Ptr) {
      oz_cacTerm(*((TaggedRef *) Y), *((TaggedRef *) Y));
      *CAP = ((RefsArray *) *CAP)->_cac();
    } else { // usual continuation
#ifdef G_COLLECT
      // Void dead G and Y registers if possible
      int gLen = ((Abstraction *) *CAP)->cacGetPred()->getGSize();
      int *gUsage = gUsageVector;
      if (gLen > StaticGUsageVectorSize) {
        // Static block not big enough
        gUsage = new int[gLen];
      }
      // Initialise G registers to not used
      for (int i=gLen; i--;) gUsage[i]=0;

      // If this frame is below a catch or lock frame then we will
      // have already dealt with it in its real parent. We detect this
      // by seeing if the Y registers have already been collected.
      // Note we must still collect the Y and CAP refs so that they get
      // updated to the new copy.
      if (!((*Y) && (*Y)->cacIsMarked())) {

        int yLen = (*Y)?(*Y)->getLen():0;
        // If either Y or G registers are in scope then check their liveness
        if (gLen || yLen) {
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
          printf("\nG-Collect pid(%d) abstr(0x%x) -> 0x%x\n", (int) getpid(), *CAP, PC);
          //      CodeArea::display(PC,50,stderr,NOCODE);
          //printf("\n");
          //ProgramCounter start=CodeArea::printDef(PC,stderr);
          //if (start != NOCODE) CodeArea::display(start,150,stderr,NOCODE);
#endif
          // Void Y registers, get liveness info for G registers
          CodeArea::livenessGY((ProgramCounter) PC, newtop,
                               yLen, (*Y),
                               gLen, gUsage);

        }
      }
      *CAP = ((Abstraction *) *CAP)->_cac(gLen, gUsage);
      if (gLen > StaticGUsageVectorSize) {
        delete [] gUsage;
      }
#else
      // Clone
      TaggedRef ct = makeTaggedConst((ConstTerm *) *CAP);
      oz_cacTerm(ct, ct);
      *CAP = tagged2Const(ct);
#endif
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
      printf("Remapped continuation's Y 0x%x --> ", *Y);
#endif
      *Y   = (*Y)->_cac();
#if defined(DEBUG_LIVECALC) || defined(DEBUG_PRINTLIVECALC)
      printf("0x%x\n", *Y);
#endif
    }
  }
}

inline
void Thread::_cacRecurse(Thread * fr) {
  taskStack = fr->taskStack->_cac();
  abstr     = fr->abstr;
  if (abstr)
    abstr->_cac();
  id        = fr->id;
}

inline
void Propagator::_cacRecurse(Propagator * fr) {
  _p = (OZ_Propagator *) oz_hrealloc(fr->_p, fr->_p->sizeOf());
  _p->_cac();

}

inline
Suspendable * Suspendable::_cacSuspendableInline(Bool compress) {
  Assert(this);

  if (isCacMarked()) {
    Suspendable * t = cacGetFwd();
    if (compress) {
      if (t->isMultiMark()) {
        return NULL;
      } else {
        t->setMultiMark();
        return t;
      }
    } else {
      return t;
    }
  }

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

  if (compress) {
    to->flags = flags|SF_MultiMark;
  } else {
    to->flags = flags;
  }

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
#ifdef S_CLONE
  const Bool compress = NO;
#else
  const Bool compress = OK;
#endif
  SuspList * sl = this;
  SuspList * pl = SuspList::_gc_sentinel;

  /*
   * Stage 1: collect
   *
   */
  while (sl) {
    Suspendable * to = sl->getSuspendable()->_cacSuspendableInline(compress);

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

  /*
   * Stage 2: reset marks
   *
   */

  if (compress) {
    for (sl = SuspList::_gc_sentinel->getNext(); sl; sl = sl->getNext()) {
      Assert(sl->getSuspendable()->isMultiMark());
      sl->getSuspendable()->unsetMultiMark();
    }
  }

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

void SuspStack::_cac(void) {
  if (isEmpty())
    return;
  cacStack.pushSuspList(&_head);
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
void Board::_cacRecurse() {
  Assert(!isCommitted() && !isFailed());

  // Do not recurse over root board (be it the global one or
  // the root board for cloning!)
  if (!isRoot() && !getParentInternal()->hasMark())
    parent = getParentInternal()->_cacBoard();

  lpq._cac();

  OZ_cacBlock(&script,&script,3);

  cacStack.pushSuspList(&suspList);
  Distributor * d = getDistributor();
  if (d) {
    setDistributor(d->_cac());
  }
  cacStack.pushSuspList((SuspList **) &nonMonoSuspList);

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
  Assert(!oz_isRef(aux));
  if (!oz_isLTupleOrRef(aux) || tagged2LTuple(aux) != this) {
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

    Assert(!oz_isRef(t));
    if (!oz_isLTupleOrRef(t)) {
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


#define UTG_PTR(ptr,tag,type) ((type) (((unsigned int) (ptr)) - (tag)))

void CacStack::_cacRecurse(void) {

  while (!isEmpty()) {
    StackEntry tp;
    pop1(tp);
    ptrtag_t ptag = (ptrtag_t) (((unsigned int) tp) & PTR_MASK);

    switch (ptag) {
    case PTR_LTUPLE:
      UTG_PTR(tp,PTR_LTUPLE,LTuple *)->_cacRecurse();
      break;
    case PTR_SRECORD:
      UTG_PTR(tp,PTR_SRECORD,SRecord *)->_cacRecurse();
      break;
    case PTR_BOARD:
      UTG_PTR(tp,PTR_BOARD,Board *)->_cacRecurse();
      break;
    case PTR_VAR:
      UTG_PTR(tp,PTR_VAR,OzVariable *)->_cacVarRecurse();
      break;
    case PTR_CONSTTERM:
      UTG_PTR(tp,PTR_CONSTTERM,ConstTerm *)->_cacConstRecurse();
      break;
    case PTR_EXTENSION:
      const2Extension(UTG_PTR(tp,PTR_EXTENSION,ConstTerm *))->_cacRecurseV();
      break;
    case PTR_SUSPLIST0:
    case PTR_SUSPLIST1:
      {
        SuspList ** slp = UTG_PTR(tp,PTR_SUSPLIST,SuspList **);
        StackEntry e;
        pop1(e);

        if (e) {
          unsigned int n = ((unsigned int) e) & PTR_MASK;
          Board * bb     = (Board *) (((unsigned int) e) - n);

          for (int i = n; i--; )
            slp[i] = slp[i]->_cacLocalRecurse(bb);
        } else {
          *slp = (*slp)->_cacRecurse(NULL);
        }
        break;
      }
    }
  }
}


/*
 * Entry points into garbage collection
 *
 */


void OZ_cacBlock(OZ_Term * frm, OZ_Term * to, int sz)
{
  register TaggedRef * f = frm - 1;
  register TaggedRef * t = to - 1;

  TaggedRef aux, *aux_ptr;

  while (sz > 0) {
    sz--; f++; t++;
    aux = *f;

    switch (tagged2ltag(aux)) {
    case LTAG_REF00:
      if (aux)
        goto DO_DEREF;
      *t = makeTaggedNULL();
      continue;
    case LTAG_REF01:    goto DO_DEREF;
    case LTAG_REF10:    goto DO_DEREF;
    case LTAG_REF11:    goto DO_DEREF;
    case LTAG_MARK0:    goto DO_MARK;
    case LTAG_MARK1:    goto DO_MARK;
    case LTAG_SMALLINT: goto DO_SMALLINT;
    case LTAG_LITERAL:  goto DO_LITERAL;
    case LTAG_LTUPLE0:  goto DO_LTUPLE;
    case LTAG_LTUPLE1:  goto DO_LTUPLE;
    case LTAG_SRECORD0: goto DO_SRECORD;
    case LTAG_SRECORD1: goto DO_SRECORD;
    case LTAG_CONST0:   goto DO_CONST;
    case LTAG_CONST1:   goto DO_CONST;
    case LTAG_VAR0:     goto DO_D_VAR;
    case LTAG_VAR1:     goto DO_D_VAR;
    }

  DO_DEREF:
    aux_ptr = tagged2Ref(aux);
    aux     = *aux_ptr;

    switch (tagged2ltag(aux)) {
    case LTAG_REF00:    goto DO_DEREF;
    case LTAG_REF01:    goto DO_DEREF;
    case LTAG_REF10:    goto DO_DEREF;
    case LTAG_REF11:    goto DO_DEREF;
    case LTAG_MARK0:    goto DO_MARK;
    case LTAG_MARK1:    goto DO_MARK;
    case LTAG_SMALLINT: goto DO_SMALLINT;
    case LTAG_LITERAL:  goto DO_LITERAL;
    case LTAG_LTUPLE0:  goto DO_LTUPLE;
    case LTAG_LTUPLE1:  goto DO_LTUPLE;
    case LTAG_SRECORD0: goto DO_SRECORD;
    case LTAG_SRECORD1: goto DO_SRECORD;
    case LTAG_CONST0:   goto DO_CONST;
    case LTAG_CONST1:   goto DO_CONST;
    case LTAG_VAR0:     goto DO_I_VAR;
    case LTAG_VAR1:     goto DO_I_VAR;
    }

  DO_MARK:
    *t = makeTaggedRef((TaggedRef*) tagged2UnmarkedPtr(aux));
    continue;

  DO_SMALLINT:
    *t = aux;
    continue;

  DO_LITERAL:
     *t = makeTaggedLiteral(tagged2Literal(aux)->_cac());
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

     OzVariable * cv;
     TaggedRef  * var_ptr;

  DO_I_VAR:
     cv = tagged2Var(aux);

     // Note: the optimized variables are checked first, before
     // "cacIsMarked"!
     if (cv->getType() == OZ_VAR_OPT) {
       Board *bb = cv->getBoardInternal();
       // 'bb' is the "from space" board;

       // Now, if we've reached a variable through a REF2, that
       // variable can be already GC"ed:
       if (!isSTAligned(aux_ptr) &&
           IsToSpaceBoard(cv->getBoardInternal())) {
         GCDBG_INTOSPACE(aux_ptr);
         *t = makeTaggedRef(aux_ptr);
       } else if (NEEDSCOPYING(bb)) {
         bb = bb->_cacBoard();
         Assert(bb);
         vf.defer(aux_ptr, t);
       } else {
         *t = makeTaggedRef(aux_ptr);
       }

       continue;
     }

     if (cv->cacIsMarked()) {
       Assert(oz_isVar(*(cv->cacGetFwd())));
       *t = makeTaggedRef(cv->cacGetFwd());
       continue;
     }

     if (!NEEDSCOPYING(cv->getBoardInternal())) {
       *t = makeTaggedRef(aux_ptr);
       continue;
     }

     var_ptr = (TaggedRef *) oz_heapMalloc(sizeof(TaggedRef));
     *t = makeTaggedRef(var_ptr);

  DO_VAR:
     *var_ptr = makeTaggedVar(cv->_cacVarInline());
     cv->_cacMark(var_ptr);
     continue;

  DO_D_VAR:
     cv = tagged2Var(aux);

     if (cv->getType() == OZ_VAR_OPT) {
       Board *bb = cv->getBoardInternal();
       // 'bb' is the "from space" board;

       // Note that we cannot reach here an already collected
       // variable: that would mean that we scan some data structure
       // for a second time;
       if (NEEDSCOPYING(bb)) {
         bb = bb->_cacBoard();
         // 'bb' can be unscanned yet (not "cacRecurse"d, in our
         // terminology). Note that the OptVar in it is already
         // collected (but not necessarily scanned);
         Assert(bb);
         *t = bb->getOptVar();
         if (isSTAligned(f)) {
           STOREFWDMARK(f, t);
         } else {
           STOREPSEUDOFWDMARK(f, t);
         }
       } else {
         *f = makeTaggedRef(t);
         *t = aux;
         if (isSTAligned(f)) {
           STOREFWDMARK(f, t);
         } else {
           STOREPSEUDOFWDMARK(f, t);
         }
       }

       continue;
     }

     if (cv->cacIsMarked()) {
       Assert(oz_isVar(*(cv->cacGetFwd())));
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
     goto DO_VAR;
  }
}

//
Suspendable * Suspendable::_cacSuspendable(void) {
  return (this == NULL) ? (Suspendable *) NULL : _cacSuspendableInline(NO);
}

OZ_Term * OZ_cacAllocBlock(int n, OZ_Term * frm) {
  if (n==0)
    return (OZ_Term *) NULL;

  OZ_Term * to = (OZ_Term *) CAC_MALLOC(n * sizeof(OZ_Term));

  OZ_cacBlock(frm, to, n);

  return to;
}
