/*
 *  Authors:
 *    Ralf Scheidhauer <Ralf.Scheidhauer@ps.uni-sb.de>
 *    Kostja Popov <kost@sics.se>
 *
 *  Contributors:
 *    Andreas Sundstroem <andreas@sics.se>
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

#ifdef INTERFACE
#pragma implementation "pickle.hh"
#endif

#include "base.hh"
#include "pickle.hh"
#include "boot-manager.hh"

//
// init stuff - must be called;
static Bool isInitialized;
void initPickleMarshaler()
{
  NMMemoryManager::init();
  isInitialized = OK;
  Assert(DIF_LAST == 51);  /* new dif(s) added? */
  initRobustMarshaler();        // called once - from here;
}

//
// CodeArea processors for pickling, resource excavation and
// unpickling;
#include "picklecode.cc"

//
void Pickler::processSmallInt(OZ_Term siTerm)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  marshalSmallInt(bs, siTerm);
}

//
void Pickler::processFloat(OZ_Term floatTerm)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  marshalFloat(bs, floatTerm);
}

//
void Pickler::processLiteral(OZ_Term litTerm)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  int litTermInd = rememberTerm(litTerm);

  //
  marshalLiteral(bs, litTerm, litTermInd);
}

//
void Pickler::processBigInt(OZ_Term biTerm, ConstTerm *biConst)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  marshalBigInt(bs, biTerm, biConst);
}

//
void Pickler::processNoGood(OZ_Term resTerm, Bool trail)
{
  OZ_error("Pickler::processNoGood is called!");
}

//
void Pickler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  Builtin *bi= (Builtin *) biConst;
  const char *pn = bi->getPrintName();
  Assert(!isResource(biTerm));

  //
  if (bi->isSited()) {
    processNoGood(biTerm, OK);
    rememberNode(this, bs, biTerm);
  } else {
    marshalDIF(bs, DIF_BUILTIN);
    rememberNode(this, bs, biTerm);
    marshalString(bs, pn);
  }
}

//
void Pickler::processExtension(OZ_Term t)
{
  Assert(!isResource(t));
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
  marshalDIF(bs,DIF_EXTENSION);
  marshalNumber(bs, tagged2Extension(t)->getIdV());
  if (!tagged2Extension(t)->marshalV(bs))
    processNoGood(t, NO);       // not remembered!
}

//
Bool Pickler::processObject(OZ_Term term, ConstTerm *objConst)
{
  Assert(!isResource(term));
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  Object *o = (Object*) objConst;

  //
  processNoGood(term, OK);
  rememberNode(this, bs, term);
  return (TRUE);
}

//
#define HandleTert(string,tert,term,tag,check)          \
    Assert(!isResource(term));                          \
    PickleBuffer *bs = (PickleBuffer *) getOpaque();            \
    processNoGood(term, OK);                            \
    rememberNode(this, bs, term);

void Pickler::processLock(OZ_Term term, Tertiary *tert)
{
  HandleTert("lock",tert,term,DIF_LOCK,OK);
}

Bool Pickler::processCell(OZ_Term term, Tertiary *tert)
{
  //  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  if (bs->cloneCells() && tert->isLocal()) {
    marshalDIF(bs, DIF_CLONEDCELL);
    rememberNode(this, bs, term);
    return (NO);
  } else {
    HandleTert("cell",tert,term,DIF_CELL,OK);
    return (OK);
  }
}

void Pickler::processPort(OZ_Term term, Tertiary *tert)
{
  HandleTert("port",tert,term,DIF_PORT,NO);
}

void Pickler::processResource(OZ_Term term, Tertiary *tert)
{
  HandleTert("resource",tert,term,DIF_RESOURCE_T,OK);
}

#undef HandleTert

//
void Pickler::processUVar(OZ_Term uv, OZ_Term *uvarTerm)
{
#ifdef DEBUG_CHECK
  OZ_Term term = processCVar(uv, uvarTerm);
  Assert(term == (OZ_Term) 0);
#else
  (void) processCVar(uv, uvarTerm);
#endif
}

//
OZ_Term Pickler::processCVar(OZ_Term cv, OZ_Term *cvarTerm)
{
  Assert(isResource(cv));
  return ((OZ_Term) 0);
}

//
void Pickler::processRepetition(OZ_Term t, int repNumber)
{
  Assert(repNumber >= 0);
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
  marshalDIF(bs, DIF_REF);
  marshalTermRef(bs, repNumber);
}

//
Bool Pickler::processLTuple(OZ_Term ltupleTerm)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
  marshalDIF(bs, DIF_LIST);
  rememberNode(this, bs, ltupleTerm);
  return (NO);
}

//
Bool Pickler::processSRecord(OZ_Term srecordTerm)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
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

  //
  return (NO);
}

