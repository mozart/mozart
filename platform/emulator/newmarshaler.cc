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

#include "newmarshaler.hh"
#include "boot-manager.hh"


inline int unmarshalRefTag(MsgBuffer *bs)
{
  return unmarshalNumber(bs);
}

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

//
int32* NMMemoryManager::freelist[NMMM_SIZE];

//
// init stuff - must be called;
static Bool isInitialized;
void initNewMarshaler()
{
  NMMemoryManager::init();
  isInitialized = OK;
  Assert(DIF_LAST == 45);  /* new dif(s) added? */
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
      processNoGood(t, NO);	// not remembered!
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

#define HandleTert(string,tert,term,tag,check)		\
    MsgBuffer *bs = (MsgBuffer *) getOpaque();		\
    if (!bs->visit(term)) return;			\
    if (check && ozconf.perdioMinimal) {		\
      processNoGood(term, OK);				\
      rememberNode(term, bs);				\
      return;						\
    }							\
    if (!bs->globalize()) return;			\
    (*marshalTertiary)(tert,tag,bs);			\
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
    rememberNode(cvarTerm, bs);
    return (0);
  }
  processNoGood(makeTaggedRef(cvarTerm), OK);
  rememberNode(cvarTerm, bs);
  return (0);
}

Bool Marshaler::processRepetition(int repNumber)
{
  Assert(repNumber >= 0);
  MsgBuffer *bs = (MsgBuffer *) getOpaque();
  marshalDIF(bs, DIF_REF);
  marshalTermRef(repNumber, bs);
  return (OK);			// nowadays always terminate here;
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
    return (OK);		// done - a leaf;
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
    return (OK);		// done - a leaf;
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
  Reg reg;
  int nxt, line, colum;
  TaggedRef file, predName;
  CodeArea::getDefinitionArgs(start, reg, nxt, file, line, colum, predName);
  //
  marshalNumber(nxt, bs);	// codesize in ByteCode"s;

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

//
//
OZ_Term newUnmarshalTerm(MsgBuffer *bs)
{
  Assert(isInitialized);
  Assert(oz_onToplevel());
  builder.build();

  while(1) {
    Builder *b = &builder;
    MarshalTag tag = (MarshalTag) bs->get();

    dif_counter[tag].recv();	// kost@ : TODO: needed?
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
	b->set(value, refTag);
	delete printname;
	break;
      }

    case DIF_COPYABLENAME:
      {
	int refTag      = unmarshalRefTag(bs);
	char *printname = unmarshalString(bs);
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
      {
	char *aux  = unmarshalString(bs);
	b->buildValue(OZ_CStringToNumber(aux));
	delete aux;
	break;
      }

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

    //
    // kost@ : remember that either all DIF_OBJECT, DIF_VAR_OBJECT and
    // DIF_OWNER are remembered, or none of them is remembered. That's
    // because both 'marshalVariable' and 'marshalObject' could yield
    // 'DIF_OWNER' (see also dpInterface.hh);
    case DIF_OWNER:
    case DIF_OWNER_SEC:
      {
	OZ_Term tert = (*unmarshalOwner)(bs, tag);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(tert, refTag);
	break;
      }

    case DIF_RESOURCE_T:
    case DIF_PORT:
    case DIF_THREAD_UNUSED:
    case DIF_SPACE:
    case DIF_CELL:
    case DIF_LOCK:
    case DIF_OBJECT:
      {
	OZ_Term tert = (*unmarshalTertiary)(bs, tag);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(tert, refTag);
	break;
      }

    case DIF_RESOURCE_N:
      {
	OZ_Term tert = (*unmarshalTertiary)(bs, tag);
	b->buildValue(tert);
	break;
      }
    
    case DIF_CHUNK:
      {
	int refTag = unmarshalRefTag(bs);
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	
	if (gname) {
	  b->buildChunkRemember(gname,refTag);
	} else {
	  b->knownChunk(value);
	  b->set(value,refTag);
	}
	break;
      }

    case DIF_CLASS:
      {
	int refTag = unmarshalRefTag(bs);
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	int flags = unmarshalNumber(bs);

	if (gname) {
	  b->buildClassRemember(gname,flags,refTag);
	} else {
	  b->knownClass(value);
	  b->set(value,refTag);
	}
	break;
      }

    case DIF_VAR: 
      {
	OZ_Term v = (*unmarshalVar)(bs, FALSE, FALSE);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(v, refTag);
	break;
      }

    case DIF_FUTURE: 
      {
	OZ_Term f = (*unmarshalVar)(bs, TRUE, FALSE);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(f, refTag);
	break;
      }

    case DIF_VAR_AUTO: 
      {
	OZ_Term va = (*unmarshalVar)(bs, FALSE, TRUE);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(va, refTag);
	break;
      }

    case DIF_FUTURE_AUTO: 
      {
	OZ_Term fa = (*unmarshalVar)(bs, TRUE, TRUE);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(fa, refTag);
	break;
      }

    case DIF_VAR_OBJECT:
      {
	OZ_Term obj = (*unmarshalTertiary)(bs, tag);
	int refTag = unmarshalRefTag(bs);
	b->buildValueRemember(obj, refTag);
	break;
      }

    case DIF_PROC:
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
	if (unmarshalCode(bs, b, desc))
	  b->finishFillBinary(opaque);
	else
	  b->suspendFillBinary(opaque);
	break;
      }

    case DIF_DICT:
      {
	int refTag = unmarshalRefTag(bs);
	int size   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildDictionaryRemember(size,refTag);
	break;
      }

    case DIF_BUILTIN:
      {
	int refTag = unmarshalRefTag(bs);
	char *name = unmarshalString(bs);
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
      int type = unmarshalNumber(bs);
      OZ_Term value = oz_extension_unmarshal(type,bs);
      if(value == 0) {
	break;  // next value is nogood
      }
      b->buildValue(value);
      break;
    }

    case DIF_FSETVALUE:
      {
	b->buildFSETValue();
	break;
      }

    case DIF_REF_DEBUG:
      { OZ_error("not implemented!"); }

    case DIF_ARRAY:
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

SendRecvCounter dif_counter[DIF_LAST];
SendRecvCounter misc_counter[MISC_LAST];


#include "pickle.cc"
