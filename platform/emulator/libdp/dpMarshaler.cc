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
        if(ERROR) { Assert(0); (void) b->finish(); return ((OZ_Term) 0); }

//
// #define DBG_TRACE

//
#if defined(DBG_TRACE)
static char dbgname[128];
static FILE *dbgout = (FILE *) 0;

#define DBGINIT()						\
  if (dbgout == (FILE *) 0) {					\
    sprintf(dbgname, "/tmp/dpm-dbg-%d.txt", osgetpid());       	\
    dbgout = fopen(dbgname, "a+");				\
  }

#define DBG_TRACE_CODE(C) C
#else
#define DBG_TRACE_CODE(C)
#endif

#if defined(DEBUG_CHECK)
void DPMarshaler::appTCheck(OZ_Term term)
{
  Assert(mts);
  mts->checkVar(term);
}
#endif

// The count of marshaled diffs should be done for the distributed
// messages and not for other marshaled structures. 
// Erik
inline void marshalDIFcounted(MarshalerBuffer *bs, MarshalTag tag) {
  dif_counter[tag].send();
  marshalDIF(bs,tag);
}
//
void DPMarshaler::processSmallInt(OZ_Term siTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  // 
  // The current term is not allowed to occupy the remaining space in
  // the buffer completely, but leave a space for 'DIF_SUSPEND';
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_SMALLINT].name, DIF_SMALLINT, toC(siTerm));
    fflush(dbgout);
#endif
    marshalSmallInt(bs, siTerm);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(siTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(siTerm);
  }
}

//
void DPMarshaler::processFloat(OZ_Term floatTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MFloatMaxSize) {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_FLOAT].name, DIF_FLOAT, toC(floatTerm));
    fflush(dbgout);
#endif
    marshalFloat(bs, floatTerm);
  } else {
    dif_counter[DIF_SUSPEND].send();
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(floatTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
  dif_counter[DIF_LIT_CONT].send();
  marshalDIFcounted(bs, DIF_LIT_CONT);
#if defined(DBG_TRACE)
  {
    DBGINIT();
    DPMarshalerLitSusp *desc = (DPMarshalerLitSusp *) arg;
    char buf[10];
    buf[0] = (char) 0;
    strncat(buf, desc->getRemainingString(),
	    min(10, desc->getCurrentSize()));
    fprintf(dbgout, "> tag: %s(%d) = %.10s\n",
	    dif_names[DIF_LIT_CONT].name, DIF_LIT_CONT, buf);
    fflush(dbgout);
  }
#endif
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
    dif_counter[litTag].send();
    marshalDIFcounted(bs, litTag);
    const char *name = lit->getPrintName();
    marshalTermDef(bs, litTermInd);
    marshalNumber(bs, nameSize);
    if (gname) marshalGName(bs, gname);

    //
    // Observe: the format is different from pickles!
    DPMarshalerLitSusp *desc = new DPMarshalerLitSusp(litTerm, nameSize);
#if defined(DBG_TRACE)
    {
      DBGINIT();
      char buf[10];
      buf[0] = (char) 0;
      strncat(buf, desc->getRemainingString(),
	      min(10, desc->getCurrentSize()));
      fprintf(dbgout, "> tag: %s(%d) = %.10s %s at %d\n",
	      dif_names[litTag].name, litTag, buf,
	      (desc->getCurrentSize() > 10 ? ".." : ""), litTermInd);
      fflush(dbgout);
    }
#endif
    dpMarshalString(bs, this, desc);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(litTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_BIGINT].name, DIF_BIGINT, toC(biTerm));
    fflush(dbgout);
#endif
    marshalBigInt(bs, biTerm, biConst);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(biTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(biTerm);
  }
}

//
Bool DPMarshaler::processNoGood(OZ_Term resTerm, Bool trail)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= DIFMaxSize + MDistSPPMaxSize) {
#if defined(DBG_TRACE)
    {
      DBGINIT();
      MarshalTag mt = trail ? DIF_RESOURCE_T : DIF_RESOURCE_N;
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[mt].name, mt, toC(resTerm));
      fflush(dbgout);
    }
#endif
    marshalSPP(bs, resTerm, trail); 
    return (OK);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(resTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(resTerm);
    return (NO);
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
      if (processNoGood(biTerm, OK))
	rememberNode(this, bs, biTerm);
      return;
    }
  } else {
    if (bs->availableSpace() >= 
	2*DIFMaxSize + MNumberMaxSize + strlen(pn)) {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_BUILTIN].name, DIF_BUILTIN, toC(biTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_BUILTIN);
      rememberNode(this, bs, biTerm);
      marshalString(bs, pn);
      return;
    }
  }

  //
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) on %s\n",
	  dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(biTerm));
  fflush(dbgout);
#endif
  marshalDIFcounted(bs, DIF_SUSPEND);
  suspend(biTerm);
}

//
void DPMarshaler::processExtension(OZ_Term t)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  OZ_Extension *oe = tagged2Extension(t);

  //
  if (oe->toBeMarshaledV()) {
    if (bs->availableSpace() >= 
	2*DIFMaxSize + 2*MNumberMaxSize + oe->minNeededSpace()) {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_EXTENSION].name, DIF_EXTENSION, toC(t));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_EXTENSION);
      rememberNode(this, bs, t);
      marshalNumber(bs, oe->getIdV());
      //
      OZ_Boolean ret = oe->marshalSuspV(t, bs, this);
      Assert(ret == OK);
      return;
    }
  } else {
    (void) processNoGood(t, NO); // not remembered!
    return;
  }

  //
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) on %s\n",
	  dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(t));
  fflush(dbgout);
#endif
  marshalDIFcounted(bs, DIF_SUSPEND);
  suspend(t);
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
      if (processNoGood(term, OK))
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
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_STUB_OBJECT].name, DIF_STUB_OBJECT, toC(term));
      fflush(dbgout);
#endif
      return (TRUE);
    }
  }

  //
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) on %s\n",
	  dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(term));
  fflush(dbgout);
