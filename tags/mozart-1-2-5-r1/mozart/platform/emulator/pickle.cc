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
#include "gname.hh"
#include "boot-manager.hh"

#define RETURN_ON_ERROR(ERROR)             \
        if (ERROR) { (void) b->finish(); return ((OZ_Term) 0); }

//
// init stuff - must be called;
static Bool isInitialized;
void initPickleMarshaler()
{
  isInitialized = OK;
  Assert(DIF_LAST == 51);  /* new dif(s) added? */
  initRobustMarshaler();	// called once - from here;
}

//
// CodeArea processors for pickling, resource excavation and
// unpickling;
#include "picklecode.cc"

//
void Pickler::processSmallInt(OZ_Term siTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalSmallInt(bs, siTerm);
}

//
void Pickler::processFloat(OZ_Term floatTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalFloat(bs, floatTerm);
}

//
void Pickler::processLiteral(OZ_Term litTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  int litTermInd = rememberTerm(litTerm);

  //
  marshalLiteral(bs, litTerm, litTermInd);
}

//
void Pickler::processBigInt(OZ_Term biTerm, ConstTerm *biConst)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalBigInt(bs, biTerm, biConst);
}

//
Bool Pickler::processNoGood(OZ_Term resTerm, Bool trail)
{
  OZ_error("Pickler::processNoGood is called!");
  return (OK);
}

//
void Pickler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  Builtin *bi= (Builtin *) biConst;
  const char *pn = bi->getPrintName();
  Assert(!bi->isSited());

  //
  marshalDIF(bs, DIF_BUILTIN);
  rememberNode(this, bs, biTerm);
  marshalString(bs, pn);
}

//
void Pickler::processExtension(OZ_Term t)
{
  Assert(tagged2Extension(t)->toBePickledV());
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  marshalDIF(bs,DIF_EXTENSION);
  rememberNode(this,bs,t);
  marshalNumber(bs, tagged2Extension(t)->getIdV());
  // Pickling must be defined for this entity:
  Bool p = tagged2Extension(t)->pickleV(bs);
  Assert(p);
}

//
Bool Pickler::processObject(OZ_Term term, ConstTerm *objConst)
{
  OZ_error("Pickler::processObject is called!");
  return (TRUE);
}

//
void Pickler::processLock(OZ_Term term, Tertiary *tert)
{
  OZ_error("Pickler::processLock is called!");
}

Bool Pickler::processCell(OZ_Term term, Tertiary *tert)
{
  //  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();	 
  Assert(cloneCells() && tert->isLocal());
  marshalDIF(bs, DIF_CLONEDCELL);
  rememberNode(this, bs, term);
  return (NO);
}

void Pickler::processPort(OZ_Term term, Tertiary *tert)
{
  OZ_error("Pickler::processPort is called!");
}

void Pickler::processResource(OZ_Term term, Tertiary *tert)
{
  OZ_error("Pickler::processResource is called!");
}

//
void Pickler::processVar(OZ_Term cv, OZ_Term *varTerm)
{
  OZ_error("Pickler::processVar is called!");
}

//
void Pickler::processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber)
{
  Assert(repNumber >= 0);
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  marshalDIF(bs, DIF_REF);
  marshalTermRef(bs, repNumber);
}

//
Bool Pickler::processLTuple(OZ_Term ltupleTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  marshalDIF(bs, DIF_LIST);
  rememberNode(this, bs, ltupleTerm);
  return (NO);
}

//
Bool Pickler::processSRecord(OZ_Term srecordTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
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
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  SChunk *ch    = (SChunk *) chunkConst;
  GName *gname  = globalizeConst(ch, bs);
  Assert(gname);

  //
  marshalDIF(bs,DIF_CHUNK);
  rememberNode(this, bs, chunkTerm);
  marshalGName(bs, gname);

  //
  return (NO);
}

