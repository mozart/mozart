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

//
// A different one because of effeciency reasons. A 'ProcessNodeProc'
// could well be supplied...
void GenTraverser::doit()
{
  while (!isEmpty()) {
    OZ_Term t = get();
    // a push-pop pair for the topmost entry is saved:
  bypass:
    CrazyDebug(incDebugNODES(););
    DEREF(t, tPtr, tTag);

    //
    switch(tTag) {

    case SMALLINT:
      processSmallInt(t);
      break;

    case OZFLOAT:
      processFloat(t);
      break;

    case LITERAL:
      {
        int ind = find(t);
        if (ind >= 0) {
          (void) processRepetition(ind);
          continue;
        }
        processLiteral(t);
        break;
      }

    case LTUPLE:
      {
        int ind = find(t);
        if (ind >= 0) {
          (void) processRepetition(ind);
          continue;
        }

        //
        if (!processLTuple(t)) {
          LTuple *l = tagged2LTuple(t);
          ensureFree(1);
          put(l->getTail());
          t = l->getHead();
          goto bypass;
        }
        break;
      }

    case SRECORD:
      {
        int ind = find(t);
        if (ind >= 0) {
          (void) processRepetition(ind);
          continue;
        }

        //
        if (!processSRecord(t)) {
          SRecord *rec = tagged2SRecord(t);
          TaggedRef label = rec->getLabel();
          int argno = rec->getWidth();

          //
          // The order is: label, [arity], subtrees...
          // Both tuple and record args appear in a reverse order;
          ensureFree(argno+1);  // pessimistic approximation;
          for(int i = 0; i < argno; i++)
            put(rec->getArg(i));
          if (!rec->isTuple())
            put(rec->getArityList());
          t = rec->getLabel();
          goto bypass;
        }
        break;
      }

  case EXT:
    processExtension(t);
    break;

  case OZCONST:
    {
      int ind = find(t);
      if (ind >= 0) {
        (void) processRepetition(ind);
        continue;
      }

      //
      ConstTerm *ct = tagged2Const(t);
      switch (ct->getType()) {

      case Co_BigInt:
        processBigInt(t, ct);
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
        processObject(t, ct);
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
        processNoGood(t,OK);
        break;
      }
      break;
    }

    case FSETVALUE:
      if (!processFSETValue(t)) {
        t = tagged2FSetValue(t)->getKnownInList();
        goto bypass;
      }
      break;

    case UVAR:
      processUVar(tPtr);
      break;

    case CVAR:
      {
        int ind = find(t);
        if (ind >= 0) {
          (void) processRepetition(ind);
          continue;
        }

        //
        OZ_Term value;
        if ((value = processCVar(tPtr))) {
          t = value;
          goto bypass;
        }
        break;
      }

    case GCTAG:
      {
        // If the argument is zero then the task is empty:
        void *arg = getPtr();

        //
        if (arg) {
          MarshalerBinaryAreaProcessor proc =
            (MarshalerBinaryAreaProcessor) lookupPtr();
          // 'proc' is preserved in stack;
          StackEntry *se = putPtrSERef(0);
          putInt(taggedBATask); // pre-cooked;

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
          dropEntry();          // 'proc';
        }
        break;
      }

    default:
      processNoGood(t,NO);
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

// for buildValueOutline see unmarshaling.cc