//
Bool Pickler::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  SChunk *ch    = (SChunk *) chunkConst;
  GName *gname  = globalizeConst(ch,bs);

  //
  marshalDIF(bs,DIF_CHUNK);
  rememberNode(this, bs, chunkTerm);
  if (gname) marshalGName(bs, gname);

  //
  return (NO);
}

//
Bool Pickler::processFSETValue(OZ_Term fsetvalueTerm)
{
  if (ozconf.perdioMinimal) {
    processNoGood(fsetvalueTerm, NO);
    return (OK);
  }

  //
  Assert(!isResource(fsetvalueTerm));
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
  marshalDIF(bs, DIF_FSETVALUE);
  return (NO);
}

//
Bool Pickler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
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
}

Bool Pickler::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  if (bs->cloneCells()) {
    OzArray *array = (OzArray *) arrayConst;
    marshalDIF(bs, DIF_ARRAY);
    rememberNode(this, bs, arrayTerm);
    marshalNumber(bs, array->getLow());
    marshalNumber(bs, array->getHigh());
    return (NO);
  } else {
    processNoGood(arrayTerm, OK);
    rememberNode(this, bs, arrayTerm);
    return (OK);
  }
}

//
Bool Pickler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{
  ObjectClass *cl = (ObjectClass *) classConst;
  PickleBuffer *bs = (PickleBuffer *) getOpaque();

  //
  if (cl->isSited()) {
    processNoGood(classTerm, OK);
    rememberNode(this, bs, classTerm);
    return (OK);                // done - a leaf;
  } else {
    marshalDIF(bs, DIF_CLASS);
    GName *gn = globalizeConst(cl, bs);
    rememberNode(this, bs, classTerm);
    if (gn) marshalGName(bs, gn);
    marshalNumber(bs, cl->getFlags());
    return (NO);
  }
}

//
Bool Pickler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();
  ProgramCounter start;

  //
  if (pred->isSited()) {
    processNoGood(absTerm, OK);
    rememberNode(this, bs, absTerm);
    return (OK);                // done - a leaf;
  } else {
    //
    GName* gname = globalizeConst(pp, bs);

    //
    marshalDIF(bs, DIF_PROC);
    rememberNode(this, bs, absTerm);

    //
    if (gname) marshalGName(bs, gname);
    marshalNumber(bs, pp->getArity());
    ProgramCounter pc = pp->getPC();
    int gs = pred->getGSize();
    marshalNumber(bs, gs);
    marshalNumber(bs, pred->getMaxX());
    marshalNumber(bs, pred->getLine());
    marshalNumber(bs, pred->getColumn());

    //
    start = pp->getPC() - sizeOf(DEFINITION);

    //
    XReg reg;
    int nxt, line, colum;
    TaggedRef file, predName;
    CodeArea::getDefinitionArgs(start, reg, nxt, file,
                                line, colum, predName);
    //
    marshalNumber(bs, nxt);     // codesize in ByteCode"s;

    //
    MarshalerCodeAreaDescriptor *desc =
      new MarshalerCodeAreaDescriptor(start, start + nxt);
    marshalBinary(pickleCode, desc);

    //
    return (NO);
  }
}

//
void Pickler::processSync()
{
  PickleBuffer *bs = (PickleBuffer *) getOpaque();
  marshalDIF(bs, DIF_SYNC);
}

//
//
void ResourceExcavator::processSmallInt(OZ_Term siTerm) {}
void ResourceExcavator::processFloat(OZ_Term floatTerm) {}
void ResourceExcavator::processLiteral(OZ_Term litTerm) {}
void ResourceExcavator::processBigInt(OZ_Term biTerm, ConstTerm *biConst) {}

//
void ResourceExcavator::processNoGood(OZ_Term resTerm, Bool trail)
{
  Assert(!isResource(resTerm));
  addNogood(resTerm);
}
void ResourceExcavator::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  Assert(!isResource(biTerm));
}
void ResourceExcavator::processExtension(OZ_Term t)
{
  if (isResource(t))
    addResource(t);
}
Bool ResourceExcavator::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  rememberTerm(objTerm);
  if (isResource(objTerm))
    addResource(objTerm);
  return (TRUE);
}
void ResourceExcavator::processLock(OZ_Term lockTerm, Tertiary *tert)
{
  rememberTerm(lockTerm);
  if (isResource(lockTerm))
    addResource(lockTerm);
}
Bool ResourceExcavator::processCell(OZ_Term cellTerm, Tertiary *tert)
{
  rememberTerm(cellTerm);
  if (isResource(cellTerm))
    addResource(cellTerm);
  return (TRUE);
}
void ResourceExcavator::processPort(OZ_Term portTerm, Tertiary *tert)
{
  rememberTerm(portTerm);
  if (isResource(portTerm))
    addResource(portTerm);
}
void ResourceExcavator::processResource(OZ_Term rTerm, Tertiary *tert)
{
  rememberTerm(rTerm);
  if (isResource(rTerm))
    addResource(rTerm);
}

