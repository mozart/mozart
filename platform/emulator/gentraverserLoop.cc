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

//
// This file is supposed to be included in other marshaling files
// (pickling, resource excavation, distribution marshaling, etc.);
//
// 'TRAVERSERCLASS' must be defined for a particular class name;

//
// A different one because of effeciency reasons. A 'ProcessNodeProc' 
// could well be supplied...
void TRAVERSERCLASS::doit()
{
  Assert(!isEmpty());
  register OZ_Term t = get();
  register OZ_Term *tPtr;
  DebugCode(tPtr = (OZ_Term *) -1;);

  while (1) {
    CrazyDebug(incDebugNODES(););
    //
    switch (tagged2ltag(t)) {

    case LTAG_REF00:
    case LTAG_REF10:
    case LTAG_REF01:
    case LTAG_REF11:
      tPtr = tagged2Ref(t);
      t = *tPtr;
      continue;

    case LTAG_SMALLINT:
      processSmallInt(t);
      break;

    case LTAG_LITERAL:
      processLiteral(t);
      break;

    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
      if (!processLTuple(t)) {
	// if we did the node, we have not suspended:
	Assert(tosNotRunning == (StackEntry *) 0);
	LTuple *l = tagged2LTuple(t);
	ensureFree(1);
	put(l->getTail());
	t = l->getHead();
	continue;
      }
      break;

    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      if (!processSRecord(t)) {
	Assert(tosNotRunning == (StackEntry *) 0);
	SRecord *rec = tagged2SRecord(t);
	TaggedRef label = rec->getLabel();
	int argno = rec->getWidth();

	// 
	// The order is: label, [arity], subtrees...
	// Both tuple and record args appear in a reverse order;
	ensureFree(argno+2);	// pessimistic approximation;
	for(int i = 0; i < argno; i++)
	  put(rec->getArg(i));
	if (!rec->isTuple()) {
	  putSync();		// will appear after the arity list;
	  put(rec->getArityList());
	}
	t = rec->getLabel();
	continue;
      }
      break;

    case LTAG_CONST0:
    case LTAG_CONST1:
      {
	ConstTerm *ct = tagged2Const(t);
	switch (ct->getType()) {

	case Co_Extension:
	  processExtension(t);
	  break;

	case Co_Float:
	  processFloat(t);
	  break;

	case Co_BigInt:
	  processBigInt(t);
	  break;

	case Co_FSetValue:
	  if (!processFSETValue(t)) {
	    Assert(tosNotRunning == (StackEntry *) 0);
	    ensureFree(1);
	    putSync();		// will appear after the list;
	    t = tagged2FSetValue(t)->getKnownInList();
	    continue;
	  }
	  break;

	case Co_Dictionary:
	  if (!processDictionary(t, ct)) {
	    OzDictionary *d = (OzDictionary *) ct;
	    int size = d->getSize();
	    if (size > 0) {
	      DictNode *nodes = d->pairsInArray();
	      DictNode *p = nodes;
	      //
	      ensureFree(size*2);
	      for ( ; size--; p++) {
		put(p->getValue());
		put(p->getKey());
	      }
	      delete [] nodes;
	    }
	  }
	  break;

	case Co_Array:
	  if (!processArray(t, ct)) {
	    OzArray *array = (OzArray *) ct;
	    ensureFree(array->getWidth());
	    for (int i = array->getHigh(); i >= array->getLow(); i--)
	      put(array->getArg(i));
	  }
	  break;

	case Co_Builtin:
	  processBuiltin(t, ct);
	  break;

	case Co_Chunk:
	  if (!processChunk(t, ct)) {
	    Assert(tosNotRunning == (StackEntry *) 0);
	    SChunk *ch = (SChunk *) ct;
	    t = ch->getValue();
	    continue;
	  }
	  break;

	case Co_Class:
	  if (!processClass(t, ct)) {
	    Assert(tosNotRunning == (StackEntry *) 0);
	    OzClass *cl = (OzClass *) ct;
	    SRecord *fs = cl->getFeatures();
	    t = fs ? makeTaggedSRecord(fs) : oz_nil();
	    continue;
	  }
	  break;

	case Co_Abstraction:
	  if (!processAbstraction(t, ct)) {
	    Abstraction *pp = (Abstraction *) ct;
	    Assert(pp->isComplete());
	    int gs = pp->getPred()->getGSize();
	    //
	    // in the stream: file, name, registers, code area:
	    ensureFree(gs+2);
	    for (int i=0; i < gs; i++)
	      put(pp->getG(i));
	    //
	    // OPTIMIZATION: does not need a 'sync' because the print
	    // name is a primitive value. And actually,
	    // PrTabEntry::PrTabEntry does not analyze that name
	    // either, but just needs a value OZ_Term.
	    put(pp->getName());
	    t = pp->getPred()->getFile();
	    continue;
	  }
	  break;

	case Co_Object:
	  if (!processObject(t, ct)) {
	    // raph: objects are no longer marshaled this way

	    //
	    OzObject *o = tagged2Object(t);

	    put(makeTaggedConst(o->getClass()));
	    //
	    SRecord *sr = o->getFreeRecord();
	    OZ_Term tsr;
	    if (sr)
	      tsr = makeTaggedSRecord(sr);
	    else
	      tsr = oz_nil();
	    // OPTIMIZATION: does not need a 'sync' because the free
	    // list is taken "as is" during object construction. That
	    // is, its content is not analyzed: all what is needed is
	    // a valid OZ_Term (in this case that's an STAG_LTUPLE
	    // one).
	    put(tsr);

	    //
	    //bmc: I should not put the state yet.
	    put(o->getStateTerm());

	    //
	    if (o->getLock())
	      t = makeTaggedConst(o->getLock());
	    else
	      t = oz_nil();
	    continue;
	  }
	  break;

	case Co_ObjectState: {
	  if (!processObjectState(t, ct)) {
	    ObjectState *s = (ObjectState *) ct;
	    t = s->getValueTerm();
	    continue;
	  }
	  break;
	}

	case Co_Lock:
	  processLock(t, ct);
	  break;

	case Co_Cell:
	  if (!processCell(t, ct) && !(static_cast<OzCell*>(ct)->isDistributed())) {
	    t = static_cast<OzCell*>(ct)->getValue();
	    continue;
	  }
	  break;

	case Co_Port:
	  processPort(t, ct);
	  break;
	case Co_Resource:
	  processResource(t, ct);
	  break;

	default:
	  processNoGood(t);
	  break;
	}
	break;
      }

    case LTAG_VAR0:
    case LTAG_VAR1:
      if (!processVar(t, tPtr)) {
	Assert(oz_isFailed(t));
	// now traverse the exception
	t = static_cast<Failed*>(tagged2Var(t))->getException();
	continue;
      }
      break;

    case LTAG_MARK0:
    case LTAG_MARK1:
      //
      switch (t) {
      case taggedBATask:
	{
	  // If the argument is zero then the task is empty:
	  GTAbstractEntity *arg = (GTAbstractEntity *) getPtr();

	  //
	  if (arg) {
	    TraverserBinaryAreaProcessor proc =
	      (TraverserBinaryAreaProcessor) lookupPtr();
	    Assert(proc > (TraverserBinaryAreaProcessor) 0xf);
	    // 'proc' is preserved in stack;
	    long pos = putPtrSERef(0);
	    putInt(taggedBATask);	// pre-cooked;

	    //
	    if (!(*proc)(this, arg)) {
	      // not yet done - restore the argument back;
	      updateSEPtr(pos, arg);
	    }
	    // ... otherwise do nothing: the empty task will be
	    // discarded later - 
	  } else {
	    CrazyDebug(decDebugNODES(););
	    // - here, to be exact:
	    dropEntry();		// 'proc';
	  }
	  break;
	}

      case taggedSyncTask:
	processSync();
	break;

      case taggedContTask:
	{
	  GTAbstractEntity *arg;
	  TraverserContProcessor proc;

	  //
	  arg = (GTAbstractEntity *) getPtr();
	  proc = (TraverserContProcessor) getPtr();
	  (*proc)(this, arg);
	  break;
	}
      }
      break;

    default:
      processNoGood(t);
      break;
    }

    //
    if (isEmpty()) {
      break;
    } else {
      t = get();
    }
  }
}
