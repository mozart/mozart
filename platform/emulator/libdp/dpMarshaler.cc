/* -*- C++ -*-
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Konstantin Popov <kost@sics.se>
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
 *
 *  Copyright:
 *    Per Brand, 1998
 *    Konstantin Popov, 1998-2000
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

#if defined(INTERFACE)
#pragma implementation "dpMarshaler.hh"
#endif

#include "base.hh"
#include "dpBase.hh"
#include "perdio.hh"
#include "msgType.hh"
#include "table.hh"
#include "dpMarshaler.hh"
#include "dpMarshalExt.hh"
#include "dpInterface.hh"
#include "var.hh"
#include "var_obj.hh"
#include "var_future.hh"
#include "var_class.hh"
#include "var_emanager.hh"
#include "var_eproxy.hh"
#include "var_gcstub.hh"
#include "gname.hh"
#include "state.hh"
#include "port.hh"
#include "dpResource.hh"
#include "boot-manager.hh"

#define RETURN_ON_ERROR(ERROR)             \
        if(ERROR) { (void) b->finish(); return 0; }

//
void dpmInit()
{
  dpmExtInit();
}

//
void DPMarshaler::processSmallInt(OZ_Term siTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  // The current term is not allowed to occupy the remaining space in
  // the buffer completely, but leave a space for 'DIF_SUSPEND';
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
    marshalSmallInt(bs, siTerm);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(siTerm);
  }
}

//
void DPMarshaler::processFloat(OZ_Term floatTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MFloatMaxSize) {
    marshalFloat(bs, floatTerm);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(floatTerm);
  }
}

//
// 'dpMarshalLitCont' marshals a string (which is passed in as 'arg')
// as long as the buffer allows. If the string does not fit
// completely, then marshaling suspends. Note that the string fragment
// looks exactly as a string!
//
static void dpMarshalLitCont(GenTraverser *gt, GTAbstractEntity *arg);
//
static inline
void dpMarshalString(ByteBuffer *bs, GenTraverser *gt,
                     DPMarshalerLitSusp *desc)
{
  const char *name = desc->getRemainingString();
  int nameSize = desc->getCurrentSize();
  int availSpace = bs->availableSpace() - MNumberMaxSize;
  Assert(availSpace >= 0);

  //
  int ms = min(nameSize, availSpace);
  marshalStringNum(bs, name, ms);
  // If it didn't fit completely, put the continuation back:
  if (ms < nameSize) {
    desc->incIndex(ms);
    gt->suspendAC(dpMarshalLitCont, desc);
  } else {
    delete desc;
  }
}

//
static void dpMarshalLitCont(GenTraverser *gt, GTAbstractEntity *arg)
{
  Assert(arg->getType() == GT_LiteralSusp);
  ByteBuffer *bs = (ByteBuffer *) gt->getOpaque();
  // we should advance:
  Assert(bs->availableSpace() > 2*DIFMaxSize + MNumberMaxSize);

  //
  marshalDIF(bs, DIF_LIT_CONT);
  dpMarshalString(bs, gt, (DPMarshalerLitSusp *) arg);
}

//
void DPMarshaler::processLiteral(OZ_Term litTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Literal *lit = tagged2Literal(litTerm);
  const char *name = lit->getPrintName();
  int nameSize = strlen(name);

  //
  // The name may not fit completely;
  if (bs->availableSpace() >=
      2*DIFMaxSize + 3*MNumberMaxSize + MGNameMaxSize) {
    int litTermInd = rememberTerm(litTerm);
    MarshalTag litTag;
    GName *gname = NULL;

    //
    if (lit->isAtom()) {
      litTag = DIF_ATOM;
    } else if (lit->isUniqueName()) {
      litTag = DIF_UNIQUENAME;
    } else if (lit->isCopyableName()) {
      litTag = DIF_COPYABLENAME;
    } else {
      litTag = DIF_NAME;
      gname = ((Name *) lit)->globalize();
    }

    //
    marshalDIF(bs, litTag);
    const char *name = lit->getPrintName();
    marshalTermDef(bs, litTermInd);
    marshalNumber(bs, nameSize);
    if (gname) marshalGName(bs, gname);

    //
    // Observe: the format is different from pickles!
    DPMarshalerLitSusp *desc = new DPMarshalerLitSusp(litTerm, nameSize);
    dpMarshalString(bs, this, desc);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(litTerm);
  }
}

//
void DPMarshaler::processBigInt(OZ_Term biTerm, ConstTerm *biConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  const char *crep = toC(biTerm);

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize + strlen(crep)) {
    marshalBigInt(bs, biTerm, biConst);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(biTerm);
  }
}

//
void DPMarshaler::processNoGood(OZ_Term resTerm, Bool trail)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= DIFMaxSize + MDistSPPMaxSize) {
    marshalSPP(bs, resTerm, trail);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(resTerm);
  }
}

//
void DPMarshaler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Builtin *bi= (Builtin *) biConst;
  const char *pn = bi->getPrintName();

  //
  if (bi->isSited()) {
    if (bs->availableSpace() >=
        MDistSPPMaxSize + MNumberMaxSize + DIFMaxSize) {
      processNoGood(biTerm, OK);
      rememberNode(this, bs, biTerm);
      return;
    }
  } else {
    if (bs->availableSpace() >=
        2*DIFMaxSize + MNumberMaxSize + strlen(pn)) {
      marshalDIF(bs, DIF_BUILTIN);
      rememberNode(this, bs, biTerm);
      marshalString(bs, pn);
      return;
    }
  }
  marshalDIF(bs, DIF_SUSPEND);
  suspend(biTerm);
}

//
void DPMarshaler::processExtension(OZ_Term t)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  OZ_Extension *oe = tagged2Extension(t);

  //
  if (bs->availableSpace() >=
      2*DIFMaxSize + MNumberMaxSize + dpMinNeededSpaceExt(oe)) {
    marshalDIF(bs, DIF_EXTENSION);
    marshalNumber(bs, oe->getIdV());
    //
    if (!dpMarshalExt(bs, this, t, oe)) {
      processNoGood(t, NO);     // not remembered!
    }
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(t);
  }
}

//
Bool DPMarshaler::processObject(OZ_Term term, ConstTerm *objConst)
{
  if (doToplevel)
    return (marshalFullObject(term, objConst));
  else
    return (marshalObjectStub(term, objConst));
}

// private methods;
Bool DPMarshaler::marshalObjectStub(OZ_Term term, ConstTerm *objConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Object *o = (Object*) objConst;
  Assert(isObject(o));

  //
  if (o->getClass()->isSited()) {
    if (bs->availableSpace() >=
        DIFMaxSize + MNumberMaxSize + MDistSPPMaxSize) {
      processNoGood(term, OK);
      rememberNode(this, bs, term);
      return (TRUE);
    }
  } else {
    if (bs->availableSpace() >=
        DIFMaxSize + MNumberMaxSize + MDistObjectMaxSize) {
      //
      Assert(o->getTertType() == Te_Local || o->getTertType() == Te_Manager);
      if (o->getTertType() == Te_Local)
        globalizeTert(o);

      //
      ObjectClass *oc = o->getClass();
      GName *gnclass = globalizeConst(oc, bs);
      Assert(gnclass);
      GName *gnobj = globalizeConst(o, bs);
      Assert(o->getGName1());
      Assert(gnobj);
      Assert(o->getTertType() == Te_Manager);
      // No "lazy class" protocol, so it isn't a tertiary:
      // Assert(oc->getTertType() == Te_Manager);
      //
      marshalOwnHead(bs, DIF_STUB_OBJECT, o->getIndex());

      //
      marshalGName(bs, gnobj);
      marshalGName(bs, gnclass);

      //
      rememberNode(this, bs, term);
      return (TRUE);
    }
  }
  marshalDIF(bs, DIF_SUSPEND);
  suspend(term);
  return (TRUE);
}

//
Bool DPMarshaler::marshalFullObject(OZ_Term term, ConstTerm *objConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Object *o = (Object*) objConst;
  Assert(isObject(o));
  Assert(o->getTertType() == Te_Manager);
  // Assert(o->getClass()->getTertType() == Te_Manager);

  //
  if (bs->availableSpace() >= 2*DIFMaxSize) {
    marshalDIF(bs, DIF_OBJECT);
    rememberNode(this, bs, term);
    marshalGName(bs, o->getGName1());
    doToplevel = FALSE;
  } else {
//      printf("suspend %d\n",osgetpid());
    marshalDIF(bs, DIF_SUSPEND);
    suspend(term);
    // 'doToplevel' is NOT reset here, since 'processObject' will be
    // re-applied when the marshaler is woken up!
  }
  return (FALSE);
}


//
#define HandleTert(string,tert,term,tag,check)          \
    ByteBuffer *bs = (ByteBuffer *) getOpaque();        \
    if (bs->availableSpace() >= DIFMaxSize +            \
               MTertiaryMaxSize + MNumberMaxSize) {     \
      marshalTertiary(bs, tert, tag);                   \
      rememberNode(this, bs, term);                     \
    } else {                                            \
      marshalDIF(bs, DIF_SUSPEND);                      \
      suspend(term);                                    \
    }

//
void DPMarshaler::processLock(OZ_Term term, Tertiary *tert)
{
  HandleTert("lock",tert,term,DIF_LOCK,OK);
}
Bool DPMarshaler::processCell(OZ_Term term, Tertiary *tert)
{
  HandleTert("cell",tert,term,DIF_CELL,OK);
  return (TRUE);
}
void DPMarshaler::processPort(OZ_Term term, Tertiary *tert)
{
  HandleTert("port",tert,term,DIF_PORT,NO);
}
void DPMarshaler::processResource(OZ_Term term, Tertiary *tert)
{
  HandleTert("resource",tert,term,DIF_RESOURCE_T,OK);
}

#undef HandleTert

//
void DPMarshaler::processUVar(OZ_Term uv, OZ_Term *uvarTerm)
{
  processCVar(uv, uvarTerm);
}

//
void DPMarshaler::processCVar(OZ_Term cv, OZ_Term *cvarTerm)
{
  Assert(cvarTerm);
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  // Note: futures are not triggered here! Instead, they are when a
  // snapshot of a term is taken;
  if (bs->availableSpace() >= DIFMaxSize + MNumberMaxSize +
             max(MDistVarMaxSize, MDistSPPMaxSize)) {
    if (marshalVariable(cvarTerm, bs)) {
      rememberVarNode(this, bs, cvarTerm);
    } else {
      // kost@ : this does not work currently: when a variable is
      // bound, its 'ref' owner entry will never be found.
      OZ_warning("marshaling a variable as a resource!");
      processNoGood(makeTaggedRef(cvarTerm), OK);
      rememberVarNode(this, bs, cvarTerm);
    }
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(makeTaggedRef(cvarTerm));
  }
}

//
void DPMarshaler::processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber)
{
  Assert(repNumber >= 0);
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
    marshalDIF(bs, DIF_REF);
    marshalTermRef(bs, repNumber);
  } else {
    Assert(t);                  // we should not get here without 't'!
    marshalDIF(bs, DIF_SUSPEND);
    if (oz_isVariable(t))
      suspend(makeTaggedRef(tPtr));
    else
      suspend(t);
  }
}

//
Bool DPMarshaler::processLTuple(OZ_Term ltupleTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
    marshalDIF(bs, DIF_LIST);
    rememberNode(this, bs, ltupleTerm);
    return (NO);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(ltupleTerm);
    // Observe: suspended nodes are obviously leaves!
    // And they must be also: otherwise the traverser will continue
    // with subtees omitting the node itself;
    return (OK);
  }
}

//
Bool DPMarshaler::processSRecord(OZ_Term srecordTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + 2*MNumberMaxSize) {
    SRecord *rec = tagged2SRecord(srecordTerm);
    TaggedRef label = rec->getLabel();

    //
    if (rec->isTuple()) {
      marshalDIF(bs, DIF_TUPLE);
      rememberNode(this, bs, srecordTerm);
      marshalNumber(bs, rec->getTupleWidth());
    } else {
      marshalDIF(bs, DIF_RECORD);
      rememberNode(this, bs, srecordTerm);
    }

    return (NO);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(srecordTerm);
    return (OK);
  }
}

//
Bool DPMarshaler::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize + MGNameMaxSize) {
    SChunk *ch    = (SChunk *) chunkConst;
    GName *gname  = globalizeConst(ch,bs);
    Assert(gname);

    //
    marshalDIF(bs,DIF_CHUNK);
    rememberNode(this, bs, chunkTerm);
    marshalGName(bs, gname);

    return (NO);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(chunkTerm);
    return (OK);
  }
}

//
Bool DPMarshaler::processFSETValue(OZ_Term fsetvalueTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize) {
    marshalDIF(bs, DIF_FSETVALUE);
    return (NO);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(fsetvalueTerm);
    return (OK);
  }
}

//
Bool DPMarshaler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >=
      2*DIFMaxSize + MDistSPPMaxSize + 2*MNumberMaxSize) {
    if (!d->isSafeDict()) {
      processNoGood(dictTerm, OK);
      rememberNode(this, bs, dictTerm);
      return (OK);
    } else {
      marshalDIF(bs,DIF_DICT);
      rememberNode(this, bs, dictTerm);
      marshalNumber(bs, d->getSize());
      return (NO);
    }
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspend(dictTerm);
    return (OK);
  }
}

//
Bool DPMarshaler::processArray(OZ_Term arrayTerm,
                               ConstTerm *arrayConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  processNoGood(arrayTerm, OK);
  rememberNode(this, bs, arrayTerm);
  return (OK);
}

//
Bool DPMarshaler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{
  ObjectClass *cl = (ObjectClass *) classConst;
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  // classes are not handled by the lazy protocol specially right now,
  // but they are still sent using SEND_LAZY, so the flag is to be
  // reset:
  doToplevel = FALSE;

  //
  if (cl->isSited()) {
    if (bs->availableSpace() >=
        DIFMaxSize + MDistSPPMaxSize + MNumberMaxSize) {
      processNoGood(classTerm, OK);
      rememberNode(this, bs, classTerm);
      return (OK);              // done - a leaf;
    }
  } else {
    if (bs->availableSpace() >= 2*DIFMaxSize + 2*MNumberMaxSize +
        MGNameMaxSize) {
      //
      marshalDIF(bs, DIF_CLASS);
      GName *gn = globalizeConst(cl, bs);
      Assert(gn);
      rememberNode(this, bs, classTerm);
      marshalGName(bs, gn);
      marshalNumber(bs, cl->getFlags());
      return (NO);
    }
  }
  marshalDIF(bs, DIF_SUSPEND);
  suspend(classTerm);
  return (OK);
}

//
Bool DPMarshaler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();

  //
  if (pred->isSited()) {
    if (bs->availableSpace() >=
        DIFMaxSize + MNumberMaxSize + MDistSPPMaxSize) {
      processNoGood(absTerm, OK);
      rememberNode(this, bs, absTerm);
      return (OK);              // done - a leaf;
    }
  } else {
    if (bs->availableSpace() >=
        2*DIFMaxSize + 7*MNumberMaxSize + MGNameMaxSize) {
      //
      GName* gname = globalizeConst(pp, bs);
      Assert(gname);

      //
      marshalDIF(bs, DIF_PROC);
      rememberNode(this, bs, absTerm);

      //
      marshalGName(bs, gname);
      marshalNumber(bs, pp->getArity());
      ProgramCounter pc = pp->getPC();
      int gs = pred->getGSize();
      marshalNumber(bs, gs);
      marshalNumber(bs, pred->getMaxX());
      marshalNumber(bs, pred->getLine());
      marshalNumber(bs, pred->getColumn());

      //
      ProgramCounter start = pp->getPC() - sizeOf(DEFINITION);

      //
      XReg reg;
      int nxt, line, colum;
      TaggedRef file, predName;
      CodeArea::getDefinitionArgs(start, reg, nxt, file,
                                  line, colum, predName);
      //
      marshalNumber(bs, nxt);   // codesize in ByteCode"s;

      //
      MarshalerCodeAreaDescriptor *desc =
        new MarshalerCodeAreaDescriptor(start, start + nxt);
      traverseBinary(dpMarshalCode, desc);

      //
      return (NO);
    }
  }

  //
  marshalDIF(bs, DIF_SUSPEND);
  suspend(absTerm);
  return (OK);
}

#include "dpMarshalcode.cc"

//
void DPMarshaler::processSync()
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  if (bs->availableSpace() >= 2*DIFMaxSize) {
    marshalDIF(bs, DIF_SYNC);
  } else {
    marshalDIF(bs, DIF_SUSPEND);
    suspendSync();
  }
}

//
//
void VariableExcavator::processSmallInt(OZ_Term siTerm) {}
void VariableExcavator::processFloat(OZ_Term floatTerm) {}
void VariableExcavator::processLiteral(OZ_Term litTerm) {}
void VariableExcavator::processBigInt(OZ_Term biTerm, ConstTerm *biConst) {}
void VariableExcavator::processBuiltin(OZ_Term biTerm, ConstTerm *biConst) {}
void VariableExcavator::processExtension(OZ_Term t) {}

//
Bool VariableExcavator::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  rememberTerm(objTerm);
  return (TRUE);
}
void VariableExcavator::processNoGood(OZ_Term resTerm, Bool trail)
{
  Assert(!oz_isVariable(resTerm));
}
void VariableExcavator::processLock(OZ_Term lockTerm, Tertiary *tert)
{
  rememberTerm(lockTerm);
}
Bool VariableExcavator::processCell(OZ_Term cellTerm, Tertiary *tert)
{
  rememberTerm(cellTerm);
  return (TRUE);
}
void VariableExcavator::processPort(OZ_Term portTerm, Tertiary *tert)
{
  rememberTerm(portTerm);
}
void VariableExcavator::processResource(OZ_Term rTerm, Tertiary *tert)
{
  rememberTerm(rTerm);
}

//
void VariableExcavator::processUVar(OZ_Term uv, OZ_Term *uvarTerm)
{
  processCVar(uv, uvarTerm);
}

//
void VariableExcavator::processCVar(OZ_Term cv, OZ_Term *cvarTerm)
{
  Assert(oz_isVariable(cv));
  rememberVarLocation(cvarTerm);
  addVar(makeTaggedRef(cvarTerm));
}

//
void VariableExcavator::processRepetition(OZ_Term t, OZ_Term *tPtr,
                                          int repNumber) {}
Bool VariableExcavator::processLTuple(OZ_Term ltupleTerm)
{
  rememberTerm(ltupleTerm);
  return (NO);
}
Bool VariableExcavator::processSRecord(OZ_Term srecordTerm)
{
  rememberTerm(srecordTerm);
  return (NO);
}
Bool VariableExcavator::processChunk(OZ_Term chunkTerm,
                                     ConstTerm *chunkConst)
{
  rememberTerm(chunkTerm);
  return (NO);
}

//
Bool VariableExcavator::processFSETValue(OZ_Term fsetvalueTerm)
{
  rememberTerm(fsetvalueTerm);
  return (NO);
}

//
Bool VariableExcavator::processDictionary(OZ_Term dictTerm,
                                          ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  rememberTerm(dictTerm);
  if (!d->isSafeDict()) {
    processNoGood(dictTerm, OK);
    return (OK);
  } else {
    return (NO);
  }
}

//
Bool VariableExcavator::processArray(OZ_Term arrayTerm,
                                     ConstTerm *arrayConst)
{
  rememberTerm(arrayTerm);
  processNoGood(arrayTerm, OK);
  return (OK);
}

//
Bool VariableExcavator::processClass(OZ_Term classTerm,
                                     ConstTerm *classConst)
{
  ObjectClass *cl = (ObjectClass *) classConst;
  rememberTerm(classTerm);
  if (cl->isSited()) {
    processNoGood(classTerm, OK);
    return (OK);                // done - a leaf;
  } else {
    return (NO);
  }
}

//
Bool VariableExcavator::processAbstraction(OZ_Term absTerm,
                                           ConstTerm *absConst)
{
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();

  //
  rememberTerm(absTerm);
  if (pred->isSited()) {
    processNoGood(absTerm, OK);
    return (OK);                // done - a leaf;
  } else {
    ProgramCounter start = pp->getPC() - sizeOf(DEFINITION);
    XReg reg;
    int nxt, line, colum;
    TaggedRef file, predName;
    CodeArea::getDefinitionArgs(start, reg, nxt, file,
                                line, colum, predName);

    //
    MarshalerCodeAreaDescriptor *desc =
      new MarshalerCodeAreaDescriptor(start, start + nxt);
    traverseBinary(traverseCode, desc);
    return (NO);
  }
}

//
void VariableExcavator::processSync() {}

//
// Not in use since we take a snapshot of a value ahead of
// marshaling now;

/*
static void contProcNOOP(GenTraverser *gt, GTAbstractEntity *cont)
{}

//
void VariableExcavator::copyStack(DPMarshaler *dpm)
{
  StackEntry *copyTop = dpm->getTop();
  StackEntry *copyBottom = dpm->getBottom();
  StackEntry *top = getTop();

  top += copyTop - copyBottom;
  setTop(top);

  //
  while (copyTop > copyBottom) {
    OZ_Term& t = (OZ_Term&) *(--copyTop);
    OZ_Term tc = t;
    DEREF(tc, tPtr, tTag);

    //
    *(--top) = (StackEntry) t;

    //
    switch (tTag) {
    case TAG_GCMARK:
      //
      switch (tc) {
      case taggedBATask:
        {
          *(--top) = *(--copyTop);
          TraverserBinaryAreaProcessor proc =
            (TraverserBinaryAreaProcessor) *(--copyTop);
          Assert(proc == dpMarshalCode);
          *(--top) = (StackEntry) traverseCode;
        }
        break;

      case taggedSyncTask:
        break;

      case taggedContTask:
        {
          *(--top) = *(--copyTop);
          TraverserContProcessor proc =
            (TraverserContProcessor) *(--copyTop);
          Assert(proc == dpMarshalLitCont ||
                 proc == dpMarshalByteArrayCont);
          *(--top) = (StackEntry) contProcNOOP;
        }
        break;
      }
      break;

    default:
      break;
    }
  }
}
*/