//
void ResourceExcavator::processUVar(OZ_Term uv, OZ_Term *uvarTerm)
{
#ifdef DEBUG_CHECK
  OZ_Term term = processCVar(uv, uvarTerm);
  Assert(term == (OZ_Term) 0);
#else
  (void) processCVar(uv, uvarTerm);
#endif
}

//
OZ_Term ResourceExcavator::processCVar(OZ_Term cv, OZ_Term *cvarTerm)
{
  Assert(isResource(cv));
  addResource(makeTaggedRef(cvarTerm));
  return ((OZ_Term) 0);
}

//
void ResourceExcavator::processRepetition(OZ_Term t, int repNumber) {}
Bool ResourceExcavator::processLTuple(OZ_Term ltupleTerm)
{
  rememberTerm(ltupleTerm);
  return (NO);
}
Bool ResourceExcavator::processSRecord(OZ_Term srecordTerm)
{
  rememberTerm(srecordTerm);
  return (NO);
}
Bool ResourceExcavator::processChunk(OZ_Term chunkTerm,
                                     ConstTerm *chunkConst)
{
  rememberTerm(chunkTerm);
  return (NO);
}

//
Bool ResourceExcavator::processFSETValue(OZ_Term fsetvalueTerm)
{
  rememberTerm(fsetvalueTerm);
  if (ozconf.perdioMinimal) {
    processNoGood(fsetvalueTerm, NO);
    return (OK);
  }
  if (isResource(fsetvalueTerm)) {
    addResource(fsetvalueTerm);
    return (OK);
  } else {
    return (NO);
  }
}

