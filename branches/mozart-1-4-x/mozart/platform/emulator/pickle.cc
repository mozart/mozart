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

#define RETURN_ON_ERROR(ERROR)          	\
  if (ERROR) {					\
    Assert(0);					\
    (void) b->finish();				\
    return ((OZ_Term) 0);			\
}

//
// init stuff - must be called;
static Bool isInitialized;
void initPickleMarshaler()
{
  if (!isInitialized) {
    isInitialized++;
    initRobustMarshaler();	// called once - from here;
  }
}

//
// CodeArea processors for pickling, resource excavation and
// unpickling;
#include "picklecode.cc"

//
inline 
void Pickler::processSmallInt(OZ_Term siTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalSmallInt(bs, siTerm);
}

//
inline 
void Pickler::processFloat(OZ_Term floatTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalFloat(bs, floatTerm);
}

//
inline 
void Pickler::processLiteral(OZ_Term litTerm)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(litTerm, vIT, bs, index, return);

  //
  Literal *lit = tagged2Literal(litTerm);
  if (lit->isAtom()) {
    if (index) {
      marshalDIF(bs, DIF_ATOM_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_ATOM);
    }
    marshalString(bs, ((Atom *) lit)->getPrintName());

  } else if (lit->isUniqueName()) {
    if (index) {
      marshalDIF(bs, DIF_UNIQUENAME_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_UNIQUENAME);
    }
    marshalString(bs, ((NamedName *) lit)->printName);

  } else if (lit->isCopyableName()) {
    if (index) {
      marshalDIF(bs, DIF_COPYABLENAME_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_COPYABLENAME);
    }
    marshalString(bs, ((NamedName *) lit)->printName);

  } else {
    if (index) {
      marshalDIF(bs, DIF_NAME_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_NAME);
    }
    if (lit->isNamedName()) 
      marshalString(bs, ((NamedName *) lit)->printName);
    else
      marshalString(bs, "");
    marshalGName(bs, ((Name *) lit)->globalize());
  }
}

//
inline 
void Pickler::processBigInt(OZ_Term biTerm)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(biTerm, vIT, bs, index, return);

  if (index) {
    marshalDIF(bs, DIF_BIGINT_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_BIGINT);
  }
  marshalString(bs, toC(biTerm));
}

//
inline 
void Pickler::processNoGood(OZ_Term resTerm)
{
  OZ_error("Pickler::processNoGood is called!");
}

//
inline 
void Pickler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(biTerm, vIT, bs, index, return);

  //
  Builtin *bi = (Builtin *) biConst;
  const char *pn = bi->getPrintName();
  Assert(!bi->isSited());

  //
  if (index) {
    marshalDIF(bs, DIF_BUILTIN_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_BUILTIN);
  }
  marshalString(bs, pn);
}

//
inline 
void Pickler::processExtension(OZ_Term et)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  OZ_Extension *ext = tagged2Extension(et);
  Assert(ext->toBePickledV());

  //
  VisitNodeM2ndP(et, vIT, bs, index, return);

  //
  if (index) {
    marshalDIF(bs, DIF_EXTENSION_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_EXTENSION);
  }
  marshalNumber(bs, ext->getIdV());
  // Pickling must be defined for this entity:
  ext->pickleV(bs, this);
}

//
inline 
Bool Pickler::processObject(OZ_Term term, ConstTerm *objConst)
{
  OZ_error("Pickler::processObject is called!");
  return (TRUE);
}

//
inline 
Bool Pickler::processObjectState(OZ_Term term, ConstTerm *objConst)
{
  OZ_error("Pickler::processObjectState is called!");
  return (TRUE);
}

//
inline 
void Pickler::processLock(OZ_Term term, ConstTerm *lockConst)
{
  OZ_error("Pickler::processLock is called!");
}

inline 
Bool Pickler::processCell(OZ_Term term, ConstTerm *cellConst)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();	 

  //
  VisitNodeM2ndP(term, vIT, bs, index, return(OK));

  //
  Assert(cloneCells() && !(static_cast<OzCell*>(cellConst)->isDistributed()));
  if (index) {
    marshalDIF(bs, DIF_CLONEDCELL_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_CLONEDCELL);
  }
  return (NO);
}