//
Bool Pickler::processFSETValue(OZ_Term fsetvalueTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalDIF(bs, DIF_FSETVALUE);
  return (NO);
}

//
Bool Pickler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  Assert(d->isSafeDict());

  //
  marshalDIF(bs,DIF_DICT);
  rememberNode(this, bs, dictTerm);
  marshalNumber(bs, d->getSize());
  return (NO);
}

Bool Pickler::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  OzArray *array = (OzArray *) arrayConst;
  Assert(cloneCells());

  //
  marshalDIF(bs, DIF_ARRAY);
  rememberNode(this, bs, arrayTerm);
  marshalNumber(bs, array->getLow());
  marshalNumber(bs, array->getHigh());
  return (NO);
}

//
Bool Pickler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  ObjectClass *cl = (ObjectClass *) classConst;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  Assert(!cl->isSited());
  GName *gn = globalizeConst(cl, bs);
  Assert(gn);

  //
  marshalDIF(bs, DIF_CLASS);
  rememberNode(this, bs, classTerm);
  marshalGName(bs, gn);
  marshalNumber(bs, cl->getFlags());
  return (NO);
}

//
Bool Pickler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  Abstraction *pp = (Abstraction *) absConst;

  //
  GName* gname = globalizeConst(pp, bs);
  Assert(gname);
  PrTabEntry *pred = pp->getPred();
  Assert(!pred->isSited());
  ProgramCounter start;

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
  start = pp->getPC() - sizeOf(DEFINITION);

  //
  XReg reg;
  int nxt, line, colum;
  TaggedRef file, predName;
  CodeArea::getDefinitionArgs(start, reg, nxt, file,
			      line, colum, predName);
  //
  marshalNumber(bs, nxt);	// codesize in ByteCode"s;

  //
  MarshalerCodeAreaDescriptor *desc = 
    new MarshalerCodeAreaDescriptor(start, start + nxt);
  traverseBinary(pickleCode, desc);

  //
  return (NO);
}

//
void Pickler::processSync()
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalDIF(bs, DIF_SYNC);
}

//
// 
void ResourceExcavator::processSmallInt(OZ_Term siTerm) {}
void ResourceExcavator::processFloat(OZ_Term floatTerm) {}
void ResourceExcavator::processLiteral(OZ_Term litTerm) {}
void ResourceExcavator::processBigInt(OZ_Term biTerm, ConstTerm *biConst) {}

//
Bool ResourceExcavator::processNoGood(OZ_Term resTerm, Bool trail)
{
  addNogood(resTerm);
  return (OK);
}
void ResourceExcavator::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  rememberTerm(biTerm);
  if (((Builtin *) biConst)->isSited())
    (void) processNoGood(biTerm, OK);
}
void ResourceExcavator::processExtension(OZ_Term t)
{
  if (!tagged2Extension(t)->toBePickledV())
    (void) processNoGood(t, NO);
}
Bool ResourceExcavator::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  rememberTerm(objTerm);
  addResource(objTerm);
  return (TRUE);
}
void ResourceExcavator::processLock(OZ_Term lockTerm, Tertiary *tert)
{
  rememberTerm(lockTerm);
  addResource(lockTerm);
}
Bool ResourceExcavator::processCell(OZ_Term cellTerm, Tertiary *tert)
{
  rememberTerm(cellTerm);
  if (cloneCells() && tert->isLocal()) {
    return (NO);
  } else {
    addResource(cellTerm);
    return (OK);
  }
}
void ResourceExcavator::processPort(OZ_Term portTerm, Tertiary *tert)
{
  rememberTerm(portTerm);
  addResource(portTerm);
}
void ResourceExcavator::processResource(OZ_Term rTerm, Tertiary *tert)
{
  rememberTerm(rTerm);
  addResource(rTerm);
}

//
void ResourceExcavator::processVar(OZ_Term cv, OZ_Term *varTerm)
{
  rememberVarLocation(varTerm);
  addResource(makeTaggedRef(varTerm));
}

