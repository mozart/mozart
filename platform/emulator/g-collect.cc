/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *
 *  Copyright:
 *    Christian Schulte, 1999
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

#include "cac.hh"

#define G_COLLECT
#undef  S_CLONE

#include "codearea.hh"
#include "indexing.hh"


/*
 * Entry points for code garbage collection
 *
 */

static int codeGCgeneration = CODE_GC_CYLES;

inline
void gCollectCode(CodeArea *block) {
  if (codeGCgeneration)
    return;

  block->gCollectCodeBlock();
}

inline
void gCollectCode(ProgramCounter PC) {
  if (codeGCgeneration)
    return;

  CodeArea::findBlock(PC)->gCollectCodeBlock();
}


/*
 * Collection stacks needed for both gCollect and sClone
 *
 */

VarFix   vf;
CacStack cacStack;



#include "cac.cc"


/*
 * Maintaining external references into heap
 *
 */

class ExtRefNode;
static ExtRefNode *extRefs = NULL;

class ExtRefNode {
public:
  USEHEAPMEMORY;

  TaggedRef *elem;
  ExtRefNode *next;

  ExtRefNode(TaggedRef *el, ExtRefNode *n): elem(el), next(n){
    Assert(elem);
  }

  void remove(void) {
    elem = NULL;
  }
  ExtRefNode *add(TaggedRef *el) {
    return new ExtRefNode(el,this);
  }

  ExtRefNode *gCollect(void) {
    ExtRefNode *aux = this;
    ExtRefNode *ret = NULL;
    while (aux) {
      if (aux->elem) {
        ret = new ExtRefNode(aux->elem,ret);
        oz_gCollectTerm(*ret->elem,*ret->elem);
      }
      aux = aux->next;
    }
    return ret;
  }

  ExtRefNode *protect(TaggedRef *el) {
    Assert(oz_isRef(*el) || !oz_isVariable(*el));
    Assert(!find(el));
    return add(el);
  }

  Bool unprotect(TaggedRef *el) {
    Assert(el);
    ExtRefNode *aux = extRefs;
    while (aux) {
      if (aux->elem==el) {
        aux->remove();
        return OK;
      }
      aux = aux->next;
    }
    return NO;
  }

  ExtRefNode * find(TaggedRef *el) {
    Assert(el);
    ExtRefNode *aux = extRefs;
    while (aux) {
      if (aux->elem==el)
        break;
      aux = aux->next;
    }
    return aux;
  }

};


Bool oz_protect(TaggedRef *ref) {
  extRefs = extRefs->protect(ref);
  return OK;
}