//
//
VariableExcavator ve;

/**********************************************************************/
/*  basic borrow, owner */
/**********************************************************************/

//
void marshalOwnHead(MarshalerBuffer *bs, int tag, int i)
{
  PD((MARSHAL_CT,"OwnHead"));
  bs->put(tag);
  myDSite->marshalDSite(bs);
  marshalNumber(bs, i);

  marshalCredit(bs,ownerTable->getOwner(i)->getCreditBig());
}

//
void saveMarshalOwnHead(int oti, Credit &c)
{
  c = ownerTable->getOwner(oti)->getCreditBig();
}

//
void marshalOwnHeadSaved(MarshalerBuffer *bs, int tag, int oti, Credit c)
{
  PD((MARSHAL_CT,"OwnHead"));
  bs->put(tag);
  myDSite->marshalDSite(bs);
  marshalNumber(bs, oti);
  marshalCredit(bs,c);
}

//
void discardOwnHeadSaved(int oti, Credit c)
{
  ownerTable->getOwner(oti)->addCredit(c);
}

//
void marshalToOwner(MarshalerBuffer *bs, int bi)
{
  PD((MARSHAL,"toOwner"));
  BorrowEntry *b = borrowTable->getBorrow(bi);
  int OTI = b->getOTI();
  marshalCreditToOwner(bs,b->getCreditSmall(),OTI);
}