//
void ResourceExcavator::processRepetition(OZ_Term t, OZ_Term *tPtr,
					  int repNumber) {}
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
  return (NO);
}

//
Bool ResourceExcavator::processDictionary(OZ_Term dictTerm,
					  ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  rememberTerm(dictTerm);
  if (d->isSafeDict()) {
    return (NO);
  } else {
    (void) processNoGood(dictTerm, OK);
    return (OK);
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
    (void) processNoGood(arrayTerm, OK);
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
    (void) processNoGood(classTerm, OK);
    return (OK);		// done - a leaf;
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
    (void) processNoGood(absTerm, OK);
    return (OK);		// done - a leaf;
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
void ResourceExcavator::processSync() {}

//
Pickler pickler;
ResourceExcavator re;
Builder unpickler;

//
//
OZ_Term unpickleTermInternal(PickleMarshalerBuffer *bs)
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
      dif_counter[tag].recv();	// kost@ : TODO: needed?
      //      printf("tag: %d\n", tag);

      switch (tag) {

      case DIF_SMALLINT: 
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term ozInt = OZ_int(unmarshalNumberRobust(bs, &error));
	  RETURN_ON_ERROR(error || !OZ_isSmallInt(ozInt));
#else
	  OZ_Term ozInt = OZ_int(unmarshalNumber(bs));
#endif
	  b->buildValue(ozInt);
	  break;
	}

      case DIF_FLOAT:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  double f = unmarshalFloatRobust(bs, &error);
	  RETURN_ON_ERROR(error);
#else
	  double f = unmarshalFloat(bs);
#endif
	  b->buildValue(OZ_float(f));
	  break;
	}

      case DIF_NAME:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  OZ_Term value;
	  GName *gname    = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
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
	  delete [] printname;
	  break;
	}

      case DIF_COPYABLENAME:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag      = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error || (printname == NULL))
#else
	  int refTag      = unmarshalRefTag(bs);
	  char *printname = unmarshalString(bs);
#endif
	  OZ_Term value;

	  NamedName *aux = NamedName::newCopyableName(strdup(printname));
	  value = makeTaggedLiteral(aux);
	  b->buildValue(value);
	  b->set(value, refTag);
	  delete [] printname;
	  break;
	}

      case DIF_UNIQUENAME:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag      = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error || (printname == NULL));
#else
	  int refTag      = unmarshalRefTag(bs);
	  char *printname = unmarshalString(bs);
#endif
	  OZ_Term value;

	  value = oz_uniqueName(printname);
	  b->buildValue(value);
	  b->set(value, refTag);
	  delete [] printname;
	  break;
	}

      case DIF_ATOM:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  char *aux  = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
	  char *aux  = unmarshalString(bs);
#endif
	  OZ_Term value = OZ_atom(aux);
	  b->buildValue(value);
	  b->set(value, refTag);
	  delete [] aux;
	  break;
	}

      case DIF_BIGINT:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  char *aux  = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error || (aux == NULL));
#else
	  char *aux  = unmarshalString(bs);
#endif
	  b->buildValue(OZ_CStringToNumber(aux));
	  delete [] aux;
	  break;
	}

      case DIF_LIST:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
#endif
	  b->buildListRemember(refTag);
	  break;
	}

      case DIF_TUPLE:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int argno  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