Bool oz_unprotect(TaggedRef *ref) {
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



/*
 * Collection of arities
 *
 */

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

inline
void ArityTable::gCollect(void) {
  for (int i = size; i--; ) {
    if (table[i] != NULL)
      (table[i])->gCollect();
  }
}

inline
void ThreadsPool::gCollect(void) {
  _q[ HI_PRIORITY].gCollect();
  _q[MID_PRIORITY].gCollect();
  _q[LOW_PRIORITY].gCollect();
}


/*
 * Code garbage collection
 *
 */


inline
void PrTabEntry::gCollectPrTabEntries(void) {
  for (PrTabEntry *aux = allPrTabEntries; aux; aux=aux->next)
    OZ_gCollectBlock(&(aux->printname),&(aux->printname),3);
}

inline
void PrTabEntry::gCollectDispose(void) {
  // Must be called before gCollectCodeBlocks!
  PrTabEntry * pte = allPrTabEntries;
  allPrTabEntries = NULL;

  while (pte) {
    PrTabEntry * n = pte->next;

    if (pte->getPC() == NOCODE || pte->getCodeBlock()->isReferenced()) {
      pte->next = allPrTabEntries;
      allPrTabEntries = pte;
    } else {
      delete pte;
    }

    pte = n;
  }
}

inline
void AbstractionEntry::freeUnusedEntries(void) {
  AbstractionEntry *aux = allEntries;
  allEntries = NULL;
  while (aux) {
    AbstractionEntry *aux1 = aux->getNext();
    if (aux->isCollected() || aux->abstr==makeTaggedNULL()) {
      // RS: HACK alert: compiler might have reference to
      // abstraction entries: how to detect them??
      aux->unsetCollected();
      aux->setNext(allEntries);
      allEntries = aux;
    } else {
      delete aux;
    }
    aux = aux1;
  }
}

inline
void AbstractionEntry::gCollectAbstractionEntry(void) {
  if (this==NULL || isCollected()) return;

  setCollected();
  oz_gCollectTerm(abstr,abstr);
}

inline
void IHashTable::gCollect(void) {
  for (int i=getSize(); i--; )
    if (entries[i].val)
      oz_gCollectTerm(entries[i].val,entries[i].val);
}


#define CODEGC_ABSTRACTION(PCR) \
{ AbstractionEntry * ae = (AbstractionEntry *) getAdressArg(PC+PCR); \
  if (ae) ae->gCollectAbstractionEntry();                            \
}

#define CODEGC_TAGGED(PCR) \
{ TaggedRef * tr = (TaggedRef *) (PC+PCR); \
  OZ_gCollectBlock(tr,tr,1);               \
}

#define CODEGC_RECORDARITY(PCR) {};

#define CODEGC_CACHE(PCR) \
  ((InlineCache *) (PC+PCR))->invalidate();

#define CODEGC_CALLMETHODINFO(PCR) \
{ CallMethodInfo * cmi = (CallMethodInfo *) getAdressArg(PC+PCR); \
  if (cmi) oz_gCollectTerm(cmi->mn, cmi->mn);                     \
}

#define CODEGC_IHASHTABLE(PCR) \
{ IHashTable * iht = (IHashTable *) getAdressArg(PC+PCR); \
  if (iht) iht->gCollect();                               \
}

inline
void CodeArea::gCollectInstructions(void) {
  ProgramCounter PC = getStart();
  while (OK) {
#ifdef THREADED
    if (!*PC) {
      // Can happen while codearea is under construction from compiler:
      // Initially, the codearea is filled with zeros.
      // The check is only necessary for threaded code, since otherwise
      // null represents ENDOFFILE;
      return;
    }
#endif
    Opcode op = getOpcode(PC);
    switch (op) {
    case ENDOFFILE:
      return;
    case PROFILEPROC:
    case RETURN:
    case POPEX:
    case DEALLOCATEL10:
    case DEALLOCATEL9:
    case DEALLOCATEL8:
    case DEALLOCATEL7:
    case DEALLOCATEL6:
    case DEALLOCATEL5:
    case DEALLOCATEL4:
    case DEALLOCATEL3:
    case DEALLOCATEL2:
    case DEALLOCATEL1:
    case DEALLOCATEL:
    case ALLOCATEL10:
    case ALLOCATEL9:
    case ALLOCATEL8:
    case ALLOCATEL7:
    case ALLOCATEL6:
    case ALLOCATEL5:
    case ALLOCATEL4:
    case ALLOCATEL3:
    case ALLOCATEL2:
    case ALLOCATEL1:
    case SKIP:
      PC += 1;
      break;
    case DEFINITIONCOPY:
    case DEFINITION:
      CODEGC_ABSTRACTION(4);
      PC += 6;
      break;
    case CLEARY:
    case GETVOID:
    case GETVARIABLEY:
    case GETVARIABLEX:
    case FUNRETURNG:
    case FUNRETURNY:
    case FUNRETURNX:
    case GETRETURNG:
    case GETRETURNY:
    case GETRETURNX:
    case EXHANDLER:
    case BRANCH:
    case SETSELFG:
    case GETSELF:
    case ALLOCATEL:
    case UNIFYVOID:
    case UNIFYVALUEG:
    case UNIFYVALUEY:
    case UNIFYVALUEX:
    case UNIFYVARIABLEY:
    case UNIFYVARIABLEX:
    case GETLISTG:
    case GETLISTY:
    case GETLISTX:
    case SETVOID:
    case SETVALUEG:
    case SETVALUEY:
    case SETVALUEX:
    case SETVARIABLEY:
    case SETVARIABLEX:
    case PUTLISTY:
    case PUTLISTX:
    case CREATEVARIABLEY:
    case CREATEVARIABLEX:
    case ENDDEFINITION:
      PC += 2;
      break;
    case INLINEMINUS1:
    case INLINEPLUS1:
    case CALLBI:
    case GETVARVARYY:
    case GETVARVARYX:
    case GETVARVARXY:
    case GETVARVARXX:
    case TESTLISTG:
    case TESTLISTY:
    case TESTLISTX:
    case LOCKTHREAD:
    case TAILCALLG:
    case TAILCALLX:
    case CALLG:
    case CALLY:
    case CALLX:
    case CALLGLOBAL:
    case UNIFYVALVARGY:
    case UNIFYVALVARGX:
    case UNIFYVALVARYY:
    case UNIFYVALVARYX:
    case UNIFYVALVARXY:
    case UNIFYVALVARXX:
    case UNIFYXG:
    case UNIFYXY:
    case UNIFYXX:
    case CREATEVARIABLEMOVEY:
    case CREATEVARIABLEMOVEX:
    case MOVEGY:
    case MOVEGX:
    case MOVEYY:
    case MOVEYX:
    case MOVEXY:
    case MOVEXX:
      PC += 3;
      break;
    case TESTLE:
    case TESTLT:
    case MOVEMOVEYXXY:
    case MOVEMOVEXYYX:
    case MOVEMOVEYXYX:
    case MOVEMOVEXYXY:
      PC += 5;
      break;
    case GETRECORDG:
    case GETRECORDY:
    case GETRECORDX:
    case PUTRECORDY:
    case PUTRECORDX:
      CODEGC_TAGGED(1);
      CODEGC_RECORDARITY(2);
      PC += 4;
      break;
    case CALLCONSTANT:
    case GETNUMBERG:
    case GETNUMBERY:
    case GETNUMBERX:
    case GETLITERALG:
    case GETLITERALY:
    case GETLITERALX:
    case PUTCONSTANTY:
    case PUTCONSTANTX:
      CODEGC_TAGGED(1);
      PC += 3;
      break;
    case LOCALVARNAME:
    case GLOBALVARNAME:
    case UNIFYLITERAL:
    case UNIFYNUMBER:
    case SETCONSTANT:
      CODEGC_TAGGED(1);
      PC += 2;
      break;
    case SETPROCEDUREREF:
      CODEGC_ABSTRACTION(1);
      PC += 2;
      break;
    case TESTBI:
    case INLINEMINUS:
    case INLINEPLUS:
    case TESTBOOLG:
    case TESTBOOLY:
    case TESTBOOLX:
    case GETLISTVALVARX:
      PC += 4;
      break;
    case CALLMETHOD:
      CODEGC_CALLMETHODINFO(1);
      PC += 3;
      break;
    case FASTTAILCALL:
    case FASTCALL:
    case CALLPROCEDUREREF:
      CODEGC_ABSTRACTION(1);
      PC += 3;
      break;
    case TAILSENDMSGG:
    case TAILSENDMSGY:
    case TAILSENDMSGX:
    case SENDMSGG:
    case SENDMSGY:
    case SENDMSGX:
      CODEGC_TAGGED(1);
      CODEGC_RECORDARITY(3);
      CODEGC_CACHE(4);
      PC += 6;
      break;
    case INLINEASSIGN:
    case INLINEAT:
      CODEGC_TAGGED(1);
      CODEGC_CACHE(3);
      PC += 5;
      break;
    case TESTNUMBERG:
    case TESTNUMBERY:
    case TESTNUMBERX:
    case TESTLITERALG:
    case TESTLITERALY:
    case TESTLITERALX:
      CODEGC_TAGGED(2);
      PC += 4;
      break;
    case TESTRECORDG:
    case TESTRECORDY:
    case TESTRECORDX:
      CODEGC_TAGGED(2);
      CODEGC_RECORDARITY(3);
      PC += 5;
      break;
    case MATCHG:
    case MATCHY:
    case MATCHX:
      CODEGC_IHASHTABLE(2);
      PC += 3;
      break;
    case DEBUGEXIT:
    case DEBUGENTRY:
      CODEGC_TAGGED(1);
      CODEGC_TAGGED(2);
      CODEGC_TAGGED(3);
      CODEGC_TAGGED(4);
      PC += 5;
      break;
    case INLINEDOT:
      CODEGC_TAGGED(2);
      CODEGC_CACHE(4);
      PC += 6;
      break;
    default:
      Assert(0);
      break;
    }
  }
}

void CodeArea::gCollectCodeBlock(void) {
  if (referenced == NO) {
    referenced = OK;
    if (this != CodeArea::skipInGC)
      gCollectInstructions();
  }
}

inline
void CodeArea::gCollectCodeAreaStart(void) {
  if (ozconf.codeGCcycles == 0) {
    codeGCgeneration = 1;
  } else if (++codeGCgeneration >= ozconf.codeGCcycles) {
    // switch code GC on
    codeGCgeneration = 0;
    return;
  }

  CodeArea *code = allBlocks;

  while (code) {
    code->gCollectCodeBlock();
    code = code->nextBlock;
  }
}

inline
void CodeArea::gCollectCollectCodeBlocks(void) {
  CodeArea *code = allBlocks;
  allBlocks = NULL;
  while (code) {
    if (!code->referenced) {
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


/*
 * Main routines for invoking garbage collections
 *
 */

// signal handler
void checkGC(void) {
  Assert(!am.isCritical());
  if (getUsedMemory() > unsigned(ozconf.heapThreshold) && ozconf.gcFlag) {
    am.setSFlag(StartGC);
  }
}

void AM::doGCollect(void) {
  Assert(oz_onToplevel());

  /* do gc */
  gCollect(ozconf.gcVerbosity);

  /* calc limits for next gc */
  int used   = getUsedMemory();
  int free   = min(ozconf.heapFree,99);
  int wanted = max(((long) used) * (100 / (100 - free)),
                   ozconf.heapMinSize);

  /* Try to align as much as possible to end of blocksize */
  int block_size = HEAPBLOCKSIZE / KB;
  int block_dist = wanted % block_size;

  if (block_dist > 0)
    block_dist = block_size - block_dist;

  wanted += min(block_dist,
                (((long) wanted) * ozconf.heapTolerance / 100));

  ozconf.heapThreshold = wanted;

  unsetSFlag(StartGC);
}

void AM::gCollect(int msgLevel) {

  gCollectWeakDictionariesInit();
  vf.init();
  cacStack.init();

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
    XREGS[j] = taggedVoidValue;

  Assert(trail.getUsed() == 1);
  Assert(cachedSelf==0);
  Assert(ozstat.currAbstr==NULL);
  Assert(_inEqEq==FALSE);
  Assert(_rootBoard);

  _rootBoard = _rootBoard->gCollectBoard();   // must go first!
  setCurrent(_rootBoard);

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

  cacStack.gCollectRecurse();
  gCollectDeferWatchers();
  (*gCollectPerdioRoots)();
  cacStack.gCollectRecurse();

  // now make sure that we preserve all weak dictionaries
  // that still have stuff to do

  gCollectWeakDictionariesPreserve();

  // now everything that is locally not garbage
  // has been marked and copied - whatever remains in
  // the weak dictionaries that is not marked is
  // otherwise inaccessible and should be finalized

  gCollectWeakDictionariesContent();
  weakReviveStack.recurse();    // copy stuff scheduled for finalization
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

  vf.gCollectFix();

  Assert(cacStack.isEmpty());

  GT.gCollectGNameTable();
  //   MERGECON gcPerdioFinal();
  gCollectSiteTable();
  (*gCollectPerdioFinal)();
  Assert(cacStack.isEmpty());

  GCDBG_EXITSPACE;

  if (!codeGCgeneration)
    PrTabEntry::gCollectDispose();
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

  vf.exit();
  cacStack.exit();

  //  malloc_stats();

} // AM::gc