//
// 'saveMarshalToOwner'/'marshalToOwnerSaved' are complimentary. These
// are used for immediate exportation of variable proxies and
// marshaling corresponding "exported variable proxies" later.
void saveMarshalToOwner(int bi, int &oti, Credit &c)
{
  PD((MARSHAL,"toOwner"));
  BorrowEntry *b = borrowTable->getBorrow(bi);

  //
  oti = b->getOTI();
  c = b->getCreditSmall();
}

//
void marshalToOwnerSaved(MarshalerBuffer *bs,Credit c,
                         int oti)
{
  marshalCreditToOwner(bs,c,oti);
}

//
void marshalBorrowHead(MarshalerBuffer *bs, MarshalTag tag, int bi)
{
  PD((MARSHAL,"BorrowHead"));
  bs->put((BYTE)tag);
  BorrowEntry *b = borrowTable->getBorrow(bi);
  NetAddress *na = b->getNetAddress();
  na->site->marshalDSite(bs);
  marshalNumber(bs, na->index);

  marshalCredit(bs, b->getCreditSmall());
}

//
void saveMarshalBorrowHead(int bi, DSite* &ms, int &oti,
                           Credit &c)
{
  PD((MARSHAL,"BorrowHead"));

  BorrowEntry *b = borrowTable->getBorrow(bi);
  NetAddress *na = b->getNetAddress();

  //
  ms = na->site;
  oti = na->index;
  //
  c = b->getCreditSmall();
}

//
void marshalBorrowHeadSaved(MarshalerBuffer *bs, MarshalTag tag, DSite *ms,
                            int oti, Credit c)
{
  bs->put((BYTE) tag);
  marshalDSite(bs, ms);
  marshalNumber(bs, oti);

  //
  marshalCredit(bs, c);
}