#endif
  marshalDIFcounted(bs, DIF_SUSPEND);
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
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_OBJECT].name, DIF_OBJECT, toC(term));
      fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_OBJECT);
    rememberNode(this, bs, term);
    marshalGName(bs, o->getGName1());
    doToplevel = FALSE;
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(term));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(term);
    // 'doToplevel' is NOT reset here, since 'processObject' will be
    // re-applied when the marshaler is woken up!
  }
  return (FALSE);
}


//
#define HandleTert(string,tert,term,tag,check)		\
    ByteBuffer *bs = (ByteBuffer *) getOpaque();	\
    if (bs->availableSpace() >= DIFMaxSize +		\
	       MTertiaryMaxSize + MNumberMaxSize) {	\
      marshalTertiary(bs, tert, tag);			\
      rememberNode(this, bs, term);			\
    } else {						\
      DBG_TRACE_CODE(DBGINIT(););			\
      DBG_TRACE_CODE(fprintf(dbgout, "> tag: %s(%d) on %s\n", dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(term)););	\
      DBG_TRACE_CODE(fflush(dbgout););			\
      marshalDIFcounted(bs, DIF_SUSPEND);		\
      suspend(term);					\
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
void DPMarshaler::processVar(OZ_Term cv, OZ_Term *varTerm)
{
  Assert(varTerm);
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  // Note: futures are not triggered here! Instead, they are when a
  // snapshot of a term is taken;
  if (bs->availableSpace() >= DIFMaxSize + MNumberMaxSize +
	     max(MDistVarMaxSize, MDistSPPMaxSize)) {
    if (marshalVariable(varTerm, bs)) {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> var = %s\n", toC(cv));
      fflush(dbgout);
#endif
      rememberVarNode(this, bs, varTerm);
    } else {
      // kost@ : this does not work currently: when a variable is
      // bound, its 'ref' owner entry will never be found.
      OZ_warning("marshaling a variable as a resource!");
      if (processNoGood(makeTaggedRef(varTerm), OK))
	rememberVarNode(this, bs, varTerm);
      return;
    }
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND,
	    toC(makeTaggedRef(varTerm)));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(makeTaggedRef(varTerm));
  }
}

//
void DPMarshaler::processRepetition(OZ_Term t, OZ_Term *tPtr, int repNumber)
{
  Assert(repNumber >= 0);
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) for %d, value =  %s\n",
	    dif_names[DIF_REF].name, DIF_REF, repNumber, toC(t));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_REF);
    marshalTermRef(bs, repNumber);
  } else {
    Assert(t);			// we should not get here without 't'!
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND,
	    toC(oz_isVar(t) ? makeTaggedRef(tPtr) : t));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    Assert(!oz_isRef(t));
    if (oz_isVarOrRef(t))
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
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_LIST].name, DIF_LIST, toC(ltupleTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_LIST);
    rememberNode(this, bs, ltupleTerm);
    return (NO);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(ltupleTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_TUPLE].name, DIF_TUPLE, toC(srecordTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_TUPLE);
      rememberNode(this, bs, srecordTerm);
      marshalNumber(bs, rec->getTupleWidth());
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_RECORD].name, DIF_RECORD, toC(srecordTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_RECORD);
      rememberNode(this, bs, srecordTerm);
    }

    return (NO);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(srecordTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_CHUNK].name, DIF_CHUNK, toC(chunkTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_CHUNK);
    rememberNode(this, bs, chunkTerm);
    marshalGName(bs, gname);

    return (NO);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(chunkTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_FSETVALUE].name, DIF_FSETVALUE, toC(fsetvalueTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_FSETVALUE);
    return (NO);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(fsetvalueTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
      if (processNoGood(dictTerm, OK))
	rememberNode(this, bs, dictTerm);
      return (OK);
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_DICT].name, DIF_DICT, toC(dictTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_DICT);
      rememberNode(this, bs, dictTerm);
      marshalNumber(bs, d->getSize());
      return (NO);
    }
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(dictTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(dictTerm);
    return (OK);
  }    
}

//
Bool DPMarshaler::processArray(OZ_Term arrayTerm,
			       ConstTerm *arrayConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  if (bs->availableSpace() >= 
      DIFMaxSize + MDistSPPMaxSize + MNumberMaxSize) {
    if (processNoGood(arrayTerm, OK))
      rememberNode(this, bs, arrayTerm);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(arrayTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(arrayTerm);
  }
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
      if (processNoGood(classTerm, OK))
	rememberNode(this, bs, classTerm);
      return (OK);		// done - a leaf;
    }
  } else {
    if (bs->availableSpace() >= 
	2*DIFMaxSize + 2*MNumberMaxSize + MGNameMaxSize) {

      //
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_CLASS].name, DIF_CLASS, toC(classTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_CLASS);
      GName *gn = globalizeConst(cl, bs);

      //
      Assert(gn);
      rememberNode(this, bs, classTerm);
      marshalGName(bs, gn);
      marshalNumber(bs, cl->getFlags());
      return (NO);
    }
  }

  //
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) on %s\n",
	  dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(classTerm));
  fflush(dbgout);
#endif
  marshalDIFcounted(bs, DIF_SUSPEND);
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
      if (processNoGood(absTerm, OK))
	rememberNode(this, bs, absTerm);
      return (OK);		// done - a leaf;
    }
  } else {
    if (bs->availableSpace() >= 
	2*DIFMaxSize + 7*MNumberMaxSize + MGNameMaxSize) {
      //
      GName* gname = globalizeConst(pp, bs);
      Assert(gname);

      //
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_PROC].name, DIF_PROC, toC(absTerm));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_PROC);
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
      marshalNumber(bs, nxt);	// codesize in ByteCode"s;

      //
      DPMarshalerCodeAreaDescriptor *desc = 
	new DPMarshalerCodeAreaDescriptor(start, start + nxt);
      traverseBinary(dpMarshalCode, desc);

      //
      return (NO);
    } 
  }

  //
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) on %s\n",
	  dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(absTerm));
  fflush(dbgout);
#endif
  marshalDIFcounted(bs, DIF_SUSPEND);
  suspend(absTerm);
  return (OK);
}

