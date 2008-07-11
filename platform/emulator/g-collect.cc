/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Raphael Collet (raphael.collet@uclouvain.be)
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
#include "gentraverser.hh"
#include "indexing.hh"

#ifndef GY_CACHE_RESET_OFF 
extern void resetLivenessCaches(void);
#endif

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

VarFixStack   vf;
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
  int count;

  ExtRefNode(TaggedRef *el, ExtRefNode *n): elem(el), next(n), count(1) { 
    Assert(elem); 
  }

  ExtRefNode(ExtRefNode* ptr, ExtRefNode* lst)
    : elem(ptr->elem), next(lst), count(ptr->count) {}

  static void gCollect(void) {
    ExtRefNode* ptr = extRefs;
    ExtRefNode* lst = NULL;
    while (ptr) {
      lst = new ExtRefNode(ptr, lst);
      oz_gCollectTerm(*lst->elem,*lst->elem);
      ptr = ptr->next;
    }
    extRefs = lst;
  }

  static void protect(TaggedRef *el) {
    Assert(oz_isRef(*el) || !oz_isVar(*el));
    ExtRefNode* ptr = extRefs;
    while (ptr) {
      if (ptr->elem==el) {
	++ptr->count;
	return;
      }
      ptr = ptr->next;
    }
    extRefs = new ExtRefNode(el, extRefs);
  }

  static Bool unprotect(TaggedRef *el) {
    Assert(el);
    ExtRefNode* ptr = extRefs;
    ExtRefNode** prev = &extRefs;
    while (ptr) {
      if (ptr->elem==el) {
	if (--ptr->count == 0) {
	  *prev = ptr->next;
	}
	return OK;
      }
      prev = &ptr->next;
      ptr  =  ptr->next;
    }
    return NO;
  }

};


Bool oz_protect(TaggedRef *ref) {
  ExtRefNode::protect(ref);
  return OK;
}

Bool oz_unprotect(TaggedRef *ref) {
  return ExtRefNode::unprotect(ref);
}

void oz_unprotectAllOnExit() {
  extRefs = (ExtRefNode *) 0;
}



/*
 * Post-mortem finalization
 *
 * The stuff below maintains a list of post-mortem triples.  Each
 * triple consists in an entity E, a port P, and an item I.  Once E
 * becomes unreachable, I is sent on port P, and the triple is removed
 * from the list.
 */

class PostMortemTriple {
public:
  USEHEAPMEMORY;

  TaggedRef entity, port, item;
  PostMortemTriple* next;

  PostMortemTriple(TaggedRef e, TaggedRef p, TaggedRef i, PostMortemTriple* n):
    entity(e), port(p), item(i), next(n) {}
};

static PostMortemTriple* pm_list = NULL;

// register a triple for post-mortem finalization
void registerPostMortem(TaggedRef entity, TaggedRef port, TaggedRef item) {
  // OptVar's are not marked by the garbage collector, so we have to
  // de-optimize them in order to make gcPostMortemCheck() work.
  DEREF(entity, entityPtr);
  if (oz_isOptVar(entity)) { (void) oz_getNonOptVar(entityPtr); }
  // now insert entity in pm_list
  pm_list = new PostMortemTriple(entity, port, item, pm_list);
}

// mark ports and items (at the beginning of GC)
void gcPostMortemRoots() {
  PostMortemTriple* t = pm_list;
  pm_list = NULL;
  for (; t; t = t->next) {	// rebuild pm_list in the new heap
    oz_gCollectTerm(t->port, t->port);
    oz_gCollectTerm(t->item, t->item);
    registerPostMortem(t->entity, t->port, t->item);
  }
}

// check which entities must be finalized (at the very end of GC)
void gcPostMortemCheck() {
  for (PostMortemTriple* t = pm_list; t; t = t->next) {
    if (isGCMarkedTerm(t->entity)) {
      oz_gCollectTerm(t->entity, t->entity);
    } else {
      t->entity = makeTaggedNULL();     // nullify refs to dead entities
    }
  }
}