//
// The problem with borrow entries is that they can go away.
void discardBorrowHeadSaved(DSite *ms, int oti,
                            Credit credit)
{
  //
  NetAddress na = NetAddress(ms, oti);
  BorrowEntry *b = borrowTable->find(&na);

  //
  if (b) {
    // still there - then just nail credits back;
    b->addCredit(credit);
  } else {
    printf("discardBorrowHeadSaved - weird case reached\n");
    sendCreditBack(ms,oti,credit);
  }
}


#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalBorrowRobust(MarshalerBuffer *bs,OB_Entry *&ob,int &bi,int *error)
#else
OZ_Term unmarshalBorrow(MarshalerBuffer *bs,OB_Entry *&ob,int &bi)
#endif
{
  PD((UNMARSHAL,"Borrow"));
#ifndef USE_FAST_UNMARSHALER
  DSite*  sd=unmarshalDSiteRobust(bs,error);
  if(*error)  {Assert(0); return 0;}
  int si=unmarshalNumberRobust(bs,error);
  if(*error) {Assert(0); return 0;}
#else
  DSite*  sd=unmarshalDSite(bs);
  int si=unmarshalNumber(bs);
#endif
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==myDSite){
    Assert(0);
//     if(mt==DIF_PRIMARY){
//       cred = unmarshalCredit(bs);
//       PD((UNMARSHAL,"myDSite is owner"));
//       OwnerEntry* oe=ownerTable->getOwner(si);
//       if(cred != PERSISTENT_CRED)
//      oe->returnCreditOwner(cred);
//       OZ_Term ret = oe->getValue();
//       return ret;}
//     Assert(mt==DIF_SECONDARY);
//     cred = unmarshalCredit(bs);
//     DSite* cs=unmarshalDSite(bs);
//     sendSecondaryCredit(cs,myDSite,si,cred);
//     PD((UNMARSHAL,"myDSite is owner"));
//     OwnerEntry* oe=ownerTable->getOwner(si);
//     OZ_Term ret = oe->getValue();
//     return ret;
  }
  NetAddress na = NetAddress(sd,si);
  BorrowEntry *b = borrowTable->find(&na);
#ifndef USE_FAST_UNMARSHALER
  Credit cred = unmarshalCreditRobust(bs, error);
  if(*error) { Assert(0); return 0; }
#else
  Credit cred = unmarshalCredit(bs);
#endif
  if (b!=NULL) {
    b->addCredit(cred);
    ob = b;
    // Assert(b->getValue() != (OZ_Term) 0);
    return b->getValue();
  }
  else {
    bi=borrowTable->newBorrow(cred,sd,si);
    b=borrowTable->getBorrow(bi);
    ob=b;
    return 0;
  }

  ob=b;
  return 0;
}

/**********************************************************************/
/*  header */
/**********************************************************************/

MessageType unmarshalHeader(MarshalerBuffer *bs)
{
  MessageType mt= (MessageType) bs->get();
  mess_counter[mt].recv();
  return (mt);
}

/**********************************************************************/
/*   lazy objects                                                     */
/**********************************************************************/

//
// kost@ : both 'DIF_VAR_OBJECT' and 'DIF_STUB_OBJECT' (currently)
// should contain the same representation, since both are unmarshaled
// using 'unmarshalTertiary';
void marshalVarObject(ByteBuffer *bs, int BTI, GName *gnobj, GName *gnclass)
{
  //
  DSite* sd = bs->getSite();
  if (sd && borrowTable->getOriginSite(BTI) == sd) {
    marshalToOwner(bs, BTI);
  } else {
    marshalBorrowHead(bs, DIF_VAR_OBJECT, BTI);

    //
    if (gnobj) marshalGName(bs, gnobj);
    if (gnclass) marshalGName(bs, gnclass);
  }
}

/* *********************************************************************/
/*   interface to Oz-core                                  */
/* *********************************************************************/

void marshalTertiary(ByteBuffer *bs, Tertiary *t, MarshalTag tag)
{
  PD((MARSHAL,"Tert"));
  switch(t->getTertType()){
  case Te_Local:
    globalizeTert(t);
    // no break here!
  case Te_Manager:
    {
      PD((MARSHAL_CT,"manager"));
      int OTI=t->getIndex();
      marshalOwnHead(bs, tag, OTI);
      break;
    }
  case Te_Frame:
  case Te_Proxy:
    {
      PD((MARSHAL,"proxy"));
      int BTI=t->getIndex();
      DSite* sd=bs->getSite();
      if (sd && borrowTable->getOriginSite(BTI)==sd)
        marshalToOwner(bs, BTI);
      else
        marshalBorrowHead(bs, tag, BTI);
      break;
    }
  default:
    Assert(0);
  }
}

void marshalSPP(MarshalerBuffer *bs, TaggedRef entity, Bool trail)
{
  int index = RHT->find(entity);
  if(index == RESOURCE_NOT_IN_TABLE){
    OwnerEntry *oe_manager;
    index=ownerTable->newOwner(oe_manager);
    PD((GLOBALIZING,"GLOBALIZING Resource index:%d",index));
    oe_manager->mkRef(entity);
    RHT->add(entity, index);
  }
  if(trail)  marshalOwnHead(bs, DIF_RESOURCE_T, index);
  else  marshalOwnHead(bs, DIF_RESOURCE_N, index);
}


static
char *tagToComment(MarshalTag tag)
{
  switch(tag){
  case DIF_PORT:
    return "port";
  case DIF_THREAD_UNUSED:
    return "thread";
  case DIF_SPACE:
    return "space";
  case DIF_CELL:
    return "cell";
  case DIF_LOCK:
    return "lock";
  case DIF_OBJECT:
  case DIF_VAR_OBJECT:
  case DIF_STUB_OBJECT:
    return "object";
  case DIF_RESOURCE_T:
  case DIF_RESOURCE_N:
    return "resource";
  default:
    Assert(0);
    return "";
}}

#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalTertiaryRobust(MarshalerBuffer *bs, MarshalTag tag, int *error)
#else
OZ_Term unmarshalTertiary(MarshalerBuffer *bs, MarshalTag tag)
#endif
{
  OB_Entry* ob;
  int bi;
#ifndef USE_FAST_UNMARSHALER
  OZ_Term val = unmarshalBorrowRobust(bs,ob,bi,error);
  if(*error) return oz_nil();
#else
  OZ_Term val = unmarshalBorrow(bs,ob,bi);
#endif
  if(val){
    PD((UNMARSHAL,"%s hit b:%d",tagToComment(tag),bi));
    switch (tag) {
    case DIF_RESOURCE_T:
    case DIF_RESOURCE_N:
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
      break;
    case DIF_CELL:{
      Tertiary *t=ob->getTertiary(); // mm2: bug: ob is 0 if I am the owner
      // kost@ : i'm the f$ck really puzzled by this comment!
      DebugCode((void) ((ConstTerm *) t)->getType());
      break;}
    case DIF_LOCK:{
      Tertiary *t=ob->getTertiary();
      break;}
    case DIF_STUB_OBJECT:
    case DIF_VAR_OBJECT:
      TaggedRef obj;
      TaggedRef clas;
#ifndef USE_FAST_UNMARSHALER
      (void) unmarshalGNameRobust(&obj,bs,error);
      if(*error) return oz_nil();
      (void) unmarshalGNameRobust(&clas,bs,error);
      if(*error) return oz_nil();
#else
      (void) unmarshalGName(&obj,bs);
      (void) unmarshalGName(&clas,bs);
#endif
      break;
    default:
      Assert(0);
    }
    return val;
  }

  PD((UNMARSHAL,"%s miss b:%d",tagToComment(tag),bi));
  Tertiary *tert;

  switch (tag) {
  case DIF_RESOURCE_N:
  case DIF_RESOURCE_T:
    tert = new DistResource(bi);
    break;
  case DIF_PORT:
    tert = new PortProxy(bi);
    break;
  case DIF_THREAD_UNUSED:
    // tert = new Thread(bi,Te_Proxy);
    break;
  case DIF_SPACE:
    tert = new Space(bi,Te_Proxy);
    break;
  case DIF_CELL:
    tert = new CellProxy(bi);
    break;
  case DIF_LOCK:
    tert = new LockProxy(bi);
    break;
  case DIF_VAR_OBJECT:
  case DIF_STUB_OBJECT:
    {
      OZ_Term obj;
      OZ_Term clas;
      OZ_Term val;
#ifndef USE_FAST_UNMARSHALER
      GName *gnobj = unmarshalGNameRobust(&obj, bs, error);
      if(*error) return oz_nil();
      GName *gnclass = unmarshalGNameRobust(&clas, bs, error);
      if(*error) return oz_nil();
#else
      GName *gnobj = unmarshalGName(&obj, bs);
      GName *gnclass = unmarshalGName(&clas, bs);
#endif
      if(!gnobj) {
//      printf("Had Object %d:%d flags:%d\n",
//             ((BorrowEntry *)ob)->getNetAddress()->index,
//             ((BorrowEntry *)ob)->getNetAddress()->site->getTimeStamp()->pid,
//             ((BorrowEntry *)ob)->getFlags());
        if(!(borrowTable->maybeFreeBorrowEntry(bi))){
          ob->mkRef(obj,ob->getFlags());
//        printf("indx:%d %xd\n",((BorrowEntry *)ob)->getNetAddress()->index,
//               ((BorrowEntry *)ob)->getNetAddress()->site);
        }
        return (obj);
      }

      //
      if (gnclass) {
        // A new proxy for the class (borrow index is not there since
        // we do not run the lazy class protocol here);
        clas = newClassProxy((int) -1, gnclass);
        addGName(gnclass, clas);
      } else {
        // There are two cases: we've got either the class or its
        // proxy. In either case, it's used for the new object proxy:
      }

      //
      val = newObjectProxy(bi, gnobj, clas);
      addGName(gnobj, val);
      ob->changeToVar(val);

      //
      return (val);
    }
  default:
    Assert(0);
  }
  val=makeTaggedConst(tert);
  ob->changeToTertiary(tert);
  switch(((BorrowEntry*)ob)->getSite()->siteStatus()){
  case SITE_OK:{
    break;}
  case SITE_PERM:{
    deferProxyTertProbeFault(tert,PROBE_PERM);
    break;}
  case SITE_TEMP:{
    deferProxyTertProbeFault(tert,PROBE_TEMP);
    break;}
  default:
    Assert(0);
  }
  return val;
}