//
// Marshaling of "hash table references" (for "match" instructions)
// into a limited space buffer is special: it can be partial, in which
// case marshaling of the code area suspends *at the instruction* that
// referenced that hash table (and not at the next one, as usual);
//
// It returns 'OK' when marshaling [of a hash table] was partial.
// In other words, 'OK' means that (a) marshaling of the code area has
// to suspend, and (b) it has to do so on the current instruction.
Bool dpMarshalHashTableRef(GenTraverser *gt,
			   DPMarshalerCodeAreaDescriptor *desc,
			   int start, IHashTable *table,
			   ByteBuffer *bs)
{
  // Make a worst-case estimation for the number of entries we can fit
  // into the remaining space in the in the buffer.
  const int nDone = desc->getHTNDone();
  // 'nDone' is non-zero when marshaling of a hash table is continued;
  const int tEntries = table->getEntries();  // Total Entries;
  Assert(tEntries > 0);
  const int rEntries = tEntries - nDone;     // Remaining Entries;
  const int wcFit =		// how many entries would fit;
    min((bs->availableSpace() - 4*MNumberMaxSize) / (4*MNumberMaxSize),
	rEntries);
  Assert(wcFit > 0);
  // Note: that field is present but not used by the pickler;
  marshalNumber(bs, wcFit);
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout,
	  ">  hash table=%p, nDone=%d, tEntries=%d, wcFit=%d\n",
	  table, nDone, tEntries, wcFit);
  fflush(dbgout);
#endif

  //
  int hti;
  if (nDone == 0) {
    marshalLabel(bs, start, table->lookupElse());
    marshalLabel(bs, start, table->lookupLTuple());
    marshalNumber(bs, tEntries);
    hti = table->getSize();
  } else {
    marshalLabel(bs, start, 0);
    marshalLabel(bs, start, 0);
    marshalNumber(bs, tEntries);
    hti = desc->getHTIndex();
  }    

  //
  int num = wcFit;
  // Tests in this order! 
  // If we suspend, we want to continue with the first unprocessed index;
  while (num > 0 && hti-- > 0) {
    if (table->entries[hti].val) {
      if (oz_isLiteral(table->entries[hti].val)) {
	if (table->entries[hti].sra == mkTupleWidth(0)) {
	  // That's a literal entry
	  marshalNumber(bs, ATOMTAG);
	  marshalLabel(bs, start, table->entries[hti].lbl);
	  gt->traverseOzValue(table->entries[hti].val);
	} else {
	  // That's a record entry
	  marshalNumber(bs, RECORDTAG);
	  marshalLabel(bs,start, table->entries[hti].lbl);
	  gt->traverseOzValue(table->entries[hti].val);
	  marshalRecordArity(gt, table->entries[hti].sra, bs);
	}
      } else {
	Assert(oz_isNumber(table->entries[hti].val));
	// That's a number entry
	marshalNumber(bs,NUMBERTAG);
	marshalLabel(bs, start, table->entries[hti].lbl);
	gt->traverseOzValue(table->entries[hti].val);
      }
      num--;
    }
  }

  //
  Assert(num == 0);
  // if we suspend, then there are some unprocessed indexes left:
  DebugCode(const Bool ret = (wcFit < rEntries) ? OK : NO;);
  Assert(ret == NO || hti > 0);
  // When we finish, the last index we checked is not necessarily 0;

  //
  if (wcFit < rEntries) {
    desc->setHTIndex(hti);
    desc->setHTNDone(nDone + wcFit);
    DebugCode(desc->setHTREntries(rEntries));
    DebugCode(desc->setHTable(table));
    return (OK);
  } else {
    DebugCode(desc->setHTIndex(-1));
    DebugCode(desc->setHTREntries(-1));
    DebugCode(desc->setHTable((IHashTable *) -1));
    desc->setHTNDone(0);
    return (NO);
  }
}

#ifdef USE_FAST_UNMARSHALER   

//
// 'suspend' turns 'OK' if unmarshaling is not complete;
ProgramCounter
dpUnmarshalHashTableRef(Builder *b,
			ProgramCounter pc, MarshalerBuffer *bs,
			DPBuilderCodeAreaDescriptor *desc, Bool &suspend)
{
  //
  if (pc) {
    const int num = unmarshalNumber(bs);
    const int elseLabel = unmarshalNumber(bs);
    const int listLabel = unmarshalNumber(bs);
    const int nEntries = unmarshalNumber(bs);
    const int nDone = desc->getHTNDone();
    IHashTable *table;

    //
    if (nDone == 0) {
      // new entry;
      table = IHashTable::allocate(nEntries, elseLabel);
      if (listLabel)
	table->addLTuple(listLabel);
    } else {
      // continuation;
      Assert(elseLabel == 0 && listLabel == 0);
      table = (IHashTable *) getAdressArg(pc);
    }

    //
    for (int i = num; i--; ) {    
      const int termTag = unmarshalNumber(bs);
      const int label   = unmarshalNumber(bs);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumber(bs);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: Assert(0); break;
      }
    }

    //
    if (num + nDone < nEntries) {
      suspend = OK;		// even if it already was;
      desc->setHTNDone(num + nDone);
    } else {
      Assert(num + nDone == nEntries);
      desc->setHTNDone(0);
    }

    // 
    // Either a new one, or a continuation: just write it off;
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    const int num = unmarshalNumber(bs);
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    const int nEntries = unmarshalNumber(bs);
    const int nDone = desc->getHTNDone();

    //
    for (int i = num; i--; ) {
      const int termTag = unmarshalNumber(bs);
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityType(bs);
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: Assert(0); break;
      }
    }

    //
    if (num + nDone < nEntries) {
      suspend = OK;		// even if it already was;
      desc->setHTNDone(num + nDone);
    } else {
      Assert(num + nDone == nEntries);
      desc->setHTNDone(0);
    }

    //
    return ((ProgramCounter) 0);
  }
}

#else