#endif
	  b->buildRecordRemember(refTag);
	  break;
	}

      case DIF_REF:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int i = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error || !b->checkIndexFound(i));
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
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected pickle tag (DIF_OWNER, %d)\n",tag); 
#else
	OZ_warning("unmarshal: unexpected pickle tag (DIF_OWNER, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_RESOURCE_T:
      case DIF_PORT:
      case DIF_THREAD_UNUSED:
      case DIF_SPACE:
      case DIF_CELL:
      case DIF_LOCK:
      case DIF_OBJECT:
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected tertiary (%d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected tertiary (%d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_RESOURCE_N:
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected resource (DIF_RESOURCE_N, %d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected resource (DIF_RESOURCE_N, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_CHUNK:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  OZ_Term value;
	  GName *gname = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  GName *gname = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
	  int flags = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected variable (DIF_VAR, %d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected variable (DIF_VAR, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_FUTURE: 
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected future (DIF_FUTURE, %d)\n",tag);
#else
	OZ_error("unmarshal: unexpected future (DIF_FUTURE, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_VAR_AUTO: 
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected auto var (DIF_VAR_AUTO, %d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected auto var (DIF_VAR_AUTO, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_FUTURE_AUTO: 
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected auto future (DIF_FUTURE_AUTO, %d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected auto future (DIF_FUTURE_AUTO, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_VAR_OBJECT:
#ifdef USE_FAST_UNMARSHALER   
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected object var (DIF_VAR_OBJECT, %d)\n",tag);
#else
	OZ_warning("unmarshal: unexpected object var (DIF_VAR_OBJECT, %d)\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#else
	(void) b->finish();
	return ((OZ_Term) 0);
#endif

      case DIF_PROC:
	{ 
	  OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag    = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  GName *gname  = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
	  int arity     = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  int gsize     = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  int maxX      = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  int line      = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  int column    = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  // in ByteCode"s;
	  int codesize  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  RETURN_ON_ERROR(maxX < 0 || maxX > NumberOfXRegisters); // maxX is the number of X registers used
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
	  case UCR_ERROR:
	    (void) b->finish();
	    return 0;
	  case UCR_DONE:
	    b->finishFillBinary(opaque);
	    break;
	  case UCR_SUSPEND:
	    b->suspendFillBinary(opaque);
	    break;
	  default:
	    Assert(0); return ((OZ_Term) 0);
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int size   = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int low    = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  int high   = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  char *name = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
	  char *name = unmarshalString(bs);
#endif
	  Builtin * found = string2CBuiltin(name);

	  OZ_Term value;
	  if (!found) {
	    OZ_warning("Builtin '%s' not in table.", name);
	    value = oz_nil();
	    delete [] name;
	  } else {
	    if (found->isSited()) {
	      OZ_warning("Unpickling sited builtin: '%s'", name);
	    }
	
	    delete [] name;
	    value = makeTaggedConst(found);
	  }
	  b->buildValue(value);
	  b->set(value, refTag);
	  break;
	}

      case DIF_EXTENSION:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int type = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
	  int type = unmarshalNumber(bs);
#endif
	  OZ_Term value = oz_extension_unmarshal(type, bs);
	  if (value == 0) {
#ifndef USE_FAST_UNMARSHALER
	    (void) b->finish();
	    return ((OZ_Term) 0);
#else
	    OZ_error("Trouble with unmarshaling an extension!");
	    b->buildValue(oz_nil());
	    break;
#endif
	  } else {
	    b->buildValue(value);
	    b->set(value, refTag);
	  }
	  break;
	}

      case DIF_FSETVALUE:
	b->buildFSETValue();
	break;

      case DIF_REF_DEBUG:
#ifndef USE_FAST_UNMARSHALER   
	(void) b->finish();
	return ((OZ_Term) 0);
#else
	OZ_error("not implemented!"); 
	b->buildValue(oz_nil());
	break;
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
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
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
	return ((OZ_Term) 0);
#else
#ifdef DEBUG_CHECK
	OZ_error("unmarshal: unexpected tag: %d\n",tag);
#else
	OZ_warning("unmarshal: unexpected tag: %d\n",tag);
#endif
	b->buildValue(oz_nil());
	break;
#endif
      }
    }
#ifndef USE_FAST_UNMARSHALER   
  }
  CATCH_UNMARSHAL_ERROR { 
    (void) b->finish();
    return ((OZ_Term) 0);
  }
#endif
}

