/*
 *  Authors:
 *    Konstantin Popov (kost@sics.se)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Contributors:
 *    Per Brand (perbrand@sics.se)
 *    Michael Mehl (mehl@dfki.de)
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
#pragma implementation "newmarshaler.hh"
#endif

#include <math.h>
#include "newmarshaler.hh"
#include "boot-manager.hh"


GName *globalizeConst(ConstTerm *t, MsgBuffer *bs)
{
  if (!bs->globalize())
    return 0;

  switch(t->getType()) {
  case Co_Object:      return ((Object*)t)->globalize();
  case Co_Class:       return ((ObjectClass*)t)->globalize();
  case Co_Chunk:       return ((SChunk*)t)->globalize();
  case Co_Abstraction: return ((Abstraction*)t)->globalize();
  default: Assert(0); return NULL;
  }
}

void marshalGName(GName *gname, MsgBuffer *bs)
{
  if (gname==NULL)
    return;

  misc_counter[MISC_GNAME].send();

  gname->site->marshalSiteForGName(bs);
  for (int i=0; i<fatIntDigits; i++) {
    marshalNumber(gname->id.number[i],bs);
  }
  marshalNumber((int)gname->gnameType,bs);
}


void unmarshalGName1(GName *gname, MsgBuffer *bs)
{
  gname->site=unmarshalSite(bs);
  for (int i=0; i<fatIntDigits; i++) {
    gname->id.number[i] = unmarshalNumber(bs);
  }
  gname->gnameType = (GNameType) unmarshalNumber(bs);
}
void unmarshalGName1Robust(GName *gname, MsgBuffer *bs, int *error)
{
  int e1,e2;
  gname->site=unmarshalSiteRobust(bs, &e1);
  for (int i=0; (i<fatIntDigits && !e1); i++) {
    int num, e;
    num = unmarshalNumberRobust(bs, &e);
    e1 = e || (num > maxDigit);
    gname->id.number[i] = num;
  }
  gname->gnameType = (GNameType) unmarshalNumberRobust(bs, &e2);
  *error = e1 || e2 || (gname->gnameType > MAX_GNT);
}

GName *unmarshalGName(TaggedRef *ret, MsgBuffer *bs)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1(&gname,bs);

  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}
GName *unmarshalGNameRobust(TaggedRef *ret, MsgBuffer *bs, int *error)
{
  misc_counter[MISC_GNAME].recv();
  GName gname;
  unmarshalGName1Robust(&gname,bs,error);

  TaggedRef aux = oz_findGName(&gname);
  if (aux) {
    if (ret) *ret = aux; // ATTENTION
    return 0;
  }
  return new GName(gname);
}

//
int32* NMMemoryManager::freelist[NMMM_SIZE];

int RobustMarshaler_Max_Shift;
int RobustMarshaler_Max_Hi_Byte;
//
// Stuff needed for to check that no overflow is done in unmarshalNumber()
void initRobustMarshaler()
{
  int intsize = sizeof(int);
  int shft = intsize*7;
  while(shft <= (intsize*8)-7) shft += 7;
  RobustMarshaler_Max_Shift = shft;
  RobustMarshaler_Max_Hi_Byte =
    (int) pow(2, (intsize*8)-RobustMarshaler_Max_Shift);
}

//
// init stuff - must be called;
static Bool isInitialized;
void initNewMarshaler()
{
  NMMemoryManager::init();
  isInitialized = OK;
  Assert(DIF_LAST == 45);  /* new dif(s) added? */
  initRobustMarshaler();
}

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
  rememberNode(litTerm, bs);
  marshalString(name, bs);
  marshalGName(gname, bs);
}

void Marshaler::processBigInt(OZ_Term biTerm, ConstTerm *biConst)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  marshalDIF(bs,DIF_BIGINT);
  marshalString(toC(biTerm),bs);
}


void Marshaler::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (bs->visit(biTerm)) {
    Builtin *bi= (Builtin *)biConst;
    if (bi->isSited()) {
      processNoGood(biTerm, OK);
      rememberNode(biTerm, bs);
      return;
    }

    //
    marshalDIF(bs,DIF_BUILTIN);
    rememberNode(biTerm,bs);
    marshalString(bi->getPrintName(),bs);
  }
}


void Marshaler::processNoGood(OZ_Term resTerm, Bool trail)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (ozconf.perdioMinimal || bs->getSite()==NULL) {
    bs->addNogood(resTerm);
  } else{
    (*marshalSPP)(resTerm,bs,trail);
  }
}

void Marshaler::processExtension(OZ_Term t)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (bs->visit(t)) {
    marshalDIF(bs,DIF_EXTENSION);
    marshalNumber(oz_tagged2Extension(t)->getIdV(),bs);
    if (!oz_tagged2Extension(t)->marshalV(bs)) {
      processNoGood(t, NO);     // not remembered!
    }
  }
}

void Marshaler::processObject(OZ_Term term, ConstTerm *objConst)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (bs->visit(term)) {
    Object *o = (Object*) objConst;
    if(ozconf.perdioMinimal || o->getClass()->isSited()) {
      processNoGood(term, OK);
      rememberNode(term, bs);
      return;
    }
    if (!bs->globalize()) return;
    (*marshalObject)(o, bs);
    rememberNode(term, bs);
  }
}

#define HandleTert(string,tert,term,tag,check)          \
    MsgBuffer *bs = (MsgBuffer *) getOpaque();          \
    if (!bs->visit(term)) return;                       \
    if (check && ozconf.perdioMinimal) {                \
      processNoGood(term, OK);                          \
      rememberNode(term, bs);                           \
      return;                                           \
    }                                                   \
    if (!bs->globalize()) return;                       \
    (*marshalTertiary)(tert,tag,bs);                    \
    rememberNode(term, bs);