inline 
void Pickler::processPort(OZ_Term termcellTerm, ConstTerm *portConst)
{
  OZ_error("Pickler::processPort is called!");
}

inline 
void Pickler::processResource(OZ_Term term, ConstTerm *resConst)
{
  OZ_error("Pickler::processResource is called!");
}

//
inline 
Bool Pickler::processVar(OZ_Term cv, OZ_Term *varTerm)
{
  if (oz_isFailed(cv)) {
    int index;
    PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

    //
    VisitNodeM2ndP(makeTaggedRef(varTerm), vIT, bs, index, return(OK));

    //
    if (index) {
      marshalDIF(bs, DIF_FAILEDVALUE_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_FAILEDVALUE);
    }
    return (NO);
  }

  OZ_error("Pickler::processVar is called!");
  return (TRUE);
}

//
inline 
Bool Pickler::processLTuple(OZ_Term ltupleTerm)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(ltupleTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_LIST_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_LIST);
  }
  return (NO);
}

//
inline 
Bool Pickler::processSRecord(OZ_Term srecordTerm)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(srecordTerm, vIT, bs, index, return(OK));

  //
  SRecord *rec = tagged2SRecord(srecordTerm);
  if (rec->isTuple()) {
    if (index) {
      marshalDIF(bs, DIF_TUPLE_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_TUPLE);
    }
    marshalNumber(bs, rec->getTupleWidth());
  } else {
    if (index) {
      marshalDIF(bs, DIF_RECORD_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIF(bs, DIF_RECORD);
    }
  }

  //
  return (NO);
}

//
inline 
Bool Pickler::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{ 
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  SChunk *ch    = (SChunk *) chunkConst;
  GName *gname  = ch->globalize();
  Assert(gname);

  //
  VisitNodeM2ndP(chunkTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_CHUNK_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_CHUNK);
  }
  marshalGName(bs, gname);

  //
  return (NO);
}

//
inline 
Bool Pickler::processFSETValue(OZ_Term fsetvalueTerm)
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalDIF(bs, DIF_FSETVALUE);
  return (NO);
}

//
inline 
Bool Pickler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(dictTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_DICT_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_DICT);
  }
  //
  OzDictionary *d = (OzDictionary *) dictConst;
  Assert(d->isSafeDict());
  marshalNumber(bs, d->getSize());
  return (NO);
}

inline 
Bool Pickler::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(arrayTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_ARRAY_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_ARRAY);
  }
  //
  OzArray *array = (OzArray *) arrayConst;
  Assert(cloneCells());
  marshalNumber(bs, array->getLow());
  marshalNumber(bs, array->getHigh());
  return (NO);
}

//
inline 
Bool Pickler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(classTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_CLASS_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_CLASS);
  }

  //
  OzClass *cl = (OzClass *) classConst;
  Assert(!cl->isSited());
  GName *gn = cl->globalize();
  Assert(gn);
  marshalGName(bs, gn);
  marshalNumber(bs, cl->getFlags());
  return (NO);
}

//
inline 
Bool Pickler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  int index;
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();

  //
  VisitNodeM2ndP(absTerm, vIT, bs, index, return(OK));

  //
  if (index) {
    marshalDIF(bs, DIF_PROC_DEF);
    marshalTermDef(bs, index);
  } else {
    marshalDIF(bs, DIF_PROC);
  }

  Abstraction *pp = (Abstraction *) absConst;
  GName* gname = pp->globalize();
  Assert(gname);
  PrTabEntry *pred = pp->getPred();
  Assert(!pred->isSited());
  ProgramCounter start;

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
    new MarshalerCodeAreaDescriptor(start, start + nxt, lIT);
  traverseBinary(pickleCode, desc);

  //
  return (NO);
}

//
inline 
void Pickler::processSync()
{
  PickleMarshalerBuffer *bs = (PickleMarshalerBuffer *) getOpaque();
  marshalDIF(bs, DIF_SYNC);
}

//
#define	TRAVERSERCLASS	Pickler
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

//
// 
inline 
void ResourceExcavator::processSmallInt(OZ_Term siTerm) {}
inline 
void ResourceExcavator::processFloat(OZ_Term floatTerm) {}

