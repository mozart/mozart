/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$
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

#if defined(INTERFACE)
#pragma implementation "gentraverser.hh"
#endif

#include "base.hh"
#include "gentraverser.hh"
#include "pickle.hh"
#include "cac.hh"
#include "gname.hh"

#ifdef DEBUG_CHECK
void marshalingErrorHook()
{
  return;
}
#endif

//
void GenTraverser::gCollect()
{
  StackEntry *top = getTop();
  StackEntry *bottom = getBottom();
  StackEntry *ptr = top;

  //
  while (ptr > bottom) {
    OZ_Term& t = (OZ_Term&) *(--ptr);
    OZ_Term tc = t;
    DEREF(tc, tPtr);

    //
    if (oz_isMark(tc)) {
      switch (tc) {
      case taggedBATask:
	{
	  GTAbstractEntity *desc = (GTAbstractEntity *) *(--ptr);
	  if (desc)
	    desc->gc();
	  --ptr;
	}
	break;

      case taggedSyncTask:
	break;

      case taggedContTask:
	{
	  // there are two arguments: processor and its argument;
	  GTAbstractEntity *desc = (GTAbstractEntity *) *(--ptr);
	  desc->gc();
	  --ptr;
	}
	break;
      }
    } else {
      // do not GC the dereferenced copy - do the original slot;
      oz_gCollectTerm(t, t);
    }
  }
}

//
// 'doit()' is instantiated anew for every single marshaling instance.

//
// Code fragments that create particular data structures should be
// factorized away, but this would make comparing this unmarshaler
// with the recursive one unfair (since it's not done for that one
// neither). 
// 
// An obvious factorization is to have 'constructXXX(...)' functions
// (or virtual methods, if more than one builder are anticipated) that
// take necessary arguments and do whatever necessary for creating the 
// structure. Arguments are (a) all the non-oz-terms, aka gnames, etc.
// and (b) non-proper subtrees, e.g. a record's arity list.
//
// Handling proper subtrees is done by the builder, symmetrically to the
// GenTraverser.

//
// Exception handling for robust unmarshaler
jmp_buf unmarshal_error_jmp;

//
// kost@ : there is no 'fsetcore.hh';
extern void makeFSetValue(OZ_Term,OZ_Term*);