void Marshaler::processLock(OZ_Term term, Tertiary *tert)
{
  HandleTert("lock",tert,term,DIF_LOCK,OK);
}


void Marshaler::processCell(OZ_Term term, Tertiary *tert)
{
  HandleTert("cell",tert,term,DIF_CELL,OK);
}

void Marshaler::processPort(OZ_Term term, Tertiary *tert)
{
  HandleTert("port",tert,term,DIF_PORT,NO);
}

void Marshaler::processResource(OZ_Term term, Tertiary *tert)
{
  HandleTert("resource",tert,term,DIF_RESOURCE_T,OK);
}

#undef HandleTert


void Marshaler::processUVar(OZ_Term *uvarTerm)
{
#ifdef DEBUG_CHECK
  OZ_Term term = processCVar(uvarTerm);
  Assert(term == (OZ_Term) 0);
#else
  (void) processCVar(uvarTerm);
#endif
}

//
OZ_Term Marshaler::processCVar(OZ_Term *cvarTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (!bs->visit(makeTaggedRef(cvarTerm))) return (0);
  if((*triggerVariable)(cvarTerm))
    return (makeTaggedRef(cvarTerm));
  if((*marshalVariable)(cvarTerm, bs)) {
    rememberNode(*cvarTerm, bs); // kost@24.08.99: we remembered ptr!!!
    return (0);
  }
  processNoGood(makeTaggedRef(cvarTerm), OK);
  rememberNode(*cvarTerm, bs);  // kost@24.08.99: we remembered ptr!!!
  return (0);
}

Bool Marshaler::processRepetition(int repNumber)
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

  marshalDIF(bs, DIF_LIST);
  rememberNode(ltupleTerm, bs);

  return (NO);
}

Bool Marshaler::processSRecord(OZ_Term srecordTerm)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  SRecord *rec = tagged2SRecord(srecordTerm);
  TaggedRef label = rec->getLabel();

  if (rec->isTuple()) {
    marshalDIF(bs, DIF_TUPLE);
    rememberNode(srecordTerm, bs);
    marshalNumber(rec->getTupleWidth(), bs);
  } else {
    marshalDIF(bs, DIF_RECORD);
    rememberNode(srecordTerm, bs);
  }

  return (NO);
}


Bool Marshaler::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  SChunk *ch    = (SChunk *) chunkConst;
  GName *gname  = globalizeConst(ch,bs);
  marshalDIF(bs,DIF_CHUNK);
  rememberNode(chunkTerm, bs);
  marshalGName(gname,bs);
  return NO;
}


#define CheckD0Compatibility(Term) \
   if (ozconf.perdioMinimal) { processNoGood(Term,NO); return OK; }


Bool Marshaler::processFSETValue(OZ_Term fsetvalueTerm)
{
  CheckD0Compatibility(fsetvalueTerm);

  MsgBuffer *bs = (MsgBuffer *) getOpaque();

  if (!bs->visit(fsetvalueTerm))
    return OK;

  marshalDIF(bs,DIF_FSETVALUE);
  return NO;
}

Bool Marshaler::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  OzDictionary *d = (OzDictionary *) dictConst;
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (!d->isSafeDict()) {
    processNoGood(dictTerm, OK);
    rememberNode(dictTerm, bs);
    return OK;
  } else {
    marshalDIF(bs,DIF_DICT);
    rememberNode(dictTerm, bs);
    marshalNumber(d->getSize(),bs);
    return (NO);
  }
}

Bool Marshaler::processClass(OZ_Term classTerm, ConstTerm *classConst)
{
  ObjectClass *cl = (ObjectClass *) classConst;
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  if (cl->isSited()) {
    processNoGood(classTerm, OK);
    rememberNode(classTerm, bs);
    return (OK);                // done - a leaf;
  }

  //
  marshalDIF(bs, DIF_CLASS);
  GName *gn = globalizeConst(cl, bs);
  rememberNode(classTerm, bs);
  marshalGName(gn, bs);
  marshalNumber(cl->getFlags(), bs);
  return (NO);
}


Bool Marshaler::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  ProgramCounter start;

  //
  Abstraction *pp = (Abstraction *) absConst;
  PrTabEntry *pred = pp->getPred();
  if (pred->isSited()) {
    processNoGood(absTerm, OK);
    rememberNode(absTerm, bs);
    return (OK);                // done - a leaf;
  }

  //
  GName* gname = globalizeConst(pp, bs);

  //
  marshalDIF(bs, DIF_PROC);
  rememberNode(absTerm, bs);

  //
  marshalGName(gname, bs);
  marshalNumber(pp->getArity(), bs);
  ProgramCounter pc = pp->getPC();
  int gs = pred->getGSize();
  marshalNumber(gs, bs);
  marshalNumber(pred->getMaxX(), bs);
  marshalNumber(pred->getLine(), bs);
  marshalNumber(pred->getColumn(), bs);

  //
  start = pp->getPC() - sizeOf(DEFINITION);

  //
  XReg reg;
  int nxt, line, colum;
  TaggedRef file, predName;
  CodeArea::getDefinitionArgs(start, reg, nxt, file, line, colum, predName);
  //
  marshalNumber(nxt, bs);       // codesize in ByteCode"s;

  //
  MarshalerCodeAreaDescriptor *desc =
    new MarshalerCodeAreaDescriptor(start, start + nxt);
  marshalBinary(newMarshalCode, desc);

  //
  return (NO);
}

#include "newmarshalcode.cc"

//
Marshaler marshaler;
Builder builder;

// for newUnmarshalTerm see unmarshaling.cc
#include "robust_unmarshaling.cc"
#include "fast_unmarshaling.cc"

SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];


#include "pickle.cc"