inline 
void ResourceExcavator::processLiteral(OZ_Term litTerm)
{
  VisitNodeTrav(litTerm, vIT, return);
}
inline 
void ResourceExcavator::processBigInt(OZ_Term biTerm)
{
  VisitNodeTrav(biTerm, vIT, return);
}

//
inline 
void ResourceExcavator::processNoGood(OZ_Term resTerm)
{
  VisitNodeTrav(resTerm, vIT, return);
  addNogood(resTerm);
}
inline 
void ResourceExcavator::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  VisitNodeTrav(biTerm, vIT, return);
  if (((Builtin *) biConst)->isSited()) {
    addNogood(biTerm);
  }
}
inline 
void ResourceExcavator::processExtension(OZ_Term et)
{
  VisitNodeTrav(et, vIT, return);
  if (!tagged2Extension(et)->toBePickledV()) {
    addNogood(et);
  }
}
inline 
Bool ResourceExcavator::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  VisitNodeTrav(objTerm, vIT, return(TRUE));
  addResource(objTerm);
  return (TRUE);
}
inline 
Bool ResourceExcavator::processObjectState(OZ_Term objTerm, ConstTerm *objConst)
{
  VisitNodeTrav(objTerm, vIT, return(TRUE));
  addResource(objTerm);
  return (TRUE);
}
inline 
void ResourceExcavator::processLock(OZ_Term lockTerm, ConstTerm *lockConst)
{
  addResource(lockTerm);
}
inline 
Bool ResourceExcavator::processCell(OZ_Term cellTerm, ConstTerm *cellConst)
{
  VisitNodeTrav(cellTerm, vIT, return(TRUE));
  if (cloneCells() && !(static_cast<OzCell*>(cellConst)->isDistributed())) {
    return (NO);
  } else {
    addResource(cellTerm);
    return (OK);
  }
}
inline 
void ResourceExcavator::processPort(OZ_Term portTerm, ConstTerm *portConst)
{
  addResource(portTerm);
}
inline 
void ResourceExcavator::processResource(OZ_Term rTerm, ConstTerm *resConst)
{
  addResource(rTerm);
}

//
inline 
Bool ResourceExcavator::processVar(OZ_Term v, OZ_Term *vRef)
{
  if (oz_isFailed(v)) {
    return (NO);     // not a resource
  } else {
    addResource(makeTaggedRef(vRef));
    return (OK);
  }
}

//
inline 
Bool ResourceExcavator::processLTuple(OZ_Term ltupleTerm)
{
  VisitNodeTrav(ltupleTerm, vIT, return(TRUE));
  return (NO);
}
inline 
Bool ResourceExcavator::processSRecord(OZ_Term srecordTerm)
{
  VisitNodeTrav(srecordTerm, vIT, return(TRUE));
  return (NO);
}
inline 
Bool ResourceExcavator::processChunk(OZ_Term chunkTerm,
				     ConstTerm *chunkConst)
{
  VisitNodeTrav(chunkTerm, vIT, return(TRUE));
  if (!tagged2SChunk(chunkTerm)->getValue()) {
    addResource(chunkTerm);
    return (OK);
  }
  return (NO);
}

//
inline 
Bool ResourceExcavator::processFSETValue(OZ_Term fsetvalueTerm)
{
  return (NO);
}

//
inline Bool
ResourceExcavator::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  VisitNodeTrav(dictTerm, vIT, return(TRUE));
  OzDictionary *d = (OzDictionary *) dictConst;
  if (d->isSafeDict()) {
    return (NO);
  } else {
    addNogood(dictTerm);
    return (OK);
  }
}

//
inline Bool
ResourceExcavator::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  VisitNodeTrav(arrayTerm, vIT, return(TRUE));
  if (cloneCells()) {
    return (NO);
  } else {
    addNogood(arrayTerm);
    return (OK);
  }
}

//
inline Bool
ResourceExcavator::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  VisitNodeTrav(classTerm, vIT, return(TRUE));
  OzClass *cl = (OzClass *) classConst;
  if (cl->isSited()) {
    addNogood(classTerm);
    return (OK);		// done - a leaf;
  } else if (!cl->isComplete()) {
    addResource(classTerm);
    return (OK);
  } else {
    return (NO);
  }
}