#ifndef USE_FAST_UNMARSHALER
OZ_Term unmarshalOwnerRobust(MarshalerBuffer *bs,MarshalTag mt,int *error)
#else
OZ_Term unmarshalOwner(MarshalerBuffer *bs,MarshalTag mt)
#endif
{
  int OTI;
#ifndef USE_FAST_UNMARSHALER
  Credit c=unmarshalCreditToOwnerRobust(bs,mt,OTI,error);
  if(*error) return oz_nil();
#else
  Credit c=unmarshalCreditToOwner(bs,mt,OTI);
#endif
  PD((UNMARSHAL,"OWNER o:%d",OTI));
  OwnerEntry* oe=ownerTable->getOwner(OTI);
  oe->addCredit(c);
  OZ_Term oz=oe->getValue();
  return oz;
}


//
//
OZ_Term dpUnmarshalTerm(ByteBuffer *bs, Builder *b)
{
  Assert(oz_onToplevel());

#ifndef USE_FAST_UNMARSHALER
  TRY_UNMARSHAL_ERROR {
#endif
    while(1) {
      MarshalTag tag = (MarshalTag) bs->get();
      dif_counter[tag].recv();  // kost@ : TODO: needed?
      //      printf("tag: %d\n", tag);

      switch (tag) {

      case DIF_SMALLINT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term ozInt = OZ_int(unmarshalNumberRobust(bs, &e));
          if(e || !OZ_isSmallInt(ozInt)) {
            (void) b->finish();
            return 0;
          }
#else
          OZ_Term ozInt = OZ_int(unmarshalNumber(bs));
#endif
          b->buildValue(ozInt);
          break;
        }

      case DIF_FLOAT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          double f = unmarshalFloatRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          double f = unmarshalFloat(bs);
#endif
          b->buildValue(OZ_float(f));
          break;
        }

      case DIF_NAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int nameSize  = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          OZ_Term value;
          GName *gname    = unmarshalGNameRobust(&value, bs, &e);
          RETURN_ON_ERROR(e);
          char *printname = unmarshalStringRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          int nameSize  = unmarshalNumber(bs);
          OZ_Term value;
          GName *gname    = unmarshalGName(&value, bs);
          char *printname = unmarshalString(bs);
#endif
          int strLen = strlen(printname);

          //
          // May be, we don't have a complete print name yet:
          if (nameSize > strLen) {
            DPUnmarshalerNameSusp *ns =
              new DPUnmarshalerNameSusp(refTag, nameSize, value,
                                        gname, printname, strLen);
            //
            b->getAbstractEntity(ns);
            b->suspend();
            // returns '-1' for an additional consistency check - the
            // caller should know whether that's a complete message;
            return ((OZ_Term) -1);
          } else {
            // complete print name;
            Assert(strLen == nameSize);

            //
            if (gname) {
              Name *aux;
              if (strcmp("", printname) == 0) {
                aux = Name::newName(am.currentBoard());
              } else {
                aux = NamedName::newNamedName(strdup(printname));
              }
              aux->import(gname);
              value = makeTaggedLiteral(aux);
              b->buildValue(value);
              addGName(gname, value);
            } else {
              b->buildValue(value);
            }

            //
            b->set(value, refTag);
            delete printname;
            break;
          }
        }

      case DIF_COPYABLENAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag      = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int nameSize  = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          char *printname = unmarshalStringRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag      = unmarshalRefTag(bs);
          int nameSize  = unmarshalNumber(bs);
          char *printname = unmarshalString(bs);
#endif
          int strLen = strlen(printname);

          //
          // May be, we don't have a complete print name yet:
          if (nameSize > strLen) {
            DPUnmarshalerCopyableNameSusp *cns =
              new DPUnmarshalerCopyableNameSusp(refTag, nameSize,
                                                printname, strLen);
            //
            b->getAbstractEntity(cns);
            b->suspend();
            // returns '-1' for an additional consistency check - the
            // caller should know whether that's a complete message;
            return ((OZ_Term) -1);
          } else {
            // complete print name;
            Assert(strLen == nameSize);
            OZ_Term value;

            NamedName *aux = NamedName::newCopyableName(strdup(printname));
            value = makeTaggedLiteral(aux);
            b->buildValue(value);
            b->set(value, refTag);
            delete printname;
          }
          break;
        }

      case DIF_UNIQUENAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag      = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int nameSize  = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          char *printname = unmarshalStringRobust(bs, &e);
          if(e || (printname == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag      = unmarshalRefTag(bs);
          int nameSize  = unmarshalNumber(bs);
          char *printname = unmarshalString(bs);
#endif
          int strLen = strlen(printname);

          //
          if (nameSize > strLen) {
            DPUnmarshalerUniqueNameSusp *uns =
              new DPUnmarshalerUniqueNameSusp(refTag, nameSize,
                                              printname, strLen);
            //
            b->getAbstractEntity(uns);
            b->suspend();
            // returns '-1' for an additional consistency check - the
            // caller should know whether that's a complete message;
            return ((OZ_Term) -1);
          } else {
            // complete print name;
            Assert(strLen == nameSize);
            OZ_Term value;

            value = oz_uniqueName(printname);
            b->buildValue(value);
            b->set(value, refTag);
            delete printname;
          }
          break;
        }

      case DIF_ATOM:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int nameSize  = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          char *aux  = unmarshalStringRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          int nameSize  = unmarshalNumber(bs);
          char *aux  = unmarshalString(bs);
#endif
          int strLen = strlen(aux);

          //
          if (nameSize > strLen) {
            DPUnmarshalerAtomSusp *as =
              new DPUnmarshalerAtomSusp(refTag, nameSize, aux, strLen);
            //
            b->getAbstractEntity(as);
            b->suspend();
            // returns '-1' for an additional consistency check - the
            // caller should know whether that's a complete message;
            return ((OZ_Term) -1);
          } else {
            // complete print name;
            Assert(strLen == nameSize);

            OZ_Term value = OZ_atom(aux);
            b->buildValue(value);
            b->set(value, refTag);
            delete aux;
          }
          break;
        }

        //
      case DIF_LIT_CONT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          char *aux  = unmarshalStringRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          char *aux  = unmarshalString(bs);