//
ProgramCounter
dpUnmarshalHashTableRefRobust(Builder *b,
			      ProgramCounter pc, MarshalerBuffer *bs,
			      DPBuilderCodeAreaDescriptor *desc,
			      Bool &suspend,
			      int *error)
{
  //
  if (pc) {
    const int num = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    const int elseLabel = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    const int listLabel = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    const int nEntries = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    const int nDone = desc->getHTNDone();
    IHashTable *table;

    //
    if (nDone == 0) {
      // new entry;
      table = IHashTable::allocate(nEntries, elseLabel);
      if (listLabel)
	table->addLTuple(listLabel);
    } else {
      // continuation;
      Assert(elseLabel == 0 && listLabel == 0);
      table = (IHashTable *) getAdressArg(pc);
    }

    //
    for (int i = num; i--; ) {    
      const int termTag = unmarshalNumberRobust(bs, error);
      if (*error) return ((ProgramCounter) 0);
      const int label = unmarshalNumberRobust(bs, error);
      if (*error) return ((ProgramCounter) 0);
      HashTableEntryDesc *desc = new HashTableEntryDesc(table, label);

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->getOzValue(getHashTableRecordEntryLabelCA, desc);
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
	  if(*error) return ((ProgramCounter) 0);
	  if (at == RECORDARITY) {
	    b->getOzValue(saveRecordArityHashTableEntryCA, desc);
	  } else {
	    Assert(at == TUPLEWIDTH);
	    int width = unmarshalNumberRobust(bs, error);
	    if(*error) return ((ProgramCounter) 0);
	    desc->setSRA(mkTupleWidth(width));
	  }
	  break;
	}

      case ATOMTAG:
	b->getOzValue(getHashTableAtomEntryLabelCA, desc);
	break;

      case NUMBERTAG:
	b->getOzValue(getHashTableNumEntryLabelCA, desc);
	break;

      default: *error = OK; break;
      }
    }

    //
    if (num + nDone < nEntries) {
      suspend = OK;
      desc->setHTNDone(num + nDone);
    } else {
      Assert(num + nDone == nEntries);
      desc->setHTNDone(0);
    }

    // 
    return (CodeArea::writeIHashTable(table, pc));
  } else {
    const int num = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    skipNumber(bs);		// elseLabel
    skipNumber(bs);		// listLabel
    const int nEntries = unmarshalNumberRobust(bs, error);
    if (*error) return ((ProgramCounter) 0);
    const int nDone = desc->getHTNDone();

    //
    for (int i = num; i--; ) {
      const int termTag = unmarshalNumberRobust(bs, error);
      if (*error) return ((ProgramCounter) 0);
      skipNumber(bs);		// label

      //
      switch (termTag) {
      case RECORDTAG:
	{
	  b->discardOzValue();
	  //
	  RecordArityType at = unmarshalRecordArityTypeRobust(bs, error);
	  if(*error) return ((ProgramCounter) 0);
	  if (at == RECORDARITY)
	    b->discardOzValue();
	  else
	    skipNumber(bs);
	  break;
	}

      case ATOMTAG:
	b->discardOzValue();
	break;

      case NUMBERTAG:
	b->discardOzValue();
	break;

      default: *error = OK; break;
      }
    }

    //
    if (num + nDone < nEntries) {
      suspend = OK;		// even if it already was;
      desc->setHTNDone(num + nDone);
    } else {
      Assert(num + nDone == nEntries);
      desc->setHTNDone(0);
    }

    //
    return ((ProgramCounter) 0);
  }
}

#endif

#include "dpMarshalcode.cc"