//
inline Bool
ResourceExcavator::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  VisitNodeTrav(absTerm, vIT, return(TRUE));

  //
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();
  //
  if (pred->isSited()) {
    addNogood(absTerm);
    return (OK);		// done - a leaf;
  } else {
    //
    ProgramCounter start = pp->getPC() - sizeOf(DEFINITION);
    XReg reg;
    int nxt, line, colum;
    TaggedRef file, predName;
    CodeArea::getDefinitionArgs(start, reg, nxt, file,
				line, colum, predName);

    //
    MarshalerCodeAreaDescriptor *desc = 
      new MarshalerCodeAreaDescriptor(start, start + nxt, 
				      (AddressHashTableO1Reset *) 0);
    traverseBinary(traverseCode, desc);
    return (NO);
  }
  Assert(0);
}

//
inline 
void ResourceExcavator::processSync() {}

//
#define	TRAVERSERCLASS	ResourceExcavator
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

//
static MarshalerDict md(ValuesITInitSize);
ResourceExcavator re(&md);
Pickler pickler(&md);
Builder unpickler;

//
//
OZ_Term unpickleTermInternal(PickleMarshalerBuffer *bs)
{
  Assert(isInitialized);
  Assert(oz_onToplevel());
  Builder *b;

  while(1) {
    b = &unpickler;
    MarshalTag tag = (MarshalTag) bs->get();
    Assert(tag < DIF_LAST);
    dif_counter[tag].recv();	// kost@ : TODO: needed?
    //      printf("tag: %d\n", tag);

    switch (tag) {

    case DIF_SMALLINT: 
      {
	OZ_Term ozInt = OZ_int(unmarshalNumber(bs));
	b->buildValue(ozInt);
	break;
      }

    case DIF_FLOAT:
      {
	double f = unmarshalFloat(bs);
	b->buildValue(OZ_float(f));
	break;
      }

    case DIF_NAME_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	char *printname = unmarshalString(bs);
	OZ_Term value;
	GName *gname    = unmarshalGName(&value, bs);

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
	b->setTerm(value, refTag);
	delete [] printname;
	break;
      }

    case DIF_NAME:
      {
	char *printname = unmarshalString(bs);
	OZ_Term value;
	GName *gname    = unmarshalGName(&value, bs);

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
	delete [] printname;
	break;
      }

    case DIF_COPYABLENAME_DEF:
      {
	int refTag      = unmarshalRefTag(bs);
	char *printname = unmarshalString(bs);
	OZ_Term value;

	NamedName *aux = NamedName::newCopyableName(strdup(printname));
	value = makeTaggedLiteral(aux);
	b->buildValue(value);
	b->setTerm(value, refTag);
	delete [] printname;
	break;
      }

    case DIF_COPYABLENAME:
      {
	char *printname = unmarshalString(bs);
	OZ_Term value;

	NamedName *aux = NamedName::newCopyableName(strdup(printname));
	value = makeTaggedLiteral(aux);
	b->buildValue(value);
	delete [] printname;
	break;
      }

    case DIF_UNIQUENAME_DEF:
      {
	int refTag      = unmarshalRefTag(bs);
	char *printname = unmarshalString(bs);
	OZ_Term value;

	value = oz_uniqueName(printname);
	b->buildValue(value);
	b->setTerm(value, refTag);
	delete [] printname;
	break;
      }

    case DIF_UNIQUENAME:
      {
	char *printname = unmarshalString(bs);
	OZ_Term value;

	value = oz_uniqueName(printname);
	b->buildValue(value);
	delete [] printname;
	break;
      }

    case DIF_ATOM_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	char *aux  = unmarshalString(bs);
	OZ_Term value = OZ_atom(aux);
	b->buildValue(value);
	b->setTerm(value, refTag);
	delete [] aux;
	break;
      }

    case DIF_ATOM:
      {
	char *aux  = unmarshalString(bs);
	OZ_Term value = OZ_atom(aux);
	b->buildValue(value);
	delete [] aux;
	break;
      }

    case DIF_BIGINT:
      {
	char *aux  = unmarshalString(bs);
	b->buildValue(OZ_CStringToNumber(aux));
	delete [] aux;
	break;
      }

    case DIF_BIGINT_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	char *aux  = unmarshalString(bs);
	OZ_Term value = OZ_CStringToNumber(aux);
	b->buildValue(value);
	b->setTerm(value, refTag);
	delete [] aux;
	break;
      }

    case DIF_LIST_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	b->buildListRemember(refTag);
	break;
      }

    case DIF_LIST:
      b->buildList();
      break;

    case DIF_TUPLE_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int argno  = unmarshalNumber(bs);
	b->buildTupleRemember(argno, refTag);
	break;
      }

    case DIF_TUPLE:
      {
	int argno  = unmarshalNumber(bs);
	b->buildTuple(argno);
	break;
      }

    case DIF_RECORD_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	b->buildRecordRemember(refTag);
	break;
      }

    case DIF_RECORD:
      b->buildRecord();
      break;

    case DIF_REF:
      {
	int i = unmarshalNumber(bs);
	b->buildValueRef(i);
	break;
      }

      //
    case DIF_OWNER:
