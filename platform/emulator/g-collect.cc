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

/*
 * Forward reference for code garbage collection
 *
 */

static void gCollectCode(CodeArea *block);
static void gCollectCode(ProgramCounter PC);


/*
 * Collection stacks needed for both gCollect and sClone
 *
 */

VarFix   varFix;
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

/* protect a ref, that will never change its initial value
 *  --> no need to remember it, if it's a small int or atom
 */
Bool oz_staticProtect(TaggedRef *ref) {
  if (needsNoCollection(*ref))
    return OK;

  return oz_protect(ref);
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

void ArityTable::gCollect(void) {
  for (int i = size; i--; ) {
    if (table[i] != NULL)
      (table[i])->gCollect();
  }
}


/*
 * Code garbage collection
 *
 */


static int codeGCgeneration = CODE_GC_CYLES;

void PrTabEntry::gCollectPrTabEntries(void) {
  for (PrTabEntry *aux = allPrTabEntries; aux; aux=aux->next)
    OZ_gCollectBlock(&(aux->printname),&(aux->printname),3);
}

void AbstractionEntry::freeUnusedEntries() {
  AbstractionEntry *aux = allEntries;
  allEntries = NULL;
  while (aux) {
    AbstractionEntry *aux1 = aux->next;
    if (aux->collected ||
        aux->abstr==makeTaggedNULL()) {
      // RS: HACK alert: compiler might have reference to
      // abstraction entries: how to detect them??
      aux->collected = NO;
      aux->next  = allEntries;
      allEntries = aux;
    } else {
      delete aux;
    }
    aux = aux1;
  }
}

inline
void AbstractionEntry::gCollectAbstractionEntry(void) {
  if (this==NULL || collected) return;

  collected = OK;
  oz_gCollectTerm(abstr,abstr);
}


void CodeArea::gCollectCodeBlock(void) {
  if (referenced == NO) {
    referenced = OK;
    gclist->collectGClist();
  }
}

void gCollectCode(CodeArea *block) {
  if (codeGCgeneration!=0)
    return;

  block->gCollectCodeBlock();
}


void gCollectCode(ProgramCounter PC) {
  gCollectCode(CodeArea::findBlock(PC));
}

void CodeGCList::collectGClist(void) {
  CodeGCList *aux = this;
  while(aux) {
    for (int i=aux->nextFree; i--; ) {
      switch (aux->block[i].tag) {
      case C_TAGGED:
        oz_gCollectTerm(*(TaggedRef*)aux->block[i].pc,
                        *(TaggedRef*)aux->block[i].pc);
        break;
      case C_ABSTRENTRY:
        ((AbstractionEntry*)*(aux->block[i].pc))->gCollectAbstractionEntry();
        break;
      case C_INLINECACHE:
        ((InlineCache*)aux->block[i].pc)->invalidate();
        break;
      case C_FREE:
        break;
      default:
        Assert(0);
      }
    }
    aux = aux->next;
  }
}

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
  int wanted = ((ozconf.heapFree == 100)
                ? ozconf.heapMaxSize
                : max(((long) used) * (100 / (100 - ozconf.heapFree)),
                      ozconf.heapMinSize));

  /* Try to align as much as possible to end of blocksize */
  int block_size = HEAPBLOCKSIZE / KB;
  int block_dist = wanted % block_size;

  if (block_dist > 0)
    block_dist = block_size - block_dist;

  wanted += min(block_dist,
                (((long) wanted) * ozconf.heapTolerance / 100));

  if (wanted > ozconf.heapMaxSize) {
    if (ozconf.runningUnderEmacs) {
      OZ_warning("\n*** Heap Max Size exceeded: Increasing from %d to %d.\n",
                 ozconf.heapMaxSize,wanted);
      prefixError();
      fflush(stdout);
    }
    ozconf.heapMaxSize = wanted;
  }

  ozconf.heapThreshold = wanted;

  unsetSFlag(StartGC);
}

void AM::gCollect(int msgLevel) {

  varFix.init();
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
    xRegs[j] = taggedVoidValue;

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

  oz_gCollectTerm(finalize_handler,finalize_handler);
  cacStack.gCollectRecurse();
  gCollect_finalize();
  gCollectWeakDictionaries();
  gCollectDeferWatchers();
  (*gCollectPerdioRoots)();
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

  varFix.gCollectFix();

  Assert(cacStack.isEmpty());

  GT.gCollectGNameTable();
  //   MERGECON gcPerdioFinal();
  gCollectSiteTable();
  (*gCollectPerdioFinal)();
  Assert(cacStack.isEmpty());

  GCDBG_EXITSPACE;

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

  varFix.exit();
  cacStack.exit();

} // AM::gc
