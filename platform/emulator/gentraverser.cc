/*
 *  Authors:
 *    Kostja Popov (kost@sics.se)
 *    Per Brand (perbrand@sics.se)
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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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

//
// kost@ : there is no 'fsetcore.hh';
extern void makeFSetValue(OZ_Term,OZ_Term*);

//
// A different one because of effeciency reasons. A 'ProcessNodeProc' 
// could well be supplied...
void GenTraverser::doit()
{
  while (!isEmpty()) {
    OZ_Term t = get();
    // a push-pop pair for the topmost entry is saved:
  bypass:
    DebugCode(debugNODES++;);
    CrazyDebug(debugNODES++;);
    DEREF(t, tPtr, tTag);

    //
    int ind = find(t);
    if (ind >= 0) {
      (void) processRepetition(t, ind);
      continue;
    }

    //
    switch(tTag) {

    case SMALLINT:
      processSmallInt(t);
      break;

    case OZFLOAT:
      processFloat(t);
      break;

    case LITERAL:
      processLiteral(t);
      break;

    case LTUPLE:
      if (!processLTuple(t)) {
	LTuple *l = tagged2LTuple(t);
	ensureFree(1);
	put(l->getTail());
	t = l->getHead();
	goto bypass;
      }
      break;

    case SRECORD:
      if (!processSRecord(t)) {
	SRecord *rec = tagged2SRecord(t);
	TaggedRef label = rec->getLabel();
	int argno = rec->getWidth();

	// 
	// The order is: label, [arity], subrees...
	// Both tuple and record args appear in a reverse order;
	ensureFree(argno+2);	// pessimistic approximation;
	for(int i = 0; i < argno; i++)
	  put(rec->getArg(i));
	if (!rec->isTuple())
	  put(rec->getArityList());
	t = rec->getLabel();
	goto bypass;
      }
      break;

  case EXT:
    processExtension(t);
    break;

  case OZCONST:
    {
      ConstTerm *ct = tagged2Const(t);
      switch (ct->getType()) {

      case Co_BigInt:
	processBigInt(t, ct);
	break;

      case Co_Dictionary:
	if (!processDictionary(t, ct)) {
	  OzDictionary *d = (OzDictionary *) ct;
	  int size = d->getSize();

	  // kost@ : what the hell is going on here???
	  int i = d->getFirst();
	  i = d->getNext(i);
	  // (pairs will be added on the receiver site in reverse order);
	  ensureFree(i+i);
	  while(i>=0) {
	    put(d->getValue(i));
	    put(d->getKey(i));
	    i = d->getNext(i);
	  }
	}
	break;

      case Co_Builtin:
	processBuiltin(t, ct);
	break;

      case Co_Chunk:
	if (!processChunk(t, ct)) {
	  SChunk *ch = (SChunk *) ct;
	  t = ch->getValue();
	  goto bypass;
	}
	break;

      case Co_Class:
	if (!processClass(t, ct)) {
	  ObjectClass *cl = (ObjectClass *) ct;
	  SRecord *fs = cl->getFeatures();
	  if (fs)
	    t = makeTaggedSRecord(fs);
	  else
	    t = oz_nil();
	  goto bypass;
	}
	break;

      case Co_Abstraction:
	if (!processAbstraction(t, ct)) {
	  Abstraction *pp = (Abstraction *) t;
	  int gs = pp->getPred()->getGSize();
	  ensureFree(gs);
	  for (int i=0; i < gs; i++)
	    put(pp->getG(i));
	}
	break;

      case Co_Object:
	processBuiltin(t, ct);
	break;

      case Co_Lock:
	processLock(t, (Tertiary *) ct);
	break;
      case Co_Cell:
	processCell(t, (Tertiary *) ct);
	break;
      case Co_Port:
	processPort(t, (Tertiary *) ct);
	break;
      case Co_Resource:
	processResource(t, (Tertiary *) ct);
	break;

      default:
	processNoGood(t);
	break;
      }
      break;
    }

    case FSETVALUE:
      if (!processFSETValue(t)) {
	OZ_FSetValue* fsetval = tagged2FSetValue(t);
	t = fsetval->getKnownInList();
	goto bypass;
      }
      break;

    case UVAR:
      processUVar(t);
      break;
    case CVAR:
      processCVar(t);
      break;

    default:
      processNoGood(t);
      break;
    }
  }
}

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
  switch(type) {

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
      DebugCode(debugNODES++;);
      CrazyDebug(debugNODES++;);
      DiscardBTFrame(frame);
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
	set(recTerm, memoIndex);
	doMemo = NO;
      }

      //
      GetNextTaskType(frame, nt);
      if (nt == BT_spointer) {
	DebugCode(debugNODES++;);
	CrazyDebug(debugNODES++;);
	GetNextTaskPtr1(frame, OZ_Term*, spointer);
	*spointer = recTerm;
	DiscardBT2Frames(frame);

	//
	OZ_Term *args = rec->getRef();
	EnsureBTSpace(frame, arity);
	while(arity-- > 0) {
	  PutBTTaskPtr(frame, BT_spointer, args++);
	}
      } else {
	//
	ReplaceTask1stArg(frame, BT_buildValue, recTerm);

	//
	OZ_Term *args = rec->getRef();
	// put tasks in reverse order (since subtrees will appear in
	// the normal order):
	args = args;		// after the last one;
	EnsureBTSpace(frame, arity);
	arity--;
	PutBTTaskPtr(frame, BT_spointer_iterate, args++);
	while(arity-- > 0)
	  PutBTTaskPtr(frame, BT_spointer, args++);
      }
      break;
    }

  case BT_takeRecordLabel:
    {
      ReplaceTask1stArg(frame, BT_takeRecordArity, value);
      break;
    }

  case BT_takeRecordLabelMemo:
    {
      ReplaceTask1stArg(frame, BT_takeRecordArityMemo, value);
      break;
    }

  case BT_takeRecordArity:
    {
      ReplaceTask2ndArg(frame, BT_makeRecord_intermediate, value);
      break;
    }

  case BT_takeRecordArityMemo:
    {
      ReplaceTask2ndArg(frame, BT_makeRecordMemo_intermediate, value);
      break;
    }

  case BT_makeRecordMemo_intermediate:
    doMemo = OK;
    // fall through;

  case BT_makeRecord_intermediate:
    {
      GetBTTaskArg1(frame, OZ_Term, label);
      GetBTTaskArg2(frame, OZ_Term, arity);

      //
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
	set(recTerm, memoIndex);
	doMemo = NO;
      }
      DiscardBT2Frames(frame);

      //
      GetBTTaskType(frame, nt);
      if (nt == BT_spointer) {
	DebugCode(debugNODES++;);
	CrazyDebug(debugNODES++;);
	GetBTTaskPtr1(frame, OZ_Term*, spointer);
	*spointer = recTerm;
	DiscardBTFrame(frame);

	//
	while (oz_isCons(arity)) {
	  EnsureBTSpace1Frame(frame);
	  PutBTTask2Args(frame, BT_recordArg, rec, oz_head(arity));
	  arity = oz_tail(arity);
	}
      } else {
	//
	PutBTTaskPtrArg(frame, BT_recordArg_iterate, rec, oz_head(arity));
	arity = oz_tail(arity);
	while (oz_isCons(arity)) {
	  EnsureBTSpace1Frame(frame);
	  PutBTTaskPtrArg(frame, BT_recordArg, rec, oz_head(arity));
	  arity = oz_tail(arity);
	}
      }

      //
      // 'value' is preserved;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
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
      DebugCode(debugNODES++;);
      CrazyDebug(debugNODES++;);
      value = makeTaggedSRecord(rec);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_dictKey:
    {
      GetBTTaskPtr1(frame, OzDictionary*, dict);
      // 'dict' remains in place:
      ReplaceTask2ndArg(frame, BT_dictVal, value);
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
    {
      OZ_Term *ret;
      DiscardBTFrame(frame);
      makeFSetValue(value, ret);
      value = *ret;
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_fsetvalueMemo:
    {
      GetBTTaskArg1(frame, int, memoIndex);
      DiscardBTFrame(frame);
      OZ_Term *ret;
      makeFSetValue(value, ret);
      value = *ret;
      set(value, memoIndex);
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
      DiscardBTFrame(frame);

      //
      OZ_Term chunkTerm;
      SChunk *sc = new SChunk(am.currentBoard(), 0);
      sc->setGName(gname);
      chunkTerm = makeTaggedConst(sc);
      addGName(gname, chunkTerm);
      sc->import(value);

      //
      value = chunkTerm;
      if (doMemo) {
	GetBTTaskArg2(frame, int, memoIndex);
	set(value, memoIndex);
	doMemo = NO;
      }
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_classMemo:
    doMemo = OK;
    // fall through;
  case BT_class:
    {
      Assert(oz_isSRecord(value));
      GetBTTaskPtr1(frame, GName*, gname);
      DiscardBTFrame(frame);

      //      
      OZ_Term classTerm;
      ObjectClass *cl = new ObjectClass(NULL, NULL, NULL, NULL, NO, NO,
					am.currentBoard());
      cl->setGName(gname);
      classTerm = makeTaggedConst(cl);
      addGName(gname, classTerm);

      //
      SRecord *feat = tagged2SRecord(value);
      TaggedRef ff = feat->getFeature(NameOoUnFreeFeat);
      // RALF IS WORKING HERE
#if 0
      Bool locking = oz_isTrue(oz_deref(feat->getFeature(NameOoLocking)));
      cl->import(feat,
		 tagged2Dictionary(feat->getFeature(NameOoFastMeth)),
		 oz_isSRecord(ff) ? tagged2SRecord(ff) : (SRecord*)NULL,
		 tagged2Dictionary(feat->getFeature(NameOoDefaults)),
		 locking);

      //
#else
      OZ_warning("New Marshaler does not work for classes!");
#endif
      value = classTerm;
      if (doMemo) {
	GetBTTaskArg2(frame, int, memoIndex);
	set(value, memoIndex);
	doMemo = NO;
      }
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  case BT_procMemo:
    doMemo = OK;
    // fall through;
  case BT_proc:
    {
      Assert(oz_onToplevel());
      GetBTTaskPtr1(frame, GName*, gname);
      GetBTTaskArg2(frame, int, maybeMemoIndex);
      DiscardBTFrame(frame);
      GetBTFrameArg1(frame, int, arity);
      GetBTFrameArg2(frame, int, gsize);
      GetBTFrameArg3(frame, int, maxX);
      DiscardBTFrame(frame);
      GetBTFramePtr1(frame, ProgramCounter, pc);
      DiscardBTFrame(frame);
      OZ_Term name  = value;
      OZ_Term procTerm;

      //
      PrTabEntry *pr = new PrTabEntry(name, mkTupleWidth(arity), 0,0,0,
				      oz_nil(), maxX);
      pr->setGSize(gsize);
      Abstraction *pp = Abstraction::newAbstraction(pr, am.currentBoard());
      procTerm = makeTaggedConst(pp);
      pp->setGName(gname);
      addGName(gname, procTerm);
      pr->PC = pc;
      pr->patchFileAndLine();

      //
      if (doMemo) {
	set(value, maybeMemoIndex);
	doMemo = NO;
      }

      //
      if (gsize > 0) {
	// reverse order...
	EnsureBTSpace(frame, gsize);
	PutBTTaskPtrArg(frame, BT_closureElem_iterate, pp, 0);
	for (int i = 1; i < gsize; i++) {
	  PutBTTaskPtrArg(frame, BT_closureElem, pp, i);
	}
	//
	break;
      } else {
	// no args;
	value = procTerm;
	GetBTTaskTypeNoDecl(frame, type);
	goto repeat;
      }
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
      DebugCode(debugNODES++;);
      CrazyDebug(debugNODES++;);
      value = makeTaggedConst(pp);
      GetBTTaskTypeNoDecl(frame, type);
      goto repeat;
    }

  default:
    OZ_error("Builder: unknown task!");
  }

  //
  SetBTFrame(frame);
}