#ifdef DEBUG_CHECK
      OZ_error("unmarshal: unexpected pickle tag (DIF_OWNER, %d)\n",tag); 
#else
      OZ_warning("unmarshal: unexpected pickle tag (DIF_OWNER, %d)\n",tag);
#endif
      b->buildValue(oz_nil());
      break;

    case DIF_PORT_DEF:
    case DIF_PORT:
    case DIF_CELL_DEF:
    case DIF_CELL:
    case DIF_LOCK_DEF:
    case DIF_LOCK:
    case DIF_OBJECT_DEF:
    case DIF_OBJECT:
#ifdef DEBUG_CHECK
      OZ_error("unmarshal: unexpected term (%d)\n",tag);
#else
      OZ_warning("unmarshal: unexpected term (%d)\n",tag);
#endif
      b->buildValue(oz_nil());
      break;

    case DIF_RESOURCE_DEF:
    case DIF_RESOURCE:
#ifdef DEBUG_CHECK
      OZ_error("unmarshal: unexpected resource (DIF_RESOURCE, %d)\n",tag);
#else
      OZ_warning("unmarshal: unexpected resource (DIF_RESOURCE, %d)\n",tag);
#endif
      b->buildValue(oz_nil());
      break;

    case DIF_CHUNK_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	if (gname) {
	  b->buildChunkRemember(gname, refTag);
	} else {
	  b->knownChunk(value);
	  b->setTerm(value, refTag);
	}
	break;
      }

    case DIF_CHUNK:
      {
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	if (gname) {
	  b->buildChunk(gname);
	} else {
	  b->knownChunk(value);
	}
	break;
      }

    case DIF_CLASS_DEF:
      {
	OZ_Term value;
	int refTag = unmarshalRefTag(bs);
	GName *gname = unmarshalGName(&value, bs);
	int flags = unmarshalNumber(bs);
	if (gname) {
	  b->buildClassRemember(gname,flags,refTag);
	} else {
	  b->knownClass(value);
	  b->setTerm(value,refTag);
	}
	break;
      }

    case DIF_CLASS:
      {
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	int flags = unmarshalNumber(bs);
	if (gname) {
	  b->buildClass(gname, flags);
	} else {
	  b->knownClass(value);
	}
	break;
      }

    case DIF_VAR_OBJECT:
    case DIF_VAR_OBJECT_DEF:
#ifdef DEBUG_CHECK
      OZ_error("unmarshal: unexpected object var (DIF_VAR_OBJECT, %d)\n",tag);
#else
      OZ_warning("unmarshal: unexpected object var (DIF_VAR_OBJECT, %d)\n",tag);