//
void DPMarshaler::processSync()
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  if (bs->availableSpace() >= 2*DIFMaxSize) {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d)\n", dif_names[DIF_SYNC].name, DIF_SYNC);
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SYNC);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d)\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND);
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
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
  if (doToplevel) {
    doToplevel = FALSE;
    return (FALSE);
  } else {
    return (TRUE);
  }
}
Bool VariableExcavator::processNoGood(OZ_Term resTerm, Bool trail)
{
  Assert(!oz_isVar(resTerm));
  return (OK);
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
void VariableExcavator::processVar(OZ_Term cv, OZ_Term *varTerm)
{
  Assert(oz_isVar(cv));
  rememberVarLocation(varTerm);
  addVar(makeTaggedRef(varTerm));
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
    (void) processNoGood(dictTerm, OK);
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
  (void) processNoGood(arrayTerm, OK);
  return (OK);
}

//
Bool VariableExcavator::processClass(OZ_Term classTerm,
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
Bool VariableExcavator::processAbstraction(OZ_Term absTerm,
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
    DPMarshalerCodeAreaDescriptor *desc = 
      new DPMarshalerCodeAreaDescriptor(start, start + nxt);
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
    DEREF(tc, tPtr);

    //
    *(--top) = (StackEntry) t;

    //
    if (oz_isMark(tc)) {
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
	  // marshaling continuations are dropped:
	  *(--top) = (StackEntry) contProcNOOP;
	}
	break;
      }
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
  dif_counter[tag].send();
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
  dif_counter[tag].send();
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

  marshalCredit(bs, b->getCreditBig());
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
  c = b->getCreditBig();
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

OZ_Term
#ifndef USE_FAST_UNMARSHALER
unmarshalBorrowRobust(MarshalerBuffer *bs,
		      OB_Entry *&ob, int &bi, int *error)
#else
unmarshalBorrow(MarshalerBuffer *bs, OB_Entry* &ob, int &bi)
#endif
{
  PD((UNMARSHAL,"Borrow"));
#ifndef USE_FAST_UNMARSHALER
  DSite *sd = unmarshalDSiteRobust(bs, error);
  if (*error)
    return ((OZ_Term) 0);	// carry on the error;
  int si = unmarshalNumberRobust(bs, error);
  if (*error)
    return ((OZ_Term) 0);
#else
  DSite* sd = unmarshalDSite(bs);
  int si = unmarshalNumber(bs);
#endif
  PD((UNMARSHAL,"borrow o:%d",si));
  if(sd==myDSite){
    Assert(0); 
//     if(mt==DIF_PRIMARY){
//       cred = unmarshalCredit(bs);      
//       PD((UNMARSHAL,"myDSite is owner"));
//       OwnerEntry* oe=ownerTable->getOwner(si);
//       if(cred != PERSISTENT_CRED)
// 	oe->returnCreditOwner(cred);
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
  if (*error)
    return ((OZ_Term) 0);
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
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "> tag: %s(%d) = %s\n",
	  dif_names[tag].name, tag, toC(makeTaggedConst(t)));
  fflush(dbgout);
#endif
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

OZ_Term
#ifndef USE_FAST_UNMARSHALER
unmarshalTertiaryRobust(MarshalerBuffer *bs, MarshalTag tag, int *error)
#else
unmarshalTertiary(MarshalerBuffer *bs, MarshalTag tag)
#endif
{
  OB_Entry* ob;
  int bi;
#ifndef USE_FAST_UNMARSHALER
  OZ_Term val = unmarshalBorrowRobust(bs, ob, bi, error);
  if (*error)
    return ((OZ_Term) 0);
#else
  OZ_Term val = unmarshalBorrow(bs, ob, bi);
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
      (void) unmarshalGNameRobust(&obj, bs, error);
      if (*error)
	return ((OZ_Term) 0);
      (void) unmarshalGNameRobust(&clas, bs, error);
      if (*error)
	return ((OZ_Term) 0);
#else
      (void) unmarshalGName(&obj, bs);
      (void) unmarshalGName(&clas, bs);
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
      if (*error)
	return ((OZ_Term) 0);
      GName *gnclass = unmarshalGNameRobust(&clas, bs, error);
      if (*error)
	return ((OZ_Term) 0);
#else
      GName *gnobj = unmarshalGName(&obj, bs);
      GName *gnclass = unmarshalGName(&clas, bs);
#endif
      if(!gnobj) {	
//  	printf("Had Object %d:%d flags:%d\n",
//  	       ((BorrowEntry *)ob)->getNetAddress()->index,
//  	       ((BorrowEntry *)ob)->getNetAddress()->site->getTimeStamp()->pid,
//  	       ((BorrowEntry *)ob)->getFlags());
	if(!(borrowTable->maybeFreeBorrowEntry(bi))){
	  ob->mkRef(obj,ob->getFlags());
//    	  printf("indx:%d %xd\n",((BorrowEntry *)ob)->getNetAddress()->index,
//    	  	 ((BorrowEntry *)ob)->getNetAddress()->site);
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


OZ_Term 
#ifndef USE_FAST_UNMARSHALER
unmarshalOwnerRobust(MarshalerBuffer *bs, MarshalTag mt, int *error)
#else
unmarshalOwner(MarshalerBuffer *bs, MarshalTag mt)
#endif
{
  int OTI;
#ifndef USE_FAST_UNMARSHALER
  Credit c = unmarshalCreditToOwnerRobust(bs, mt, OTI, error);
  if (*error)
    return ((OZ_Term) 0);
#else
  Credit c = unmarshalCreditToOwner(bs, mt, OTI);
#endif
  PD((UNMARSHAL,"OWNER o:%d",OTI));
  OwnerEntry* oe=ownerTable->getOwner(OTI);
  oe->addCredit(c);
  OZ_Term oz=oe->getValue();
  return oz;
}

//
void DPBuilderCodeAreaDescriptor::gc()
{
  Assert(current >= start && current <= end);
  // Unfortunately, 'ENDOFFILE' has to be recorded eagerly, because
  // 'CodeArea::gCollectCodeAreaStart()' (in its current incarnation)
  // has to be called before any codearea can be reached by other
  // means;
#if defined(DEBUG_CHECK)
  if (getCurrent()) {
    Opcode op = CodeArea::getOpcode(getCurrent());
    Assert(op == ENDOFFILE);
  }
#endif
}

//
//
OZ_Term dpUnmarshalTerm(ByteBuffer *bs, Builder *b)
{
  Assert(oz_onToplevel());
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "<bs: %p (getptr=%p,posMB=%p,endMB=%p)\n",
	  bs, bs->getGetptr(), bs->getPosMB(), bs->getEndMB());
  fflush(dbgout);
#endif

#ifndef USE_FAST_UNMARSHALER
  TRY_UNMARSHAL_ERROR {
#endif
    while(1) {
      MarshalTag tag = (MarshalTag) bs->get();
      dif_counter[tag].recv();	// kost@ : TODO: needed?
#if defined(DBG_TRACE)
      fprintf(dbgout, "< tag: %s(%d)", dif_names[tag].name, tag);
      fflush(dbgout);
#endif

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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %d\n", ozInt);
	  fflush(dbgout);
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %f\n", f);
	  fflush(dbgout);
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
	  int nameSize  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  OZ_Term value;
	  GName *gname    = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " >>> (at %d)\n", refTag);
	    fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
	    break;
	  }
	}

      case DIF_COPYABLENAME:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag      = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int nameSize  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " >>> (at %d)\n", refTag);
	    fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
	  }
	  break;
	}

      case DIF_UNIQUENAME:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag      = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int nameSize  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  char *printname = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error || (printname == NULL));
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " >>> (at %d)\n", refTag);
	    fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
	  }
	  break;
	}

      case DIF_ATOM:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  int nameSize  = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
	  char *aux  = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " >>> (at %d)\n", refTag);
	    fflush(dbgout);
#endif
	    // returns '-1' for an additional consistency check - the
	    // caller should know whether that's a complete message;
	    return ((OZ_Term) -1);
	  } else {
	    // complete print name;
	    Assert(strLen == nameSize);

	    OZ_Term value = OZ_atom(aux);
	    b->buildValue(value);
	    b->set(value, refTag);
	    delete [] aux;
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
	  }
	  break;
	}

	//
      case DIF_LIT_CONT:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  char *aux  = unmarshalStringRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " >>> (at %d)\n", ns->getRefTag());
		fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " = %s (at %d)\n",
			toC(value), ns->getRefTag());
		fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " >>> (at %d)\n", as->getRefTag());
		fflush(dbgout);
#endif
		// returns '-1' for an additional consistency check - the
		// caller should know whether that's a complete message;
		return ((OZ_Term) -1);
	      } else {
		Assert(as->getNameSize() == as->getPNSize());

		OZ_Term value = OZ_atom(as->getPrintname());
		b->buildValue(value);
		b->set(value, as->getRefTag());
		delete as;
#if defined(DBG_TRACE)
		fprintf(dbgout, " = %s (at %d)\n",
			toC(value), as->getRefTag());
		fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " >>> (at %d)\n", uns->getRefTag());
		fflush(dbgout);