#endif
          int strLen = strlen(aux);

          //
          GTAbstractEntity *bae = b->buildAbstractEntity();
          switch (bae->getType()) {
          case GT_NameSusp:
            {
              DPUnmarshalerNameSusp *ns = (DPUnmarshalerNameSusp *) bae;
              ns->appendPrintname(strLen, aux);
              if (ns->getNameSize() > ns->getPNSize()) {
                // still not there;
                b->getAbstractEntity(ns);
                b->suspend();
                // returns '-1' for an additional consistency check - the
                // caller should know whether that's a complete message;
                return ((OZ_Term) -1);
              } else {
                Assert(ns->getNameSize() == ns->getPNSize());
                OZ_Term value;

                //
                if (ns->getGName()) {
                  Name *aux;
                  if (strcmp("", ns->getPrintname()) == 0) {
                    aux = Name::newName(am.currentBoard());
                  } else {
                    char *pn = ns->getPrintname();
                    aux = NamedName::newNamedName(strdup(pn));
                  }
                  aux->import(ns->getGName());
                  value = makeTaggedLiteral(aux);
                  b->buildValue(value);
                  addGName(ns->getGName(), value);
                } else {
                  value = ns->getValue();
                  b->buildValue(value);
                }

                //
                b->set(value, ns->getRefTag());
                delete ns;
              }
              break;
            }

          case GT_AtomSusp:
            {
              DPUnmarshalerAtomSusp *as = (DPUnmarshalerAtomSusp *) bae;
              as->appendPrintname(strLen, aux);
              if (as->getNameSize() > as->getPNSize()) {
                // still not there;
                b->getAbstractEntity(as);
                b->suspend();
                // returns '-1' for an additional consistency check - the
                // caller should know whether that's a complete message;
                return ((OZ_Term) -1);
              } else {
                Assert(as->getNameSize() == as->getPNSize());

                OZ_Term value = OZ_atom(as->getPrintname());
                b->buildValue(value);
                b->set(value, as->getRefTag());
                delete as;
              }
              break;
            }

          case GT_UniqueNameSusp:
            {
              DPUnmarshalerUniqueNameSusp *uns =
                (DPUnmarshalerUniqueNameSusp *) bae;
              uns->appendPrintname(strLen, aux);
              if (uns->getNameSize() > uns->getPNSize()) {
                // still not there;
                b->getAbstractEntity(uns);
                b->suspend();
                // returns '-1' for an additional consistency check - the
                // caller should know whether that's a complete message;
                return ((OZ_Term) -1);
              } else {
                Assert(uns->getNameSize() == uns->getPNSize());

                OZ_Term value = oz_uniqueName(uns->getPrintname());
                b->buildValue(value);
                b->set(value, uns->getRefTag());
                delete uns;
              }
              break;
            }

          case GT_CopyableNameSusp:
            {
              DPUnmarshalerCopyableNameSusp *cns =
                (DPUnmarshalerCopyableNameSusp *) bae;
              cns->appendPrintname(strLen, aux);
              if (cns->getNameSize() > cns->getPNSize()) {
                // still not there;
                b->getAbstractEntity(cns);
                b->suspend();
                // returns '-1' for an additional consistency check - the
                // caller should know whether that's a complete message;
                return ((OZ_Term) -1);
              } else {
                Assert(cns->getNameSize() == cns->getPNSize());
                OZ_Term value;
                char *pn = cns->getPrintname();

                NamedName *aux = NamedName::newCopyableName(strdup(pn));
                value = makeTaggedLiteral(aux);
                b->buildValue(value);
                b->set(value, cns->getRefTag());
                delete cns;
              }
              break;
            }

          default:
#ifndef USE_FAST_UNMARSHALER
            (void) b->finish();
            return 0;
#else
            OZ_error("Illegal GTAbstractEntity (builder) for a LitCont!");
#endif
          }
          break;
        }

      case DIF_BIGINT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          char *aux  = unmarshalStringRobust(bs, &e);
          if(e || (aux == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          char *aux  = unmarshalString(bs);
#endif
          b->buildValue(OZ_CStringToNumber(aux));
          delete aux;
          break;
        }

      case DIF_LIST:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildListRemember(refTag);
          break;
        }

      case DIF_TUPLE:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int argno  = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          int argno  = unmarshalNumber(bs);
#endif
          b->buildTupleRemember(argno, refTag);
          break;
        }

      case DIF_RECORD:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildRecordRemember(refTag);
          break;
        }

      case DIF_REF:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int i = unmarshalNumberRobust(bs, &e);
          if(e || !b->checkIndexFound(i)) {
            (void) b->finish();
            return 0;
          }
#else
          int i = unmarshalNumber(bs);