// perform finalization of dead entities (after GC)
void gcPostMortemFinalize() {
  PostMortemTriple** tptr = &pm_list;
  PostMortemTriple* t;
  while (t = *tptr) {
    if (t->entity != makeTaggedNULL()) {
      tptr = &(t->next);
    } else {
      OZ_Return ret = oz_sendPort(t->port, t->item);
      Assert(ret == PROCEED);
      *tptr = t->next;		// drop post-mortem triple t from list
    }
  }
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

void PrTabEntry::gCollect(void)
{
  gCollectCode(getCodeBlock());
}

void PrTabEntry::sClone(void)
{
}

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

    Assert(pte->getPC() != NOCODE);
    if (pte->getCodeBlock()->isReferenced()) {
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
  Assert(this);
  if (isCollected()) return;

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
  iht->gCollect();                                        \
}

//
inline
void CodeArea::gCollectInstructions(void) {
  ProgramCounter PC = getStart();
  while (OK) {
    // Among others, the distribution's builder assumes a code area is
    // scanned/GCed until the 'ENDOFFILE' opcode;
#ifdef THREADED
    Assert(*PC);
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
    case DECONSCALLX:
    case DECONSCALLY:
    case DECONSCALLG:
    case TAILDECONSCALLX:
    case TAILDECONSCALLG:
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
    case CONSCALLX:
    case CONSCALLY:
    case CONSCALLG:
    case TAILCONSCALLX:
    case TAILCONSCALLG:
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
void CodeArea::gCollectCodeAreaStart(void)
{
  (CodeArea::skipInGC)->referenced = OK;
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

//
// 'fillNode' just fills up a gap, 'uFillNode' ("unique fillNode")
// makes also sure that the corresponding spointer in the builder's
// stack points exactly to it. 'fillNode' is only useful for records,
// since records are filled not using spointer"s but dedicated tasks.
static OZ_Term fillNode = 0;
static OZ_Term uFillNode = 0;

//
// Step through every stack frame and (a) fill up the gaps (NB: they
// are in the 'from' space) with a dedicated value ('fillNode',
// 'uFillNode'), and (b) collect all the terms saved there. Since the
// action is atomic, all the gaps are filled before the garbage
// collector can reach them.
void Builder::gCollect()
{
  //
  GetBTFrame(frame);
  //
  // 'uFillNode' stays for 'unique fillNode', that is: there is one
  // single filling for both the hole in a compound term and the
  // corresponding spointer. One cannot just fill&GC the hole beneath
  // an spointer with 'fillNode' because then there would be two
  // copies of it in the 'to' space, so the spointer will be invalid.

  if (!((int) fillNode)) {
#ifdef DEBBUG_CHECK
    // a "unique name" does not need to be collected, so it is
    // initialized only once. Should that change, the bug will be easy
    // to discover;
    fillNode = oz_uniqueName("fill node");
#else
    fillNode = makeTaggedSmallInt(-7);
#endif
  }
  Assert(uFillNode);

  //
  // frame is cached, so no 'BuilderStack::isEmpty()' here:
  while (frame > BuilderStack::array) {
    GetBTTaskType(frame, type);
    switch (type) {

      //
      // Trivial case: just dump the 'fillNode' into the hole;
    case BT_spointer:
      // ... the same as before modulo that the next task should not
      // refer to any gaps (in terms of builder, no 'value' is
      // available for that next task);
    case BT_spointer_iterate:
      {
	GetBTTaskPtr1(frame, OZ_Term*, spointer);
	if (spointer == &result || spointer == &blackhole) {
	  // in either case do nothing 'cause these cells are not
	  // visible from the heap;
	} else {
	  // first phase: fill the gap;
	  *spointer = uFillNode;	// in the term;
	  // second phase: GC it, such that the spointer in the copy
	  // will get eventually updated (using GC's VarFixStack business).
	  OZ_Term &v = GetBTTaskArg1Ref(frame, OZ_Term);
	  oz_gCollectTerm(v, v);
	}

	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // 'buildValue' task keeps a value that is to be used by the
      // next task, so let's GC it:
    case BT_buildValue:
      {
	OZ_Term &v = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(v, v);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // 'makeTuple' does not contain any Oz values. The place where
      // that tuple will be stored will be botched with 'fillNode';
    case BT_makeTuple:
    case BT_makeTupleMemo:
      NextBTFrame(frame);
      break;			// case;

      //
      // 'takeRecordArity(Label)' does not hold any Oz values, so just 
      // skip them:
    case BT_takeRecordLabel:
    case BT_takeRecordLabelMemo:
      NextBT2Frames(frame);
      break;			// case;

      //
      // 'takeRecordArity(Label)' holds label:
    case BT_takeRecordArity:
    case BT_takeRecordArityMemo:
      {
	OZ_Term &l = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(l, l);
	//
	NextBT2Frames(frame);
	break;			// case;
      }

      //
      // 'makeRecordSync' holds both label and arity list:
    case BT_makeRecordSync:
    case BT_makeRecordMemoSync:
      {
	OZ_Term &l = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(l, l);
	OZ_Term &a = GetBTTaskArg2Ref(frame, OZ_Term);
	oz_gCollectTerm(a, a);
	//
	// no optimizations for following 'spointer' task, etc.
	NextBT2Frames(frame);
	break;
      }

      //
      // 'recordArg' holds a reference to the corresponding record and
      // a feature name, which both must be updated. Holes are to be
      // filled as well:
    case BT_recordArg:
    case BT_recordArg_iterate:
      {
	GetBTTaskPtr1(frame, SRecord*, rec);
	SRecord *nrec = rec->gCollectSRecord();
	ReplaceBTTask1stPtrOnly(frame, nrec);
	//
	OZ_Term &fea = GetBTTaskArg2Ref(frame, OZ_Term);
	oz_gCollectTerm(fea, fea);
	//
	nrec->setFeature(fea, fillNode);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // 'dictKey' holds only the dictionary; 'dictVal' in addition -
      // the key. The not-yet-processed dictionary entries do not need
      // to be initialized, so the dictionary itself is left
      // untouched;
    case BT_dictKey:
      {
	OzDictionary* &od = GetBTTaskArg1Ref(frame, OzDictionary*);
	OZ_Term d = makeTaggedConst(od);
	oz_gCollectTerm(d, d);
	od = tagged2Dictionary(d);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
    case BT_dictVal:
      {
	OzDictionary* &od = GetBTTaskArg1Ref(frame, OzDictionary*);
	OZ_Term d = makeTaggedConst(od);
	oz_gCollectTerm(d, d);
	od = tagged2Dictionary(d);
	OZ_Term &k = GetBTTaskArg2Ref(frame, OZ_Term);
	oz_gCollectTerm(k, k);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // no values;
    case BT_fsetvalue:
    case BT_fsetvalueMemo:
      NextBTFrame(frame);
      break;			// case;

      //
      // ... saved head of the list. Note that by now it must be a
      // valid Oz data structure (but not necessarily a well-formed
      // one), with holes filled up!
    case BT_fsetvalueSync:
    case BT_fsetvalueMemoSync:
      {
	OZ_Term &l = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(l, l);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // GName is hanging around:
    case BT_chunk:
    case BT_chunkMemo:
      {
	GetBTTaskPtr1(frame, GName*, gname);
	overwriteGName(gname, fillNode);
	gCollectGName(gname);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
    case BT_takeObjectLock:
    case BT_takeObjectLockMemo:
      NextBT2Frames(frame);
      break;			// case;

      //
    case BT_takeObjectState:
    case BT_takeObjectStateMemo:
      {
	OZ_Term &l = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(l, l);
	//
	NextBT2Frames(frame);
	break;			// case;
      }

      //
    case BT_makeObject:
    case BT_makeObjectMemo:
      {
	OZ_Term &l = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(l, l);
	OZ_Term &s = GetBTTaskArg2Ref(frame, OZ_Term);
	oz_gCollectTerm(s, s);
	//
	NextBT2Frames(frame);
	break;
      }

      //
      // 'OzClass' is already there:
    case BT_classFeatures:
      {
	GetBTTaskPtr1(frame, OzClass*, oc);
	OzClass *noc = (OzClass *) oc->gCollectConstTerm();
	ReplaceBTTask1stPtrOnly(frame, noc);
	// yet unset features are already initialized to zero;
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // yet just GName"s:
    case BT_procFile:
    case BT_procFileMemo:
      {
	NextBTFrame(frame);
	GetBTTaskPtr1(frame, GName*, gname);
	overwriteGName(gname, fillNode);
	gCollectGName(gname);
	//
	NextBT3Frames(frame);
	break;			// case;
      }

      // file name is there:
    case BT_proc:
    case BT_procMemo:
      {
	OZ_Term &fn = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(fn, fn);
	//
	NextBTFrame(frame);
	GetBTTaskPtr1(frame, GName*, gname);
	overwriteGName(gname, fillNode);
	gCollectGName(gname);
	//
	NextBT3Frames(frame);
	break;			// case;
      }

      //
      // 'Abstraction*' held in 'closureElem' must be updated:
    case BT_closureElem:
    case BT_closureElem_iterate:
      {
	GetBTTaskPtr1(frame, Abstraction*, ap);
	GetBTTaskArg2(frame, int, ind);
	Abstraction *nap = (Abstraction *) ap->gCollectConstTerm();
	ReplaceBTTask1stPtrOnly(frame, nap);
	// Patch the 'TO'-space abstraction (there can be several
	// G-registers (and, thus, their patches), but the abstraction
	// itself is copied exactly once!);
	nap->initG(ind, fillNode);
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
    case BT_abstractEntity:
      {
	GetBTTaskPtr1(frame, GTAbstractEntity*, bae);
	bae->gc();
	//
	NextBTFrame(frame);
	break;
      }

      //
      // Dealing with binary areas (e.g. code areas);
    case BT_binary:
      {
	GetBTTaskPtr1(frame, GTAbstractEntity*, bae);
	if (bae)
	  bae->gc();
	//
	NextBTFrame(frame);
	break;
      }

      //
    case BT_binary_getValue:
      {
	NextBTFrame(frame);
	GetBTFramePtr2(frame, GTAbstractEntity*, bae);
	if (bae)
	  bae->gc();
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // ... but 'binary_getValueSync' holds a value, which at that
      // point is already well-formed (without holes):
    case BT_binary_getValueSync:
      {
	OZ_Term &v = GetBTTaskArg1Ref(frame, OZ_Term);
	oz_gCollectTerm(v, v);
	//
	NextBTFrame(frame);
	GetBTFramePtr2(frame, GTAbstractEntity*, bae);
	if (bae)
	  bae->gc();
	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // the 'arg' cannot refer heap-based data:
    case BT_binary_doGenAction_intermediate:
      NextBTFrame(frame);
      break;			// case;

      //
    case BT_nop:
      NextBTFrame(frame);
      break;			// case;

      //
    default:
      OZ_error("Builder: unknown task!");
    }
  }

  //
  // If a (partially instantiated) result is known already, gc it
  // *after* filling up the holes. That's because 'oz_gCollectTerm'
  // implicitly assumes that a term is well-formed (that is, there are
  // no holes in it); otherwise the 'to'-space copy of the term cannot
  // be scanned.
  if (result) {
    oz_gCollectTerm(result, result);
  }

  //
  // In addition, we have to GC the BuilderRefTable<OZ_Term> as well:
  OZ_Term *tp = valueRT->getFirstRef();
  while (tp) {
    OZ_Term &tte = *tp;
    oz_gCollectTerm(tte, tte);
    tp = valueRT->getNextRef(tp);
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
  unsigned int used   = getUsedMemory();
  unsigned int free   = min(ozconf.heapFree,99);
  unsigned int wanted = max(((long) used) * (100 / (100 - free)),
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

void AM::gCollect(int msgLevel)
{
  gCollectWeakDictionariesInit();
  vf.init();
  cacStack.init();
  am.nextGCStep();

  (*gCollectGlueStart)();

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

#ifndef GY_CACHE_RESET_OFF
  resetLivenessCaches();
#endif
  //
  uFillNode = am.getCurrentOptVar();
  _rootBoard = _rootBoard->gCollectBoard();   // must go first!
  setCurrent(_rootBoard, _rootBoard->getOptVar());
  aritytable.gCollect();

  // gCollectCodeAreaStart() decides whether or not this gc will discard
  // CodeAreas. Since threads contain indirect pointers to CodeAreas, it
  // must be run before threadsPool.gCollect().  Fixes a bug where a thread
  // had the only live reference to a CodeArea.
  // PR#953.
  CodeArea::gCollectCodeAreaStart();

  threadsPool.gCollect();

  // mm2: Assert(isEmptySuspendVarList());
  emptySuspendVarList();

  if (defaultExceptionHdl)
    oz_gCollectTerm(defaultExceptionHdl,defaultExceptionHdl);
  oz_gCollectTerm(debugPort, debugPort);

  PrTabEntry::gCollectPrTabEntries();
  ExtRefNode::gCollect();
  gcPostMortemRoots();
  cacStack.gCollectRecurse();

  gCollectDeferWatchers();
  (*gCollectGlueRoots)();
  cacStack.gCollectRecurse();

  // now make sure that we preserve all weak dictionaries
  // that still have stuff to do

  gCollectWeakDictionariesPreserve();
  cacStack.gCollectRecurse();	// recurse on weak dicts

  // now everything that is locally not garbage
  // has been marked and copied - whatever remains in
  // the weak dictionaries that is not marked is
  // otherwise inaccessible and should be finalized

  // raph: The loop below is necessary for finalizing weak
  // dictionaries that are revived by other weak dictionaries.

  while (gCollectWeakDictionariesHasMore()) {
    gCollectWeakDictionariesContent();
    weakReviveStack.recurse();      // copy stuff scheduled for finalization
    cacStack.gCollectRecurse();
  }

#ifdef NEW_NAMER
  GCMeManager::gCollect();
  cacStack.gCollectRecurse();
#endif

  (*gCollectGlueWeak)();
  cacStack.gCollectRecurse();

  weakStack.recurse();		// must come after namer gc

// -----------------------------------------------------------------------
// ** second phase: the reference update stack has to be checked now

  vf.gCollectFix();

  Assert(cacStack.isEmpty());

  gcPostMortemCheck();
  gnameTable.gCollectGNameTable();
  //   MERGECON gcPerdioFinal();
  gCollectSiteTable();
  (*gCollectGlueFinal)();
  Assert(cacStack.isEmpty());

  GCDBG_EXITSPACE;

  if (!codeGCgeneration)
    PrTabEntry::gCollectDispose();
  CodeArea::gCollectCollectCodeBlocks();
  AbstractionEntry::freeUnusedEntries();

  oldChain->deleteChunkChain();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                garbage collection is finished here

  gCollectGeSpaces();

  cachedStack = NULL;

  ozstat.printGcMsg(msgLevel);
  
#ifdef DEBUG_CHECK
  isCollecting = NO;
#endif

  vf.exit();
  cacStack.exit();

  gcPostMortemFinalize();

  //  malloc_stats();
  DebugCode(uFillNode = 0;);
} // AM::gc
