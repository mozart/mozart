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
  while (!isEmpty()) {
    OZ_Term t = get();
    // a push-pop pair for the topmost entry is saved:
  bypass:
    register TaggedRef *tPtr;
    DebugCode(tPtr = (TaggedRef *) -1;);
    CrazyDebug(incDebugNODES(););

    //
  deref:
    switch (tagged2ltag(t)) {

    case LTAG_REF00:
    case LTAG_REF10:
    case LTAG_REF01:
    case LTAG_REF11:
      tPtr = tagged2Ref(t);
      t = *tPtr;
      goto deref;

    case LTAG_SMALLINT:
      processSmallInt(t);
      break;

    case LTAG_LITERAL:
      {
	int ind = findTerm(t);
	if (ind >= 0) {
	  processRepetition(t, tPtr, ind);
	} else {
	  processLiteral(t);
	}
	break;
      }

    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
      {
	int ind = findTerm(t);
	if (ind >= 0) {
	  processRepetition(t, tPtr, ind);
	  break;
	}

	//
	if (!processLTuple(t)) {
	  LTuple *l = tagged2LTuple(t);
	  ensureFree(1);
	  put(l->getTail());
	  if (!isEmpty()) {
	    t = l->getHead();
	    goto bypass;
	  } else {
	    put(l->getHead());
	  }
	}
	break;
      }

    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      {
	int ind = findTerm(t);
	if (ind >= 0) {
	  processRepetition(t, tPtr, ind);
	  break;
	}

	//
	if (!processSRecord(t)) {
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
	  if (!isEmpty()) {
	    t = rec->getLabel();
	    goto bypass;
	  } else {
	    put(rec->getLabel());
	  }
	}
	break;
      }

    case LTAG_CONST0:
    case LTAG_CONST1:
      {
	int ind = findTerm(t);
	if (ind >= 0) {
	  processRepetition(t, tPtr, ind);
	  break;
	}

	//
	ConstTerm *ct = tagged2Const(t);
	switch (ct->getType()) {

	case Co_Extension:
	  processExtension(t);
	  break;

	case Co_Float:
	  processFloat(t);
	  break;

	case Co_BigInt:
	  processBigInt(t, ct);
	  break;

	case Co_FSetValue:
	  if (!processFSETValue(t)) {
	    ensureFree(1);
	    putSync();		// will appear after the list;
	    if (!isEmpty()) {
	      t = tagged2FSetValue(t)->getKnownInList();
	      goto bypass;
	    } else {
	      put(tagged2FSetValue(t)->getKnownInList());
	    }
	  }
	  break;

	case Co_Dictionary:
	  if (!processDictionary(t, ct)) {
	    OzDictionary *d = (OzDictionary *) ct;
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

	case Co_Array:
	if (!processArray(t, ct)) {
	  OzArray *array = (OzArray *) ct;
	  ensureFree(array->getWidth());
	  for (int i = array->getHigh(); i >= array->getLow(); i--) {
	    put(array->getArg(i));
	  }
	}
	break;

	case Co_Builtin:
	  processBuiltin(t, ct);
	  break;

	case Co_Chunk:
	  if (!processChunk(t, ct)) {
	    SChunk *ch = (SChunk *) ct;
	    if (!isEmpty()) {
	      t = ch->getValue();
	      goto bypass;
	    } else {
	      put(ch->getValue());
	    }
	  }
	  break;

	case Co_Class:
	  if (!processClass(t, ct)) {
	    ObjectClass *cl = (ObjectClass *) ct;
	    SRecord *fs = cl->getFeatures();
	    if (!isEmpty()) {
	      t = fs ? makeTaggedSRecord(fs) : oz_nil();
	      goto bypass;
	    } else {
	      put(fs ? makeTaggedSRecord(fs) : oz_nil());
	    }
	  }
	  break;

	case Co_Abstraction:
	  {
	    if (!processAbstraction(t, ct)) {
	      Abstraction *pp = (Abstraction *) ct;
	      int gs = pp->getPred()->getGSize();
	      //
	      // in the stream: file, name, registers, code area:
	      ensureFree(gs+2);
	      for (int i=0; i < gs; i++)
		put(pp->getG(i));
	      //
	      put(pp->getName());
	      put(pp->getPred()->getFile());
	    }
	  }
	  break;

	case Co_Object:
	  if (!processObject(t, ct)) {
	    //
	    Object *o = (Object *) tagged2Const(t);

	    //
	    SRecord *sr = o->getFreeRecord();
	    OZ_Term tsr;
	    if (sr)
	      tsr = makeTaggedSRecord(sr);
	    else
	      tsr = oz_nil();
	    put(tsr);

	    //
	    put(makeTaggedConst(getCell(o->getState())));

	    //
	    OZ_Term tlck;
	    if (o->getLock())
	      tlck = makeTaggedConst(o->getLock());
	    else
	      tlck = oz_nil();
	    put(tlck);
	  }
	  break;

	case Co_Lock:
	  processLock(t, (Tertiary *) ct);
	  break;

	case Co_Cell:
	  if (!processCell(t, (Tertiary *) ct) && 
	      ((Tertiary *) ct)->isLocal()) {
	    t = ((CellLocal *) ct)->getValue();
	    goto bypass;
	  }
	  break;

	case Co_Port:
	  processPort(t, (Tertiary *) ct);
	  break;
	case Co_Resource:
	  processResource(t, (Tertiary *) ct);
	  break;

	default:
	  (void) processNoGood(t,OK);
	  break;
	}
	break;
      }

    case LTAG_VAR0:
    case LTAG_VAR1:
      {
	// Note: we remember locations of variables, - not the
	// variables themselves! This works, since values and
	// variables cannot co-reference.
	int ind = findVarLocation(tPtr);
	if (ind >= 0) {
	  processRepetition(t, tPtr, ind);
	  break;
	} else {
	  processVar(t, tPtr);
	  break;
	}
      }

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
	    StackEntry *se = putPtrSERef(0);
	    putInt(taggedBATask);	// pre-cooked;

	    //
	    if (!(*proc)(this, arg)) {
	      // not yet done - restore the argument back;
	      updateSEPtr(se, arg);
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
      (void) processNoGood(t,NO);
      break;
    }
  }
}