#endif
          b->buildValue(b->get(i));
          break;
        }

        //
        // kost@ : remember that either all DIF_STUB_OBJECT,
        // DIF_VAR_OBJECT and DIF_OWNER are remembered, or none of
        // them is remembered. That's because both 'marshalVariable'
        // and 'marshalObject' could yield 'DIF_OWNER' (see also
        // dpInterface.hh);
      case DIF_OWNER:
      case DIF_OWNER_SEC:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term tert = unmarshalOwnerRobust(bs, tag, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term tert = unmarshalOwner(bs, tag);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(tert, refTag);
          break;
        }

      case DIF_RESOURCE_T:
      case DIF_PORT:
      case DIF_THREAD_UNUSED:
      case DIF_SPACE:
      case DIF_CELL:
      case DIF_LOCK:
      case DIF_STUB_OBJECT:
      case DIF_VAR_OBJECT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term tert = unmarshalTertiaryRobust(bs, tag, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term tert = unmarshalTertiary(bs, tag);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(tert, refTag);
          break;
        }

      case DIF_RESOURCE_N:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term tert = unmarshalTertiaryRobust(bs, tag, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term tert = unmarshalTertiary(bs, tag);
#endif
          b->buildValue(tert);
          break;
        }

      case DIF_CHUNK:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          OZ_Term value;
          GName *gname = unmarshalGNameRobust(&value, bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          OZ_Term value;
          GName *gname = unmarshalGName(&value, bs);
#endif
          if (gname) {
            b->buildChunkRemember(gname, refTag);
          } else {
            b->knownChunk(value);
            b->set(value, refTag);
          }
          break;
        }

      case DIF_CLASS:
        {
          OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          GName *gname = unmarshalGNameRobust(&value, bs, &e);
          RETURN_ON_ERROR(e);
          int flags = unmarshalNumberRobust(bs, &e);
          if(e || (flags > CLASS_FLAGS_MAX)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          GName *gname = unmarshalGName(&value, bs);
          int flags = unmarshalNumber(bs);
#endif
          //
          if (gname) {
            b->buildClassRemember(gname, flags, refTag);
          } else {
            // Either we have the class itself, or that's the lazy
            // object protocol, in which case it must be a variable
            // that denotes an object (of this class);
            OZ_Term vd = oz_deref(value);
            switch (tagTypeOf(vd)) {
            case TAG_CONST:
              Assert(tagged2Const(vd)->getType() == Co_Class);
              b->knownClass(value);
              b->set(value, refTag);
              break;

            case TAG_CVAR:
              {
                OzVariable *cvar = tagged2CVar(vd);
                Assert(cvar->getType() == OZ_VAR_EXT);
                ExtVar *evar = (ExtVar *) cvar;
                Assert(evar->getIdV() == OZ_EVAR_LAZY);
                LazyVar *lvar = (LazyVar *) evar;
                Assert(lvar->getLazyType() == LT_CLASS);
                ClassVar *cv = (ClassVar *) lvar;
                // The binding of a class'es gname is kept until the
                // construction of a class is finished.
                gname = cv->getGName();
                b->buildClassRemember(gname, flags, refTag);
              }
              break;

            default:
#ifndef USE_FAST_UNMARSHALER
              (void) b->finish();
              return 0;
#else
              OZ_error("Unexpected value is bound to an object's gname");
#endif
            }
          }
          break;
        }

      case DIF_VAR:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term v = unmarshalVarRobust(bs, FALSE, FALSE, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term v = unmarshalVar(bs, FALSE, FALSE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(v, refTag);
          break;
        }

      case DIF_FUTURE:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term f = unmarshalVarRobust(bs, TRUE, FALSE, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term f = unmarshalVar(bs, TRUE, FALSE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(f, refTag);
          break;
        }

      case DIF_VAR_AUTO:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term va = unmarshalVarRobust(bs, FALSE, TRUE, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term va = unmarshalVar(bs, FALSE, TRUE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(va, refTag);
          break;
        }

      case DIF_FUTURE_AUTO:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          OZ_Term fa = unmarshalVarRobust(bs, TRUE, TRUE, &e);
          RETURN_ON_ERROR(e);
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
#else
          OZ_Term fa = unmarshalVar(bs, TRUE, TRUE);
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildValueRemember(fa, refTag);
          break;
        }

      case DIF_OBJECT:
        {
          OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          GName *gname = unmarshalGNameRobust(&value, bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          GName *gname = unmarshalGName(&value, bs);
#endif
          if (gname) {
            // If objects are distributed only using the lazy
            // protocol, this shouldn't happen: There must be a proxy
            // already;
            b->buildObjectRemember(gname, refTag);
          } else {
            // The same as for classes: either we have already the
            // object (how comes? requested twice??), or that's the
            // lazy object protocol;
            OZ_Term vd = oz_deref(value);
            switch (tagTypeOf(vd)) {
            case TAG_CONST:
              Assert(tagged2Const(vd)->getType() == Co_Object);
              OZ_warning("Full object is received again.");
              b->knownObject(value);
              b->set(value, refTag);
              break;

            case TAG_CVAR:
              {
                OzVariable *cvar = tagged2CVar(vd);
                Assert(cvar->getType() == OZ_VAR_EXT);
                ExtVar *evar = (ExtVar *) cvar;
                Assert(evar->getIdV() == OZ_EVAR_LAZY);
                LazyVar *lvar = (LazyVar *) evar;
                Assert(lvar->getLazyType() == LT_OBJECT);
                ObjectVar *ov = (ObjectVar *) lvar;
                gname = ov->getGName();
                // Observe: the gname points to the proxy;
                b->buildObjectRemember(gname, refTag);
              }
              break;

            default:
#ifndef USE_FAST_UNMARSHALER
              (void) b->finish();
              return 0;
#else
              OZ_error("Unexpected value is bound to an object's gname.");
#endif
            }
          }
          break;
        }

      case DIF_PROC:
        {
          OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag    = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          GName *gname  = unmarshalGNameRobust(&value, bs, &e);
          RETURN_ON_ERROR(e);
          int arity     = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          int gsize     = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          int maxX      = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          int line      = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          int column    = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
          int codesize  = unmarshalNumberRobust(bs, &e); // in ByteCode"s;
          RETURN_ON_ERROR(e);
          if (maxX < 0 || maxX >= NumberOfXRegisters) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag    = unmarshalRefTag(bs);
          GName *gname  = unmarshalGName(&value, bs);
          int arity     = unmarshalNumber(bs);
          int gsize     = unmarshalNumber(bs);
          int maxX      = unmarshalNumber(bs);
          int line      = unmarshalNumber(bs);
          int column    = unmarshalNumber(bs);
          int codesize  = unmarshalNumber(bs); // in ByteCode"s;
#endif

          //
          if (gname) {
            //
            CodeArea *code = new CodeArea(codesize);
            ProgramCounter start = code->getStart();
            ProgramCounter pc = start + sizeOf(DEFINITION);
            //
            BuilderCodeAreaDescriptor *desc =
              new BuilderCodeAreaDescriptor(start, start+codesize, code);
            b->buildBinary(desc);

            //
            b->buildProcRemember(gname, arity, gsize, maxX, line, column,
                                 pc, refTag);
          } else {
            Assert(oz_isAbstraction(oz_deref(value)));
            // ('zero' descriptions are not allowed;)
            BuilderCodeAreaDescriptor *desc =
              new BuilderCodeAreaDescriptor(0, 0, 0);
            b->buildBinary(desc);

            //
            b->knownProcRemember(value, refTag);
          }
          break;
        }

        //
        // 'DIF_CODEAREA' is an artifact due to the non-recursive
        // unmarshaling of code areas: in order to unmarshal an Oz term
        // that occurs in an instruction, unmarshaling of instructions
        // must be interrupted and later resumed; 'DIF_CODEAREA' tells the
        // unmarshaler that a new code area chunk begins;
      case DIF_CODEAREA:
        {
          BuilderOpaqueBA opaque;
          BuilderCodeAreaDescriptor *desc =
            (BuilderCodeAreaDescriptor *) b->fillBinary(opaque);
          //
#ifndef USE_FAST_UNMARSHALER
          switch (dpUnmarshalCodeRobust(bs, b, desc)) {
          case ERR:
            (void) b->finish();
            return 0;
          case OK:
            b->finishFillBinary(opaque);
            break;
          case NO:
            b->suspendFillBinary(opaque);
            break;
          }
#else
          if (dpUnmarshalCode(bs, b, desc))
            b->finishFillBinary(opaque);
          else
            b->suspendFillBinary(opaque);
#endif
          break;
        }

      case DIF_DICT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          int size   = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          int size   = unmarshalNumber(bs);
#endif
          Assert(oz_onToplevel());
          b->buildDictionaryRemember(size,refTag);
          break;
        }

      case DIF_BUILTIN:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          RETURN_ON_ERROR(e);
          char *name = unmarshalStringRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int refTag = unmarshalRefTag(bs);
          char *name = unmarshalString(bs);
#endif
          Builtin * found = string2CBuiltin(name);

          OZ_Term value;
          if (!found) {
            OZ_warning("Builtin '%s' not in table.", name);
            value = oz_nil();
            delete name;
          } else {
            if (found->isSited()) {
              OZ_warning("Unpickling sited builtin: '%s'", name);
            }

            delete name;
            value = makeTaggedConst(found);
          }
          b->buildValue(value);
          b->set(value, refTag);
          break;
        }

      case DIF_EXTENSION:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int type = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int type = unmarshalNumber(bs);
#endif

          //
          GTAbstractEntity *bae;
          OZ_Term value = oz_extension_unmarshal(type, bs, bae);
          switch (value) {
          case (OZ_Term) -1:    // suspension;
            Assert(bae);
            b->getAbstractEntity(bae);
            b->suspend();
            return ((OZ_Term) -1);

          case (OZ_Term) 0:     // error;
#ifndef USE_FAST_UNMARSHALER
            (void) b->finish();
            return 0;
#else
            OZ_error("Trouble with unmarshaling an extension!");
#endif

          default:              // got it!
            b->buildValue(value);
            break;
          }
          break;
        }

        // Continuation for "extensions";
      case DIF_EXT_CONT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int type = unmarshalNumberRobust(bs, &e);
          RETURN_ON_ERROR(e);
#else
          int type = unmarshalNumber(bs);
#endif

          //
          GTAbstractEntity *bae = b->buildAbstractEntity();
          Assert(bae->getType() == GT_ExtensionSusp);
          OZ_Term value = oz_extension_unmarshalCont(type, bs, bae);
          switch (value) {
          case (OZ_Term) -1:
            b->getAbstractEntity(bae);
            b->suspend();
            return ((OZ_Term) -1);

          case (OZ_Term) 0:
#ifdef USE_FAST_UNMARSHALER
            OZ_error("Problem(s) with unmarshaling an ExtCont!");
#endif
            (void) b->finish();
            return 0;

          default:
            b->buildValue(value);
            break;
          }
          break;
        }

      case DIF_FSETVALUE:
        b->buildFSETValue();
        break;

      case DIF_REF_DEBUG:
#ifndef USE_FAST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        OZD_error("not implemented!");
#endif

      case DIF_ARRAY:
#ifndef USE_FAST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        OZD_error("not implemented!");
        break;
#endif

        //
        // 'DIF_SYNC' and its handling is a part of the interface
        // between the builder object and the unmarshaler itself:
      case DIF_SYNC:
        b->processSync();
        break;

      case DIF_EOF:
        return (b->finish());

      case DIF_SUSPEND:
        //
        b->suspend();
        // returns '-1' for an additional consistency check - the
        // caller should know whether that's a complete message;
        return ((OZ_Term) -1);

      default:
#ifndef USE_FAST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        DebugCode(OZ_error("unmarshal: unexpected tag: %d\n",tag);)
        Assert(0);
        b->buildValue(oz_nil());
#endif
      }
    }
#ifndef USE_FAST_UNMARSHALER
  }
  CATCH_UNMARSHAL_ERROR {
    (void) b->finish();
    return 0;
  }
#endif
}

//
void DPMarshalers::dpAllocateMarshalers(int numof)
{
  if (musNum != numof) {
    MU *nmus = new MU[numof];
    int i = 0;

    //
    for (; i < min(musNum, numof); i++) {
      nmus[i] = mus[i];
    }
    for (; i < numof; i++) {
      nmus[i].flags = MUEmpty;
      nmus[i].m = (DPMarshaler *) 0; // lazy allocation;
      nmus[i].b = (Builder *) 0;
    }
    for (; i < musNum; i++) {
      if (mus[i].m) delete mus[i].m;
      if (mus[i].b) delete mus[i].b;
    }
    delete mus;

    //
    musNum = numof;
    mus = nmus;
  }
}