//
void
Builder::buildValueOutline(OZ_Term value, BTFrame *frame,
			   BuilderTaskType type)
{
  Assert(type != BT_spointer);
  Bool doMemo = NO;

  //
  // Iteration invariant: there are correct 'frame' and 'type', but 
  // no argument.
  // Procedure invariant: it gets frame but must get rid of it;
repeat:
  //
  switch (type) {

    //
    // Though it's handled inline, we can get here iteratively:
  case BT_spointer:
    {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      DiscardBTFrame(frame);
      *spointer = value;
      break;
    }

  case BT_spointer_iterate:
    {
      GetBTTaskPtr1(frame, OZ_Term*, spointer);
      *spointer = value;
      CrazyDebug(incDebugNODES(););
      DiscardBTFrame(frame);
      DebugCode(value = (OZ_Term) -1;);	// 'value' is expired;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_buildValue:
    {
      GetBTTaskArg1NoDecl(frame, OZ_Term, value);
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_makeTupleMemo:
    doMemo = OK;
    // fall through;
  case BT_makeTuple:
    {
      GetBTTaskArg1(frame, int, arity);
      SRecord *rec = SRecord::newSRecord(value, arity);
      OZ_Term recTerm = makeTaggedSRecord(rec);
      if (doMemo) {
	GetBTTaskArg2(frame, int, memoIndex);
	fillTermSafe(recTerm, memoIndex);
	doMemo = NO;
      }

      //
      GetBTNextTaskType(frame, nt);
      if (nt == BT_spointer) {
	CrazyDebug(incDebugNODES(););
	GetBTNextTaskPtr1(frame, OZ_Term*, spointer);
	*spointer = recTerm;
	DiscardBT2Frames(frame);

	//
	OZ_Term *args = rec->getRef();
	// put tasks in reverse order (since subtrees will appear in
	// the normal order):
	EnsureBTSpace(frame, arity);
	while (arity-- > 0) {
	  PutBTTaskPtr(frame, BT_spointer, args++);
	}
      } else {
	//
	ReplaceBTTask1stArg(frame, BT_buildValue, recTerm);
	//
	OZ_Term *args = rec->getRef();
	EnsureBTSpace(frame, arity);
	arity--;
	PutBTTaskPtr(frame, BT_spointer_iterate, args++);
	while (arity-- > 0) {
	  PutBTTaskPtr(frame, BT_spointer, args++);
	}
      }
      break;
    }

  case BT_takeRecordLabel:
    ReplaceBTTask1stArg(frame, BT_takeRecordArity, value);
    break;

  case BT_takeRecordLabelMemo:
    ReplaceBTTask1stArg(frame, BT_takeRecordArityMemo, value);
    break;

  case BT_takeRecordArity:
    ReplaceBTTask2ndArg(frame, BT_makeRecordSync, value);
    break;

  case BT_takeRecordArityMemo:
    ReplaceBTTask2ndArg(frame, BT_makeRecordMemoSync, value);
    break;

  case BT_makeRecordMemoSync:
    doMemo = OK;
    // fall through;

  case BT_makeRecordSync:
    {
      Assert(value == (OZ_Term) 0);
      GetBTTaskArg1(frame, OZ_Term, label);
      GetBTTaskArg2(frame, OZ_Term, arity);
      //
      arity = packlist(arity);
      OZ_Term sortedArity = arity;
      if (!isSorted(arity)) {
	int arityLen;
	TaggedRef aux = duplist(arity, arityLen);
	sortedArity = sortlist(aux, arityLen);
      }
      //
      SRecord *rec =
	SRecord::newSRecord(label, aritytable.find(sortedArity));
      OZ_Term recTerm = makeTaggedSRecord(rec);
      if (doMemo) {
	GetNextBTFrameArg1(frame, int, memoIndex);
	fillTermSafe(recTerm, memoIndex);
	doMemo = NO;
      }
      DiscardBT2Frames(frame); 

      //
      GetBTTaskType(frame, nt);
      // An optimization (for the most frequent case?): 
      // if the record is just to be stored somewhere ('spointer'
      // task), then let's do it now:
      if (nt == BT_spointer) {
	CrazyDebug(incDebugNODES(););
	GetBTTaskPtr1(frame, OZ_Term*, spointer);
	*spointer = recTerm;
	DiscardBTFrame(frame);

	//
	while (oz_isCons(arity)) {
	  EnsureBTSpace1Frame(frame);
	  PutBTTaskPtrArg(frame, BT_recordArg, rec, oz_head(arity));
	  arity = oz_tail(arity);
	}
      } else {
	//
	// The last 'iterate' task will restore 'value' to the record
	// and iterate to the (non-spointer) task that handles it:
	EnsureBTSpace1Frame(frame);
	PutBTTaskPtrArg(frame, BT_recordArg_iterate, rec, oz_head(arity));
	arity = oz_tail(arity);
	while (oz_isCons(arity)) {
	  EnsureBTSpace1Frame(frame);
	  PutBTTaskPtrArg(frame, BT_recordArg, rec, oz_head(arity));
	  arity = oz_tail(arity);
	}
      }

      //
      break;
    }

  case BT_recordArg:
    {
      GetBTTaskPtr1(frame, SRecord*, rec);
      GetBTTaskArg2(frame, OZ_Term, fea);
      DiscardBTFrame(frame);
      rec->setFeature(fea, value);
      break;
    }

  case BT_recordArg_iterate:
    {
      GetBTTaskPtr1(frame, SRecord*, rec);
      GetBTTaskArg2(frame, OZ_Term, fea);
      DiscardBTFrame(frame);
      rec->setFeature(fea, value);
      //
      CrazyDebug(incDebugNODES(););
      value = makeTaggedSRecord(rec); // new value;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_dictKey:
    {
      // 'dict' remains in place:
      ReplaceBTTask2ndArg(frame, BT_dictVal, value);
      break;
    }

  case BT_dictVal:
    {
      GetBTTaskPtr1(frame, OzDictionary*, dict);
      GetBTTaskArg2(frame, OZ_Term, key);
      DiscardBTFrame(frame);
      dict->setArg(key, value);
      break;
    }

  case BT_fsetvalue:
    ReplaceBTTask2ndArg(frame, BT_fsetvalueSync, value);
    break;

  case BT_fsetvalueMemo:
    ReplaceBTTask2ndArg(frame, BT_fsetvalueMemoSync, value);
    break;

  case BT_fsetvalueMemoSync:
    doMemo = OK;

  case BT_fsetvalueSync:
    {
      Assert(value == (OZ_Term) 0);
      //
      GetBTTaskArg2(frame, OZ_Term, listRep);
      // will iterate to the task that handles value:
      makeFSetValue(listRep, &value);
      if (doMemo) {
	GetBTTaskArg1(frame, int, memoIndex);
	// The slot has been either accessed or not:
	fillTerm(value, memoIndex);
	doMemo = NO;
      }
      DiscardBTFrame(frame);

      //
      // 'value' now is the 'fsetvalue' we've just built. Let's just
      // go and do whatever is supposed to happen with it:
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_chunkMemo:
    doMemo = OK;
    // fall through;
  case BT_chunk:
    {
      Assert(oz_onToplevel());
      GetBTTaskPtr1(frame, GName*, gname);

      //
      OZ_Term chunkTerm = gname->getValue();
      if (chunkTerm) {
	SChunk* sc = tagged2SChunk(chunkTerm);
	if (!sc->getValue()) sc->import(value);
      } else {
	SChunk *sc = new SChunk(am.currentBoard(), 0);
	sc->setGName(gname);
	chunkTerm = makeTaggedConst(sc);
	overwriteGName(gname, chunkTerm);
	sc->import(value);
      }

      //
      if (doMemo) {
	GetBTTaskArg2(frame, int, memoIndex);
	fillTerm(chunkTerm, memoIndex);
	doMemo = NO;
      }

      //
      value = chunkTerm;
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_classFeatures:
    {
      Assert(oz_isSRecord(value));
      GetBTTaskPtr1(frame, OzClass*, cl);
      GetBTTaskArg2(frame, int, flags);
      DiscardBTFrame(frame);

      //
      SRecord *feat = tagged2SRecord(value);
      TaggedRef ff = feat->getFeature(NameOoFeat);
      //
      cl->import(value,
		 feat->getFeature(NameOoFastMeth),
		 oz_isSRecord(ff) ? ff : makeTaggedNULL(),
		 feat->getFeature(NameOoDefaults),
		 flags);

      //
      // Observe: it can overwrite the gname's binding even without
      // any lazy protocols, just due to the concurrency & suspendable
      // unmarshaling!! The same holds of course for other "named"
      // data structures, notably - objects and procedures.
      GName *gn = cl->getGName();
      OZ_Term ct = makeTaggedConst(cl);
      overwriteGName(gn, ct);
      gn->gcOn();
      //
      value = makeTaggedConst(cl);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_takeObjectLock:
    ReplaceBTTask1stArg(frame, BT_takeObjectState, value);
    break;

  case BT_takeObjectLockMemo:
    ReplaceBTTask1stArg(frame, BT_takeObjectStateMemo, value);
    break;

  case BT_takeObjectState:
    ReplaceBTTask2ndArg(frame, BT_makeObject, value);
    break;

  case BT_takeObjectStateMemo:
    ReplaceBTTask2ndArg(frame, BT_makeObjectMemo, value);
    break;

  case BT_makeObjectMemo:
    doMemo = OK;
  case BT_makeObject:
    {
      GetBTTaskArg1(frame, OZ_Term, lockTerm);
      GetBTTaskArg2(frame, OZ_Term, state);
      DiscardBTFrame(frame);
      //
      GetBTFramePtr1(frame, GName*, gname);

      // 'value' is the free record (incompete by now, - see also
      // gentraverserLoop.cc, case Co_Object):
      SRecord *feat = oz_isNil(value) ?
	(SRecord *) NULL : tagged2SRecord(value);
      OzLock *lock = oz_isNil(lockTerm) ? 
	(OzLock *) NULL : (OzLock *) tagged2Const(lockTerm);
      OzObject *o = new OzObject(oz_rootBoard(), gname, state, feat, lock);
      OZ_Term objTerm = makeTaggedConst(o);
      overwriteGName(gname, objTerm);
      if (doMemo) {
	GetBTFrameArg2(frame, int, memoIndex);
	fillTerm(objTerm, memoIndex);
	doMemo = NO;
      }
      DiscardBTFrame(frame);

      //
      value = objTerm;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_procFile:
    // 'value' cannot reference the procedure itself (since the
    // procedure is not created&remembered yet);
    ReplaceBTTask1stArg(frame, BT_proc, value);
    break;

  case BT_procFileMemo:
    ReplaceBTTask1stArg(frame, BT_procMemo, value);
    break;

  case BT_procMemo:
    doMemo = OK;
    // fall through;
  case BT_proc:
    {
      OZ_Term name = value;
      GetBTTaskArg1(frame, OZ_Term, file);
      DiscardBTFrame(frame);
      GetBTFramePtr1(frame, GName*, gname);
      GetBTFramePtr2(frame, ProgramCounter, pc);
      GetBTFrameArg3(frame, int, maybeMemoIndex);
      DiscardBTFrame(frame);
      GetBTFrameArg1(frame, int, maxX);
      GetBTFrameArg2(frame, int, line);
      GetBTFrameArg3(frame, int, column);
      DiscardBTFrame(frame);
      GetBTFrameArg1(frame, int, arity);
      GetBTFrameArg2(frame, int, gsize);
      DiscardBTFrame(frame);

      //
      Assert(gname);		// must be an unknown procedure here;
      OZ_Term procTerm;
      // kost@ : 'flags' are obviously not used (otherwise something
      // would not work: flags are not passed as e.g. 'file' is);
      PrTabEntry *pr = new PrTabEntry(name, mkTupleWidth(arity),
				      file, line, column,
				      oz_nil(), maxX);
      Assert(pc != NOCODE && gsize >= 0);
      pr->setPC(pc);
      pr->setGSize(gsize);

      //
      Abstraction *pp = Abstraction::newAbstraction(pr, am.currentBoard());
      procTerm = makeTaggedConst(pp);
      pp->setGName(gname);
      overwriteGName(gname, procTerm);

      //
      if (doMemo) {
	fillTerm(procTerm, maybeMemoIndex);
	doMemo = NO;
      }

      //
      if (gsize > 0) {
	// reverse order... and don't bother with 'spointer' tasks:
	// just issue an '_iterate' task;
	EnsureBTSpace(frame, gsize);
	PutBTTaskPtrArg(frame, BT_closureElem_iterate, pp, 0);
	for (int i = 1; i < gsize; i++) {
	  PutBTTaskPtrArg(frame, BT_closureElem, pp, i);
	}
	break;			// BT_proc:
      } else {
	value = makeTaggedConst(pp);
	GetBTTaskTypeNoDecl(frame, type);
	goto repeat;
      }

      //
      // (code area is done by the user himself;)
      Assert(0);
    }

  case BT_closureElem:
    {
      GetBTTaskPtr1(frame, Abstraction*, pp);
      GetBTTaskArg2(frame, int, ind);
      DiscardBTFrame(frame);
      pp->initG(ind, value);
      break;
    }

  case BT_closureElem_iterate:
    {
      GetBTTaskPtr1(frame, Abstraction*, pp);
      GetBTTaskArg2(frame, int, ind);
      DiscardBTFrame(frame);
      pp->initG(ind, value);
      //
      CrazyDebug(incDebugNODES(););
      value = makeTaggedConst(pp);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

    //
    // 'BT_abstractEntity' is decomposed by 'buildAbstractEntity':
  case BT_abstractEntity:
    OZ_error("Builder: never fetch BT_abstractEntity by 'buildValue?!");
    break;

  //
  case BT_binary:
    {
      GetBTTaskPtr1(frame, GTAbstractEntity*, arg);
      Assert(arg == 0);
      DiscardBTFrame(frame);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_binary_getValue:
    ReplaceBTTask1stArg(frame, BT_binary_getValueSync, value);
    break;

  case BT_binary_getValueSync:
    {
      Assert(value == (OZ_Term) 0);
      GetBTTaskArg1(frame, OZ_Term, ozValue);
      DiscardBTFrame(frame);
      GetBTFramePtr1(frame, OzValueProcessor, proc);
      GetBTFramePtr2(frame, GTAbstractEntity*, arg);
      DiscardBTFrame(frame);
      //
      (*proc)(arg, ozValue);

      //
      break;
    }

  case BT_binary_doGenAction_intermediate:
    {
      GetBTTaskPtr1(frame, BuilderGenAction, proc);
      GetBTTaskPtr2(frame, void*, arg);
      DiscardBTFrame(frame);
      //
      (*proc)(arg);

      //
      // Note that 'value' is preserved;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_nop:
    Assert(value == (OZ_Term) 0);
    DiscardBTFrame(frame);
    break;

  default:
    OZ_error("Builder: unknown task!");
  }

  //
  SetBTFrame(frame);
}

//
BTFrame* Builder::liftTask(int sz)
{
  GetBTFrame(frame);
  BTFrame *newTop = frame + bsFrameSize*sz;
  SetBTFrame(newTop);

  //
  // Iterate until a non-'goto iterate' task is found:
 repeat:
  GetBTTaskType(frame, type);
  switch (type) {

    // single frame:
  case BT_spointer:
  case BT_makeTupleMemo:
  case BT_makeTuple:
  case BT_recordArg:
  case BT_dictKey:
  case BT_dictVal:
  case BT_fsetvalue:
  case BT_fsetvalueMemo:
  case BT_closureElem:
  case BT_abstractEntity:
  case BT_nop:
    CopyBTFrame(frame, newTop);
    break;

    // single frame iterate:
  case BT_binary:
    { 
      DebugCode(GetBTTaskPtr1(frame, void*, bp););
      Assert(bp == 0);
    }
  case BT_spointer_iterate:
  case BT_buildValue:
  case BT_recordArg_iterate:
  case BT_fsetvalueMemoSync:
  case BT_fsetvalueSync:
  case BT_chunkMemo:
  case BT_chunk:
  case BT_classFeatures:
  case BT_closureElem_iterate:
  case BT_binary_doGenAction_intermediate:
    CopyBTFrame(frame, newTop);
    goto repeat;

    // two frames:
  case BT_takeRecordLabel:
  case BT_takeRecordLabelMemo:
  case BT_takeRecordArity:
  case BT_takeRecordArityMemo:
  case BT_makeRecordMemoSync:
  case BT_makeRecordSync:
  case BT_takeObjectLock:
  case BT_takeObjectLockMemo:
  case BT_takeObjectState:
  case BT_takeObjectStateMemo:
  case BT_binary_getValue:
  case BT_binary_getValueSync:
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    break;

    // two frames iterate:
  case BT_makeObjectMemo:
  case BT_makeObject:
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    goto repeat;

    // four frames:
  case BT_procFile:
  case BT_procFileMemo:
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    CopyBTFrame(frame, newTop);
    break;

    // four frames a'la procMemo:
  case BT_procMemo:
  case BT_proc:
    {
      CopyBTFrame(frame, newTop);
      CopyBTFrame(frame, newTop);
      CopyBTFrame(frame, newTop);
      GetBTFrameArg2(frame, int, gsize);
      CopyBTFrame(frame, newTop);
      if (gsize > 0)
	break;
      else
	goto repeat;
    }

  default:
    OZ_error("Builder: unknown task!");
  }

  //
  return (frame);
}

//
BTFrame* Builder::findBinary(BTFrame *frame)
{
  void *bp;
  //
  // Iterate until a non-'goto iterate' task is found:
 repeat:
  GetBTTaskType(frame, type);
  switch (type) {

    // found:
  case BT_binary:
    GetBTTaskPtr1NoDecl(frame, void*, bp);
    if (bp)
      break;
    // else fall through;

    // single frame:
  case BT_spointer:
  case BT_makeTupleMemo:
  case BT_makeTuple:
  case BT_recordArg:
  case BT_dictKey:
  case BT_dictVal:
  case BT_fsetvalue:
  case BT_fsetvalueMemo:
  case BT_closureElem:
  case BT_abstractEntity:
  case BT_nop:
    // single frame iterate:
  case BT_spointer_iterate:
  case BT_buildValue:
  case BT_recordArg_iterate:
  case BT_fsetvalueMemoSync:
  case BT_fsetvalueSync:
  case BT_chunkMemo:
  case BT_chunk:
  case BT_classFeatures:
  case BT_closureElem_iterate:
  case BT_binary_doGenAction_intermediate:
    NextBTFrame(frame);
    goto repeat;

    // two frames:
  case BT_takeRecordLabel:
  case BT_takeRecordLabelMemo:
  case BT_takeRecordArity:
  case BT_takeRecordArityMemo:
  case BT_makeRecordMemoSync:
  case BT_makeRecordSync:
  case BT_takeObjectLock:
  case BT_takeObjectLockMemo:
  case BT_takeObjectState:
  case BT_takeObjectStateMemo:
  case BT_makeObjectMemo:
  case BT_makeObject:
  case BT_binary_getValue:
  case BT_binary_getValueSync:
    NextBTFrame(frame);
    NextBTFrame(frame);
    goto repeat;

    // four frames:
  case BT_procFile:
  case BT_procFileMemo:
    // four frames a'la procMemo:
  case BT_procMemo:
  case BT_proc:
    NextBTFrame(frame);
    NextBTFrame(frame);
    NextBTFrame(frame);
    NextBTFrame(frame);
    goto repeat;

  default:
    OZ_error("Builder: unknown task!");
  }

  //
  return (frame);
}

#if defined(DEBUG_CHECK)

//
static OZ_Term fillNode = 0;
static OZ_Term uFillNode = 0;

//
// Fill the holes in the term such that it could be 'toC'ed.
void Builder::dbgWrap()
{
  //
  GetBTFrame(frame);
  //
  if (!((int) fillNode)) {
#ifdef DEBBUG_CHECK
    // a "unique name" does not need to be collected, so it is
    // initialized only once. Should that change, the bug will be easy
    // to discover;
    fillNode = oz_uniqueName("fill node");
#else
    fillNode = makeTaggedSmallInt(-7);
#endif
    // For this purpose it's good enough:
    uFillNode = fillNode;
  }

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
	*spointer = uFillNode;	// maybe also 'result' and 'blackhole';

	//
	NextBTFrame(frame);
	break;			// case;
      }

      //
      // 'buildValue' task keeps a value that is to be used by the
      // next task, so let's GC it:
    case BT_buildValue:
      NextBTFrame(frame);
      break;			// case;

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
      NextBT2Frames(frame);
      break;			// case;

      //
      // 'makeRecordSync' holds both label and arity list:
    case BT_makeRecordSync:
    case BT_makeRecordMemoSync:
      //
      // no optimizations for following 'spointer' task, etc.
      NextBT2Frames(frame);
      break;

      //
      // 'recordArg' holds a reference to the corresponding record and
      // a feature name, which both must be updated. Holes are to be
      // filled as well:
    case BT_recordArg:
    case BT_recordArg_iterate:
      {
	GetBTTaskPtr1(frame, SRecord*, rec);
	//
	OZ_Term &fea = GetBTTaskArg2Ref(frame, OZ_Term);
	//
	rec->setFeature(fea, fillNode);
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
      NextBTFrame(frame);
      break;			// case;

      //
    case BT_dictVal:
      NextBTFrame(frame);
      break;			// case;

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
      NextBTFrame(frame);
      break;			// case;

      //
      // GName is hanging around:
    case BT_chunk:
    case BT_chunkMemo:
      NextBTFrame(frame);
      break;			// case;

      //
    case BT_takeObjectLock:
    case BT_takeObjectLockMemo:
      NextBT2Frames(frame);
      break;			// case;

      //
    case BT_takeObjectState:
    case BT_takeObjectStateMemo:
      NextBT2Frames(frame);
      break;			// case;

      //
    case BT_makeObject:
    case BT_makeObjectMemo:
      NextBT2Frames(frame);
      break;

      //
      // 'OzClass' is already there:
    case BT_classFeatures:
      NextBTFrame(frame);
      break;			// case;

      //
      // yet just GName"s:
    case BT_procFile:
    case BT_procFileMemo:
      NextBTFrame(frame);
      NextBT3Frames(frame);
      break;			// case;

      // file name is there:
    case BT_proc:
    case BT_procMemo:
      NextBTFrame(frame);
      NextBT3Frames(frame);
      break;			// case;

      //
      // 'Abstraction*' held in 'closureElem' must be updated:
    case BT_closureElem:
    case BT_closureElem_iterate:
      NextBTFrame(frame);
      break;			// case;

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
      NextBTFrame(frame);
      break;

      //
      // 'binary_getValue' holds only a (stateless) binary area
      // processor and its (again stateless) argument;
    case BT_binary_getValue:
      NextBT2Frames(frame);
      break;			// case;

      //
      // ... but 'binary_getValueSync' holds a value, which at that
      // point is already well-formed (without holes):
    case BT_binary_getValueSync:
      NextBT2Frames(frame);
      break;			// case;

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
}
#endif // DEBUG_CHECK