#endif
		// returns '-1' for an additional consistency check - the
		// caller should know whether that's a complete message;
		return ((OZ_Term) -1);
	      } else {
		Assert(uns->getNameSize() == uns->getPNSize());

		OZ_Term value = oz_uniqueName(uns->getPrintname());
		b->buildValue(value);
		b->set(value, uns->getRefTag());
		delete uns;
#if defined(DBG_TRACE)
		fprintf(dbgout, " = %s (at %d)\n",
			toC(value), uns->getRefTag());
		fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " >>> (at %d)\n", cns->getRefTag());
		fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
		fprintf(dbgout, " = %s (at %d)\n",
			toC(value), cns->getRefTag());
		fflush(dbgout);
#endif
	      }
	      break;
	    }

	  default:
#ifndef USE_FAST_UNMARSHALER
	    Assert(0);
	    (void) b->finish();
	    return ((OZ_Term) 0);
#else
	    OZ_error("Illegal GTAbstractEntity (builder) for a LitCont!");
	    b->buildValue(oz_nil());
#endif
	  }
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " %s\n", aux);
	  fflush(dbgout);
#endif
	  b->buildValue(OZ_CStringToNumber(aux));
	  delete aux;
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (at %d)\n", refTag);
	  fflush(dbgout);
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (at %d)\n", refTag);
	  fflush(dbgout);
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (at %d)\n", refTag);
	  fflush(dbgout);
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (from %d)\n", i);
	  fflush(dbgout);
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
	  int error;
	  OZ_Term tert = unmarshalOwnerRobust(bs, tag, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term tert = unmarshalOwner(bs, tag);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(tert), refTag);
	  fflush(dbgout);
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
	  int error;
	  OZ_Term tert = unmarshalTertiaryRobust(bs, tag, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term tert = unmarshalTertiary(bs, tag);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(tert), refTag);
	  fflush(dbgout);
#endif
	  b->buildValueRemember(tert, refTag);
	  break;
	}

      case DIF_RESOURCE_N:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term tert = unmarshalTertiaryRobust(bs, tag, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term tert = unmarshalTertiary(bs, tag);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(tert));
	  fflush(dbgout);
#endif
	  b->buildValue(tert);
	  break;
	}

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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " (at %d)\n", refTag);
	    fflush(dbgout);
#endif
	    b->buildChunkRemember(gname, refTag);
	  } else {
	    b->knownChunk(value);
	    b->set(value, refTag);
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
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
	  RETURN_ON_ERROR(error || (flags > CLASS_FLAGS_MAX));
#else
	  int refTag = unmarshalRefTag(bs);
	  GName *gname = unmarshalGName(&value, bs);
	  int flags = unmarshalNumber(bs);
#endif
	  //
	  if (gname) {
#if defined(DBG_TRACE)
	    fprintf(dbgout, " (at %d)\n", refTag);
	    fflush(dbgout);
#endif
	    b->buildClassRemember(gname, flags, refTag);
	  } else {
	    // Either we have the class itself, or that's the lazy
	    // object protocol, in which case it must be a variable
	    // that denotes an object (of this class);
	    OZ_Term vd = oz_deref(value);
	    Assert(!oz_isRef(vd));
	    if (oz_isConst(vd)) {
	      Assert(tagged2Const(vd)->getType() == Co_Class);
	      b->knownClass(value);
	      b->set(value, refTag);
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	      fflush(dbgout);
#endif
	    } else if (oz_isVar(vd)) {
	      OzVariable *var = tagged2Var(vd);
	      Assert(var->getType() == OZ_VAR_EXT);
	      ExtVar *evar = var2ExtVar(var);
	      Assert(evar->getIdV() == OZ_EVAR_LAZY);
	      LazyVar *lvar = (LazyVar *) evar;
	      Assert(lvar->getLazyType() == LT_CLASS);
	      ClassVar *cv = (ClassVar *) lvar;
	      // The binding of a class'es gname is kept until the
	      // construction of a class is finished.
	      gname = cv->getGName();
	      b->buildClassRemember(gname, flags, refTag);
#if defined(DBG_TRACE)
	      fprintf(dbgout, " (at %d)\n", refTag);
	      fflush(dbgout);
#endif
	    } else {
#ifndef USE_FAST_UNMARSHALER
	      Assert(0);
	      (void) b->finish();
	      return ((OZ_Term) 0);
#else
	      OZ_error("Unexpected value is bound to an object's gname");
	      b->buildValue(oz_nil());
#endif
	    }
	  }
	  break;
	}

      case DIF_VAR:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term v = unmarshalVarRobust(bs, FALSE, FALSE, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term v = unmarshalVar(bs, FALSE, FALSE);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(v), refTag);
	  fflush(dbgout);
#endif
	  b->buildValueRemember(v, refTag);
	  break;
	}

      case DIF_FUTURE: 
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term f = unmarshalVarRobust(bs, TRUE, FALSE, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term f = unmarshalVar(bs, TRUE, FALSE);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(f), refTag);
	  fflush(dbgout);
#endif
	  b->buildValueRemember(f, refTag);
	  break;
	}

      case DIF_VAR_AUTO: 
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term va = unmarshalVarRobust(bs, FALSE, TRUE, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term va = unmarshalVar(bs, FALSE, TRUE);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(va), refTag);
	  fflush(dbgout);
#endif
	  b->buildValueRemember(va, refTag);
	  break;
	}

      case DIF_FUTURE_AUTO: 
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  OZ_Term fa = unmarshalVarRobust(bs, TRUE, TRUE, &error);
	  RETURN_ON_ERROR(error);
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
#else
	  OZ_Term fa = unmarshalVar(bs, TRUE, TRUE);
	  int refTag = unmarshalRefTag(bs);
#endif
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(fa), refTag);
	  fflush(dbgout);