//
DPMarshaler* DPMarshalers::dpGetMarshaler()
{
  for (int i = 0; i < musNum; i++) {
    if (!(mus[i].flags & MUMarshalerBusy)) {
      if (!mus[i].m)
        mus[i].m = (DPMarshaler *) new DPMarshaler;
      mus[i].flags = mus[i].flags | MUMarshalerBusy;
      return (mus[i].m);
    }
  }
  OZ_error("dpGetMarshaler asked for an unallocated marshaler!");
  return ((DPMarshaler *) 0);
}
//
Builder* DPMarshalers::dpGetUnmarshaler()
{
  for (int i = 0; i < musNum; i++) {
    if (!(mus[i].flags & MUBuilderBusy)) {
      if (!mus[i].b)
        mus[i].b = new Builder;
      mus[i].flags = mus[i].flags | MUBuilderBusy;
      return (mus[i].b);
    }
  }
  OZ_error("dpGetUnmarshaler asked for an unallocated builder!");
  return ((Builder *) 0);
}

//
void DPMarshalers::dpReturnMarshaler(DPMarshaler* dpm)
{
  for (int i = 0; i < musNum; i++) {
    if (mus[i].m == dpm) {
      Assert(mus[i].flags & MUMarshalerBusy);
      mus[i].flags = mus[i].flags & ~MUMarshalerBusy;
      return;
    }
  }
  OZ_error("dpReturnMarshaler got an unallocated builder!!");
}
//
void DPMarshalers::dpReturnUnmarshaler(Builder* dpb)
{
  for (int i = 0; i < musNum; i++) {
    if (mus[i].b == dpb) {
      Assert(mus[i].flags & MUBuilderBusy);
      mus[i].flags = mus[i].flags & ~MUBuilderBusy;
      return;
    }
  }
  OZ_error("dpReturnMarshaler got an unallocated builder!!");
}

//
//
SntVarLocation* takeSntVarLocsOutline(OZ_Term vars, DSite *dest)
{
  SntVarLocation *svl = (SntVarLocation *) 0;
  Assert(!oz_isNil(vars));

  //
  do {
    OZ_Term vr = oz_head(vars);
    Assert(oz_isRef(vr));
    OZ_Term *vp = tagged2Ref(vr);
    Assert(oz_isVariable(*vp));
    OZ_Term v = *vp;

    //
  repeat:
    if (oz_isExtVar(v)) {
      ExtVarType evt = oz_getExtVar(v)->getIdV();
      switch (evt) {
      case OZ_EVAR_MANAGER:
        {
          // if it's a future, let's kick it now:
          if (triggerVariable(vp)) {
            v = makeTaggedRef(vp);
            DEREF(v, vp, _tag);
            if (oz_isVariable(v)) {
              goto repeat;      // 'v' can be a var again;
            } else {
              break;            // var is gone;
            }
          }

          // make&save an "exported" manager;
          ManagerVar *mvp = oz_getManagerVar(v);
          ExportedManagerVar *emvp = new ExportedManagerVar(mvp, dest);
          OZ_Term emv = makeTaggedCVar(emvp);
          //
          svl = new SntVarLocation(makeTaggedRef(vp), emv, svl);
        }
        break;

      case OZ_EVAR_PROXY:
        {
          // make&save an "exported" proxy:
          ProxyVar *pvp = oz_getProxyVar(v);
          ExportedProxyVar *epvp = new ExportedProxyVar(pvp, dest);
          OZ_Term epv = makeTaggedCVar(epvp);
          //
          svl = new SntVarLocation(makeTaggedRef(vp), epv, svl);
        }
        break;

      case OZ_EVAR_LAZY:
        // First, lazy variables are transparent for the programmer.
        // Secondly, the earlier an object gets over the network the
        // better for failure handling (aka "no failure - simple
        // failure handling!"). Altogether, delayed exportation is a
        // good thing here, so we ignore these vars;
        break;

      case OZ_EVAR_EMANAGER:
      case OZ_EVAR_EPROXY:
      case OZ_EVAR_GCSTUB:
        Assert(0);
        break;

      default:
        Assert(0);
        break;
      }
    } else if (oz_isFree(v) || oz_isFuture(v)) {
      Assert(perdioInitialized);

      // if it's a future, let's kick it now:
      if (triggerVariable(vp)) {
        v = makeTaggedRef(vp);
        DEREF(v, vp, _tag);
        if (oz_isVariable(v)) {
          goto repeat;          // 'v' can be a var again;
        } else {
          break;                // var is gone;
        }
      }

      //
      ManagerVar *mvp = globalizeFreeVariable(vp);
      ExportedManagerVar *emvp = new ExportedManagerVar(mvp, dest);
      OZ_Term emv = makeTaggedCVar(emvp);
      //
      svl = new SntVarLocation(makeTaggedRef(vp), emv, svl);
    } else {
      //

      // Actualy, we should copy all these types of variables
      // (constraint stuff), and then marshal them as "nogood"s.  So,
      // just ignoring them is a limitation: the system behaves
      // differently when such variables are bound between logical
      // sending of a message and their marshaling.
    }

    //
    vars = oz_tail(vars);
  } while (!oz_isNil(vars));

  //
  return (svl);
}

//
void deleteSntVarLocsOutline(SntVarLocation *locs)
{
  Assert(locs);
  do {
    OZ_Term vr = locs->getLoc();
    OZ_Term *vp = tagged2Ref(vr);
    OZ_Term v = locs->getVar();
    Assert(oz_isExtVar(v));     // must be distributed by now;
    ExtVarType evt = oz_getExtVar(v)->getIdV();

    //
    switch (evt) {
    case OZ_EVAR_EPROXY:
      {
        ExportedProxyVar *epvp = oz_getEProxyVar(v);
        epvp->disposeV();
      }
      break;

    case OZ_EVAR_EMANAGER:
      {
        ExportedManagerVar *emvp = oz_getEManagerVar(v);
        emvp->disposeV();
      }
      break;

    case OZ_EVAR_MANAGER:
    case OZ_EVAR_PROXY:
    case OZ_EVAR_LAZY:
    case OZ_EVAR_GCSTUB:
    default:
      Assert(0);
      break;
    }

    //
    SntVarLocation *locsTmp = locs;
    locs = locs->getNextLoc();
    delete locsTmp;
  } while (locs);
}

//
void MsgTermSnapshotImpl::gcStart()
{
  Assert(!(flags&MTS_SET));
  SntVarLocation* l = locs;
  while(l) {
    OZ_Term hvr = l->getLoc();
    OZ_Term *hvp = tagged2Ref(hvr);
    OZ_Term hv = *hvp;
    DebugCode(OZ_Term v = l->getVar(););
    Assert(oz_isExtVar(v));
    Assert(l->getSavedValue() == (OZ_Term) -1);

    //
    // Keep variables. It can also happen that the variable appears in
    // more than one snapshot currently being GCed:
    if (!oz_isVariable(hv) && !oz_isGCStubVar(hv)) {
      GCStubVar *svp = new GCStubVar(hv);
      *hvp = makeTaggedCVar(svp);
    }

    //
    l = l->getNextLoc();
  }
}

//
void MsgTermSnapshotImpl::gcFinish()
{
  SntVarLocation* l = locs;
  while(l) {
    OZ_Term hvr = l->getLoc();
    OZ_Term *hvp = tagged2Ref(hvr);
    OZ_Term hv = *hvp;

    //
    if (oz_isRef(hv)) {
      _DEREF(hv, hvp, _tag);
      Assert(oz_isVariable(hv));
      l->gcSetLoc(makeTaggedRef(hvp));
    }

    //
    if (oz_isGCStubVar(hv)) {
      GCStubVar *gcsv = oz_getGCStubVar(hv);
      *hvp = gcsv->getValue();
      gcsv->disposeV();
    }

    //
    l = l->getNextLoc();
  }
}

//
void MsgTermSnapshotImpl::gc()
{
  SntVarLocation* l = locs;
  while(l) {
    OZ_Term &vr = l->getLocRef();
    OZ_Term &v = l->getVarRef();

    //
    oz_gCollectTerm(vr, vr);
    // Note: 'v' is a direct variable here. This is OK since nobody
    // else can access it:
    oz_gCollectTerm(v, v);

    //
    l = l->getNextLoc();
  }
}