//
Bool ResourceExcavator::processDictionary(OZ_Term dictTerm,
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
Bool ResourceExcavator::processArray(OZ_Term arrayTerm,
                                     ConstTerm *arrayConst)
{
  rememberTerm(arrayTerm);
  if (cloneCells()) {
    return (NO);
  } else {
    processNoGood(arrayTerm, OK);
    return (OK);
  }
}

//
Bool ResourceExcavator::processClass(OZ_Term classTerm,
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
Bool ResourceExcavator::processAbstraction(OZ_Term absTerm,
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
    marshalBinary(traverseCode, desc);
    return (NO);
  }
}

//
void ResourceExcavator::processSync() {}

//
Pickler pickler;
ResourceExcavator re;
Builder unpickler;

//
//
OZ_Term unpickleTermInternal(PickleBuffer *bs)
{
  Assert(isInitialized);
  Assert(oz_onToplevel());
  Builder *b;

#ifndef USE_FAST_UNMARSHALER
  TRY_UNMARSHAL_ERROR {
#endif
    while(1) {
      b = &unpickler;
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
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          double f = unmarshalFloat(bs);
#endif
          b->buildValue(OZ_float(f));
          break;
        }

      case DIF_NAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2,e3;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          OZ_Term value;
          GName *gname    = unmarshalGNameRobust(&value, bs, &e3);
          if(e1 || e2 || e3) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
          OZ_Term value;
          GName *gname    = unmarshalGName(&value, bs);
#endif

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

      case DIF_COPYABLENAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag      = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (printname == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag      = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
#endif
          OZ_Term value;

          NamedName *aux = NamedName::newCopyableName(strdup(printname));
          value = makeTaggedLiteral(aux);
          b->buildValue(value);
          b->set(value, refTag);
          delete printname;
          break;
        }

      case DIF_UNIQUENAME:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag      = unmarshalRefTagRobust(bs, b, &e1);
          char *printname = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (printname == NULL)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag      = unmarshalRefTag(bs);
          char *printname = unmarshalString(bs);
#endif
          OZ_Term value;

          value = oz_uniqueName(printname);
          b->buildValue(value);
          b->set(value, refTag);
          delete printname;
          break;
        }

      case DIF_ATOM:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *aux  = unmarshalStringRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          char *aux  = unmarshalString(bs);
#endif
          OZ_Term value = OZ_atom(aux);
          b->buildValue(value);
          b->set(value, refTag);
          delete aux;
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
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildListRemember(refTag);
          break;
        }

      case DIF_TUPLE:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          int argno  = unmarshalNumberRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
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
          if(e) {
            (void) b->finish();
            return 0;
          }
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
      case DIF_OWNER:
      case DIF_OWNER_SEC:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected pickle tag (DIF_OWNER, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_RESOURCE_T:
      case DIF_PORT:
      case DIF_THREAD_UNUSED:
      case DIF_SPACE:
      case DIF_CELL:
      case DIF_LOCK:
      case DIF_OBJECT:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected tertiary (%d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_RESOURCE_N:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected resource (DIF_RESOURCE_N, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_CHUNK:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          OZ_Term value;
          GName *gname = unmarshalGNameRobust(&value, bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
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
          int e1,e2,e3;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          GName *gname = unmarshalGNameRobust(&value, bs, &e2);
          int flags = unmarshalNumberRobust(bs, &e3);
          if(e1 || e2 || e3 || (flags > CLASS_FLAGS_MAX)) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          GName *gname = unmarshalGName(&value, bs);
          int flags = unmarshalNumber(bs);
#endif

          if (gname) {
            b->buildClassRemember(gname,flags,refTag);
          } else {
            b->knownClass(value);
            b->set(value,refTag);
          }
          break;
        }

      case DIF_VAR:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected variable (DIF_VAR, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_FUTURE:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected future (DIF_FUTURE, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_VAR_AUTO:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected auto var (DIF_VAR_AUTO, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_FUTURE_AUTO:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected auto future (DIF_FUTURE_AUTO, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_VAR_OBJECT:
#ifdef USE_FAST_UNMARSHALER
        DebugCode(OZ_error("unmarshal: unexpected object var (DIF_VAR_OBJECT, %d)\n",tag);)
        b->buildValue(oz_nil());
#else
        (void) b->finish();
        return 0;
#endif

      case DIF_PROC:
        {
          OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
          int e1,e2,e3,e4,e5,e6,e7,e8;
          int refTag    = unmarshalRefTagRobust(bs, b, &e1);
          GName *gname  = unmarshalGNameRobust(&value, bs, &e2);
          int arity     = unmarshalNumberRobust(bs, &e3);
          int gsize     = unmarshalNumberRobust(bs, &e4);
          int maxX      = unmarshalNumberRobust(bs, &e5);
          int line      = unmarshalNumberRobust(bs, &e6);
          int column    = unmarshalNumberRobust(bs, &e7);
          int codesize  = unmarshalNumberRobust(bs, &e8); // in ByteCode"s;
          if(e1 || e2 || e3 || e4 || e5 || e6 || e7 || e8) {
            (void) b->finish();
            return 0;
          }
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
          switch (unpickleCodeRobust(bs, b, desc)) {
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
          if (unpickleCode(bs, b, desc))
            b->finishFillBinary(opaque);
          else
            b->suspendFillBinary(opaque);
#endif
          break;
        }

      case DIF_DICT:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          int size   = unmarshalNumberRobust(bs, &e2);
          if(e1 || e2) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          int size   = unmarshalNumber(bs);
#endif
          Assert(oz_onToplevel());
          b->buildDictionaryRemember(size,refTag);
          break;
        }

      case DIF_ARRAY:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2,e3;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          int low    = unmarshalNumberRobust(bs, &e2);
          int high   = unmarshalNumberRobust(bs, &e3);
          if(e1 || e2 || e3) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
          int low    = unmarshalNumber(bs);
          int high   = unmarshalNumber(bs);
#endif
          Assert(oz_onToplevel());
          b->buildArrayRemember(low, high, refTag);
          break;
        }

      case DIF_BUILTIN:
        {
#ifndef USE_FAST_UNMARSHALER
          int e1,e2;
          int refTag = unmarshalRefTagRobust(bs, b, &e1);
          char *name = unmarshalStringRobust(bs, &e2);
          if(e1 || e2 || (name == NULL)) {
            (void) b->finish();
            return 0;
          }
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
          if(e) {
            (void) b->finish();
            return 0;
          }
#else
          int type = unmarshalNumber(bs);
#endif
          OZ_Term value = oz_extension_unmarshal(type,bs);
          if(value == 0) {
            break;  // next value is nogood
          }
          b->buildValue(value);
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

        //
        // 'DIF_SYNC' and its handling is a part of the interface
        // between the builder object and the unmarshaler itself:
      case DIF_SYNC:
        b->processSync();
        break;

      case DIF_CLONEDCELL:
        {
#ifndef USE_FAST_UNMARSHALER
          int e;
          int refTag = unmarshalRefTagRobust(bs, b, &e);
          if (e) {
            (void) b->finish();
            return 0;
          }
#else
          int refTag = unmarshalRefTag(bs);
#endif
          b->buildClonedCellRemember(refTag);
          break;
        }

      case DIF_EOF:
        return (b->finish());

      default:
#ifndef USE_FAST_UNMARSHALER
        (void) b->finish();
        return 0;
#else
        DebugCode(OZ_error("unmarshal: unexpected tag: %d\n",tag);)
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