#endif
	  b->buildValueRemember(fa, refTag);
	  break;
	}

      case DIF_OBJECT:
	{
	  OZ_Term value;
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int refTag = unmarshalRefTagRobust(bs, b, &error);
	  RETURN_ON_ERROR(error);
	  GName *gname = unmarshalGNameRobust(&value, bs, &error);
	  RETURN_ON_ERROR(error);
#else
	  int refTag = unmarshalRefTag(bs);
	  GName *gname = unmarshalGName(&value, bs);
#endif
	  if (gname) {
	    // If objects are distributed only using the lazy
	    // protocol, this shouldn't happen: There must be a proxy
	    // already;
#if defined(DBG_TRACE)
	    fprintf(dbgout, " (at %d)\n", refTag);
	    fflush(dbgout);
#endif
	    b->buildObjectRemember(gname, refTag);
	  } else {
	    // The same as for classes: either we have already the
	    // object (how comes? requested twice??), or that's the
	    // lazy object protocol;
	    OZ_Term vd = oz_deref(value);
	    Assert(!oz_isRef(vd));
	    if (oz_isConst(vd)) {
	      Assert(tagged2Const(vd)->getType() == Co_Object);
	      OZ_warning("Full object is received again.");
	      b->knownObject(value);
	      b->set(value, refTag);
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	      fflush(dbgout);
#endif
	    } else if (oz_isVar(vd)) {
	      OzVariable *var = tagged2Var(vd);
	      Assert(var->getType() == OZ_VAR_EXT);
	      ExtVar *evar = var2ExtVar(var);
	      Assert(evar->getIdV() == OZ_EVAR_LAZY);
	      LazyVar *lvar = (LazyVar *) evar;
	      Assert(lvar->getLazyType() == LT_OBJECT);
	      ObjectVar *ov = (ObjectVar *) lvar;
	      gname = ov->getGName();
	      // Observe: the gname points to the proxy;
	      b->buildObjectRemember(gname, refTag);
#if defined(DBG_TRACE)
	      fprintf(dbgout, " (at %d)\n", refTag);
	      fflush(dbgout);
#endif
	    } else {
#ifndef USE_FAST_UNMARSHALER
	      Assert(0);
	      (void) b->finish();
	      return ((OZ_Term) 0);
#else
	      OZ_error("Unexpected value is bound to an object's gname.");
	      b->buildValue(oz_nil());
#endif
	    }
	  }
	  break;
	}

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
	  int codesize  = unmarshalNumberRobust(bs, &error); // in ByteCode"s;
	  RETURN_ON_ERROR(error || maxX < 0 || maxX >= NumberOfXRegisters);
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
	    Assert(codesize > 0);
	    DPBuilderCodeAreaDescriptor *desc =
	      new DPBuilderCodeAreaDescriptor(start, start+codesize, code);
	    b->buildBinary(desc);

	    //
	    b->buildProcRemember(gname, arity, gsize, maxX, line, column, 
				 pc, refTag);
#if defined(DBG_TRACE)
	    fprintf(dbgout,
		    " do arity=%d,line=%d,column=%d,codesize=%d (at %d)\n",
		    arity, line, column, codesize, refTag);
	    fflush(dbgout);
#endif
	  } else {
	    Assert(oz_isAbstraction(oz_deref(value)));
	    // ('zero' descriptions are not allowed;)
	    DPBuilderCodeAreaDescriptor *desc =
	      new DPBuilderCodeAreaDescriptor(0, 0, 0);
	    b->buildBinary(desc);

	    //
	    b->knownProcRemember(value, refTag);
#if defined(DBG_TRACE)
	    fprintf(dbgout,
		    " skip %s (arity=%d,line=%d,column=%d,codesize=%d) (at %d)\n",
		    toC(value), arity, line, column, codesize, refTag);
	    fflush(dbgout);
#endif
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
	  DPBuilderCodeAreaDescriptor *desc = 
	    (DPBuilderCodeAreaDescriptor *) b->fillBinary(opaque);
	  //
#if defined(DBG_TRACE)
	  fprintf(dbgout,
		  " [begin=%p, end=%p, current=%p]",
		  desc->getStart(), desc->getEnd(), desc->getCurrent());
	  fflush(dbgout);
#endif
#ifndef USE_FAST_UNMARSHALER
	  switch (dpUnmarshalCodeRobust(bs, b, desc)) {
	  case UCR_ERROR:
	    Assert(0);
	    (void) b->finish();
	    return 0;
	  case UCR_DONE:
#if defined(DBG_TRACE)
	    fprintf(dbgout, " ..finished\n");
	    fflush(dbgout);
#endif
	    b->finishFillBinary(opaque);
	    break;
	  case UCR_SUSPEND:
#if defined(DBG_TRACE)
	    fprintf(dbgout, " ..suspended\n");
	    fflush(dbgout);
#endif
	    b->suspendFillBinary(opaque);
	    break;
	  }
#else
	  if (dpUnmarshalCode(bs, b, desc)) {
#if defined(DBG_TRACE)
	    fprintf(dbgout, " ..finished\n");
	    fflush(dbgout);
#endif
	    b->finishFillBinary(opaque);
	  } else {
#if defined(DBG_TRACE)
	    fprintf(dbgout, " ..suspended\n");
	    fflush(dbgout);
#endif
	    b->suspendFillBinary(opaque);
	  }
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " size=%d (at %d)\n", size, refTag);
	  fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
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

	  //
	  GTAbstractEntity *bae;
	  OZ_Term value = oz_extension_unmarshal(type, bs, bae);
	  switch (value) {
	  case (OZ_Term) -1:	// suspension;
	    Assert(bae);
	    b->getAbstractEntity(bae);
	    b->suspend();
#if defined(DBG_TRACE)
	    fprintf(dbgout, " suspended (for %d)\n", refTag);
	    fflush(dbgout);
#endif
	    return ((OZ_Term) -1);

	  case (OZ_Term) 0:	// error;
#ifndef USE_FAST_UNMARSHALER
	    Assert(0);
	    (void) b->finish();
	    return ((OZ_Term) 0);
#else
	    OZ_error("Trouble with unmarshaling an extension!");
	    b->buildValue(oz_nil());
	    break;
#endif

	  default:		// got it!
	    b->buildValue(value);
	    b->set(value, refTag);
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	    fflush(dbgout);
#endif
	    break;
	  }
	  break;
	}

	// Continuation for "extensions";
      case DIF_EXT_CONT:
	{
#ifndef USE_FAST_UNMARSHALER
	  int error;
	  int type = unmarshalNumberRobust(bs, &error);
	  RETURN_ON_ERROR(error);
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
#if defined(DBG_TRACE)
	    fprintf(dbgout, " suspended\n");
	    fflush(dbgout);
#endif
	    return ((OZ_Term) -1);

	  case (OZ_Term) 0:
#ifndef USE_FAST_UNMARSHALER
	    Assert(0);
	    (void) b->finish();
	    return ((OZ_Term) 0);
#else
	    OZ_error("Trouble with unmarshaling an extension!");
	    b->buildValue(oz_nil());
	    break;
#endif

	  default:
#if defined(DBG_TRACE)
	    fprintf(dbgout, " = %s\n", toC(value));
	    fflush(dbgout);
#endif
	    b->buildValue(value);
	    break;
	  }
	  break;
	}

      case DIF_FSETVALUE:
#if defined(DBG_TRACE)
	fprintf(dbgout, "\n");
	fflush(dbgout);
#endif
	b->buildFSETValue();
	break;

      case DIF_REF_DEBUG:
#ifndef USE_FAST_UNMARSHALER   
	Assert(0);
	(void) b->finish();
	return ((OZ_Term) 0);
#else
	OZ_error("not implemented!"); 
	b->buildValue(oz_nil());
	break;
#endif

      case DIF_ARRAY:
#ifndef USE_FAST_UNMARSHALER   
	Assert(0);
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
#if defined(DBG_TRACE)
	fprintf(dbgout, "\n");
	fflush(dbgout);
#endif
	b->processSync();
	break;

      case DIF_EOF: 
#if defined(DBG_TRACE)
	fprintf(dbgout, "\n");
	fflush(dbgout);
#endif
	return (b->finish());

      case DIF_SUSPEND:
#if defined(DBG_TRACE)
	fprintf(dbgout, "\n");
	fflush(dbgout);
#endif
	// 
	b->suspend();
	// returns '-1' for an additional consistency check - the
	// caller should know whether that's a complete message;
	return ((OZ_Term) -1);

      default:
#ifndef USE_FAST_UNMARSHALER   
	Assert(0);
	(void) b->finish();
	return ((OZ_Term) 0);
#else
	OZ_error("unmarshal: unexpected tag: %d\n",tag);
	b->buildValue(oz_nil());
	break;
#endif
      }
    }
#ifndef USE_FAST_UNMARSHALER
  }
  CATCH_UNMARSHAL_ERROR { 
    Assert(0);
    (void) b->finish();
    return ((OZ_Term) 0);
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
    delete [] mus;

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
      dpm->reset();
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
    Assert(oz_isVar(*vp));
    OZ_Term v = *vp;

    //
    if (oz_isExtVar(v)) {
      ExtVarType evt = oz_getExtVar(v)->getIdV();
      switch (evt) {
      case OZ_EVAR_MANAGER:
	{
	  // make&save an "exported" manager;
	  ManagerVar *mvp = oz_getManagerVar(v);
	  ExportedManagerVar *emvp = new ExportedManagerVar(mvp, dest);
	  OZ_Term emv = makeTaggedVar(extVar2Var(emvp));
	  //
	  svl = new SntVarLocation(makeTaggedRef(vp), emv, svl);

	  // There are no distributed futures: once exported, it is
	  // kicked (just now). Note also that futures are first
	  // exported, then kicked (since kicking can immediately
	  // yield a new subtree we cannot handle).
	  Assert(oz_isVar(*vp));
  	  (void) triggerVariable(vp);
	}
	break;

      case OZ_EVAR_PROXY:
	{
	  // make&save an "exported" proxy:
	  ProxyVar *pvp = oz_getProxyVar(v);
	  ExportedProxyVar *epvp = new ExportedProxyVar(pvp, dest);
	  OZ_Term epv = makeTaggedVar(extVar2Var(epvp));
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

      //
      ManagerVar *mvp = globalizeFreeVariable(vp);
      ExportedManagerVar *emvp = new ExportedManagerVar(mvp, dest);
      OZ_Term emv = makeTaggedVar(extVar2Var(emvp));
      //
      svl = new SntVarLocation(makeTaggedRef(vp), emv, svl);

      //
      Assert(oz_isVar(*vp));
      (void) triggerVariable(vp);
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
    Assert(oz_isExtVar(v));	// must be distributed by now;
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
    // Keep locations of former variables. Note that a single
    // 'GCStubVar' can be shared between different snapshots during
    // GC.
    if (oz_isRef(hv) || !oz_isVarOrRef(hv)) {
      GCStubVar *svp = new GCStubVar(hv);
      *hvp = makeTaggedVar(extVar2Var(svp));
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
      _DEREF(hv, hvp);
      Assert(oz_isVar(hv));
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

//
#if defined(DEBUG_CHECK)
void MsgTermSnapshotImpl::checkVar(OZ_Term t)
{
  DEREF(t, tPtr);
  Assert(!oz_isRef(t));
  if (oz_isVarOrRef(t)) {
    if (oz_isExtVar(t)) {
      ExtVarType evt = oz_getExtVar(t)->getIdV();
      switch (evt) {
      case OZ_EVAR_MANAGER:
      case OZ_EVAR_PROXY:
	OZ_error("An unexported manager/proxy var is found!");
	break;

      case OZ_EVAR_LAZY:
	break;

      case OZ_EVAR_EMANAGER:
      case OZ_EVAR_EPROXY:
	{
	  SntVarLocation *l = locs;
	  Bool found = NO;
	  while(l) {
	    OZ_Term vr = l->getLoc();
	    OZ_Term *vp = tagged2Ref(vr);
	    if (tPtr == vp) {
	      Assert(t == l->getVar());
	      found = OK;
	      break;
	    }
	    l = l->getNextLoc();
	  }
	  if (!found)
	    OZ_error("A foreign exported manager/proxy is found!");
	}
	break;

      case OZ_EVAR_GCSTUB:
	OZ_error("A gcstub var is found!");
	break;

      default:
	Assert(0);
	break;
      }
    } else if (oz_isFree(t) || oz_isFuture(t)) {
      OZ_error("An unexported free variable/future is found!");
    }
  }
}
#endif
