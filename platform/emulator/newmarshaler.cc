/*
 *  Authors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
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
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifdef INTERFACE
#pragma implementation "newmarshaler.hh"
#endif

#include "newmarshaler.hh"

//
#define RememberNode(node,bs)                   \
  int ind = remember(node);                     \
  marshalTermDef(ind, bs);

//
void Marshaler::processSmallInt(OZ_Term siTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  marshalDIF(bs, DIF_SMALLINT);
  marshalNumber(smallIntValue(siTerm), bs);
}

void Marshaler::processFloat(OZ_Term floatTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  marshalDIF(bs, DIF_FLOAT);
  marshalFloat(tagged2Float(floatTerm)->getValue(), bs);
}

void Marshaler::processLiteral(OZ_Term litTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  Literal *lit = tagged2Literal(litTerm);

  MarshalTag litTag;
  GName *gname = NULL;

  if (lit->isAtom()) {
    litTag = DIF_ATOM;
  } else if (lit->isUniqueName()) {
    litTag = DIF_UNIQUENAME;
  } else if (lit->isCopyableName()) {
    litTag = DIF_COPYABLENAME;
  } else {
    litTag = DIF_NAME;
    if (bs->globalize())
      gname = ((Name *) lit)->globalize();
  }

  marshalDIF(bs, litTag);
  const char *name = lit->getPrintName();
  RememberNode(litTerm, bs);
  marshalString(name, bs);
  marshalGName(gname, bs);
}

void Marshaler::processExtension(OZ_Term extensionTerm)
{ OZ_error("not implemented!"); }

void Marshaler::processBigInt(OZ_Term biTerm, ConstTerm *biConst)
{ OZ_error("not implemented!"); }

void Marshaler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{ OZ_error("not implemented!"); }

void Marshaler::processObject(OZ_Term objTerm, ConstTerm *objConst)
{ OZ_error("not implemented!"); }

void Marshaler::processLock(OZ_Term lockTerm, Tertiary *lockTert)
{ OZ_error("not implemented!"); }

void Marshaler::processCell(OZ_Term cellTerm, Tertiary *cellTert)
{ OZ_error("not implemented!"); }

void Marshaler::processPort(OZ_Term portTerm, Tertiary *portTert)
{ OZ_error("not implemented!"); }

void Marshaler::processResource(OZ_Term resTerm, Tertiary *resTert)
{ OZ_error("not implemented!"); }

void Marshaler::processNoGood(OZ_Term resTerm)
{ OZ_error("not implemented!"); }

void Marshaler::processUVar(OZ_Term uvarTerm)
{ OZ_error("not implemented!"); }

void Marshaler::processCVar(OZ_Term cvarTerm)
{ OZ_error("not implemented!"); }

Bool Marshaler::processRepetition(OZ_Term term, int repNumber)
{
  Assert(repNumber >= 0);
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  marshalDIF(bs, DIF_REF);
  marshalTermRef(repNumber, bs);
  return (OK);                  // nowadays always terminate here;
}

Bool Marshaler::processLTuple(OZ_Term ltupleTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  LTuple *l = tagged2LTuple(ltupleTerm);

  marshalDIF(bs, DIF_LIST);
  RememberNode(ltupleTerm, bs);

  return (NO);
}

Bool Marshaler::processSRecord(OZ_Term srecordTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  SRecord *rec = tagged2SRecord(srecordTerm);
  TaggedRef label = rec->getLabel();

  if (rec->isTuple()) {
    marshalDIF(bs, DIF_TUPLE);
    RememberNode(srecordTerm, bs);
    marshalNumber(rec->getTupleWidth(), bs);
  } else {
    marshalDIF(bs, DIF_RECORD);
    RememberNode(srecordTerm, bs);
  }

  return (NO);
}

Bool Marshaler::processFSETValue(OZ_Term fsetvalueTerm)
{ OZ_error("not implemented!"); return(OK); }

Bool Marshaler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{ OZ_error("not implemented!"); return(OK); }

Bool Marshaler::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{ OZ_error("not implemented!"); return(OK); }

Bool Marshaler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ OZ_error("not implemented!"); return(OK); }

Bool Marshaler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{ OZ_error("not implemented!"); return(OK); }

//
Marshaler marshaler;
Builder builder;

//
//
OZ_Term newUnmarshalTerm(MsgBuffer *bs)
{
  builder.build();

  while(1) {
    Builder *b = &builder;
    MarshalTag tag = (MarshalTag) bs->get();

    dif_counter[tag].recv();    // kost@ : TODO: needed?
    switch(tag) {

    case DIF_SMALLINT:
      b->buildValue(OZ_int(unmarshalNumber(bs)));
      break;

    case DIF_FLOAT:
      b->buildValue(OZ_float(unmarshalFloat(bs)));
      break;

    case DIF_NAME:
      {
        int refTag = unmarshalRefTag(bs);
        GName *gname;
        char *printname;
        OZ_Term value;

        printname = unmarshalString(bs);
        gname     = unmarshalGName(&value, bs);

        if (gname) {
          Name *aux;
          if (strcmp("", printname) == 0) {
            aux = Name::newName(am.currentBoard());
          } else {
            aux = NamedName::newNamedName(ozstrdup(printname));
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
        int refTag      = unmarshalRefTag(bs);
        char *printname = unmarshalString(bs);
        OZ_Term value;

        NamedName *aux = NamedName::newCopyableName(ozstrdup(printname));
        value = makeTaggedLiteral(aux);
        b->buildValue(value);
        b->set(value, refTag);
        delete printname;
        break;
      }

    case DIF_UNIQUENAME:
      {
        int refTag      = unmarshalRefTag(bs);
        char *printname = unmarshalString(bs);
        OZ_Term value;

        value = oz_uniqueName(printname);
        b->buildValue(value);
        b->set(value, refTag);
        delete printname;
        break;
      }

    case DIF_ATOM:
      {
        int refTag = unmarshalRefTag(bs);
        char *aux  = unmarshalString(bs);
        OZ_Term value = OZ_atom(aux);
        b->buildValue(value);
        b->set(value, refTag);
        delete aux;
        break;
      }

    case DIF_BIGINT:
      { OZ_error("not implemented!"); }

    case DIF_LIST:
      {
        int refTag = unmarshalRefTag(bs);
        b->buildListRemember(refTag);
        break;
      }

    case DIF_TUPLE:
      {
        int refTag = unmarshalRefTag(bs);
        int argno  = unmarshalNumber(bs);
        b->buildTupleRemember(argno, refTag);
        break;
      }

    case DIF_RECORD:
      {
        int refTag = unmarshalRefTag(bs);
        b->buildRecordRemember(refTag);
        break;
      }

    case DIF_REF:
      {
        int i = unmarshalNumber(bs);
        b->buildValue(b->get(i));
        break;
      }

    case DIF_REF_DEBUG:
      { OZ_error("not implemented!"); }

    case DIF_OWNER:
    case DIF_OWNER_SEC:
      { OZ_error("not implemented!"); }
    case DIF_RESOURCE_T:
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
    case DIF_CELL:
    case DIF_LOCK:
    case DIF_OBJECT:
      { OZ_error("not implemented!"); }
    case DIF_RESOURCE_N:
      { OZ_error("not implemented!"); }

    case DIF_CHUNK:
      { OZ_error("not implemented!"); }

    case DIF_CLASS:
      { OZ_error("not implemented!"); }

    case DIF_VAR:
      { OZ_error("not implemented!"); }

    case DIF_FUTURE:
      { OZ_error("not implemented!"); }

    case DIF_VAR_AUTO:
      { OZ_error("not implemented!"); }

    case DIF_FUTURE_AUTO:
      { OZ_error("not implemented!"); }

    case DIF_PROC:
      { OZ_error("not implemented!"); }

    case DIF_DICT:
      { OZ_error("not implemented!"); }

    case DIF_ARRAY:
      { OZ_error("not implemented!"); }

    case DIF_BUILTIN:
      { OZ_error("not implemented!"); }

    case DIF_FSETVALUE:
      { OZ_error("not implemented!"); }

    case DIF_EXTENSION:
      { OZ_error("not implemented!"); }

    case DIF_EOF:
      return (b->finish());

    default:
      OZ_error("unmarshal: unexpected tag: %d\n",tag);
      Assert(0);
      b->buildValue(oz_nil());
    }
  }
  Assert(0);
}