#endif
      b->buildValue(oz_nil());
      break;

    case DIF_PROC_DEF:
      { 
	OZ_Term value;
	int refTag    = unmarshalRefTag(bs);
	GName *gname  = unmarshalGName(&value, bs);
	int arity     = unmarshalNumber(bs);
	int gsize     = unmarshalNumber(bs);
	int maxX      = unmarshalNumber(bs);
	int line      = unmarshalNumber(bs);
	int column    = unmarshalNumber(bs);
	int codesize  = unmarshalNumber(bs); // in ByteCode"s;

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

    case DIF_PROC:
      { 
	OZ_Term value;
	GName *gname  = unmarshalGName(&value, bs);
	int arity     = unmarshalNumber(bs);
	int gsize     = unmarshalNumber(bs);
	int maxX      = unmarshalNumber(bs);
	int line      = unmarshalNumber(bs);
	int column    = unmarshalNumber(bs);
	int codesize  = unmarshalNumber(bs); // in ByteCode"s;

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
	  b->buildProc(gname, arity, gsize, maxX, line, column, pc);
	} else {
	  Assert(oz_isAbstraction(oz_deref(value)));
	  // ('zero' descriptions are not allowed;)
	  BuilderCodeAreaDescriptor *desc =
	    new BuilderCodeAreaDescriptor(0, 0, 0);
	  b->buildBinary(desc);

	  //
	  b->knownProc(value);
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
	if (unpickleCode(bs, b, desc))
	  b->finishFillBinary(opaque);
	else
	  b->suspendFillBinary(opaque);
	break;
      }

    case DIF_DICT_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int size   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildDictionaryRemember(size,refTag);
	break;
      }

    case DIF_DICT:
      {
	int size   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildDictionary(size);
	break;
      }

    case DIF_ARRAY_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int low    = unmarshalNumber(bs);
	int high   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildArrayRemember(low, high, refTag);
	break;
      }

    case DIF_ARRAY:
      {
	int low    = unmarshalNumber(bs);
	int high   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildArray(low, high);
	break;
      }

    case DIF_BUILTIN_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	char *name = unmarshalString(bs);
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
	b->setTerm(value, refTag);
	break;
      }

    case DIF_BUILTIN:
      {
	char *name = unmarshalString(bs);
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
	break;
      }

    case DIF_EXTENSION_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int type = unmarshalNumber(bs);
	OZ_Term value = oz_extension_unmarshal(type, bs, b);
	if (value == 0) {
	  OZ_error("Trouble with unmarshaling an extension!");
	  b->buildValue(oz_nil());
	  break;
	} else {
	  b->buildValue(value);
	  b->setTerm(value, refTag);
	}
	break;
      }

    case DIF_EXTENSION:
      {
	int type = unmarshalNumber(bs);
	OZ_Term value = oz_extension_unmarshal(type, bs, b);
	if (value == 0) {
	  OZ_error("Trouble with unmarshaling an extension!");
	  b->buildValue(oz_nil());
	  break;
	} else {
	  b->buildValue(value);
	}
	break;
      }

    case DIF_FSETVALUE:
      b->buildFSETValue();
      break;

      //
      // 'DIF_SYNC' and its handling is a part of the interface
      // between the builder object and the unmarshaler itself:
    case DIF_SYNC:
      b->processSync();
      break;

    case DIF_CLONEDCELL_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	b->buildClonedCellRemember(refTag);
	break;
      }

    case DIF_CLONEDCELL:
      b->buildClonedCell();
      break;

    case DIF_FAILEDVALUE:
      b->buildFailedValue();
      break;

    case DIF_FAILEDVALUE_DEF: {
      int refTag = unmarshalRefTag(bs);
      b->buildFailedValueRemember(refTag);
      break;
    }

    case DIF_EOF: 
      return (b->finish());

    case DIF_UNUSED0:
    case DIF_UNUSED1:
    case DIF_UNUSED2:
    case DIF_UNUSED3:
    case DIF_UNUSED4:
    case DIF_UNUSED5:
    case DIF_UNUSED6:
    case DIF_UNUSED7:
    case DIF_UNUSED8:
      OZ_error("unmarshal: unexpected UNUSED tag: %d\n",tag);
      b->buildValue(oz_nil());
      break;

    case DIF_VAR:
    case DIF_VAR_DEF:
    case DIF_VAR_AUTO:
    case DIF_VAR_AUTO_DEF:
    case DIF_READONLY:
    case DIF_READONLY_DEF:
    case DIF_READONLY_AUTO:
    case DIF_READONLY_AUTO_DEF:
      OZ_error("unmarshal: unexpected var/readonly tag: %d\n",tag);
      b->buildValue(oz_nil());
      break;

    default:
      OZ_error("unmarshal: unexpected tag: %d\n",tag);
      b->buildValue(oz_nil());
      break;
    }
  }
}

