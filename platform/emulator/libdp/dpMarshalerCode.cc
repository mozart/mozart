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

//
// This file contains the marshaling code for DPMarshaler.
//
// 'DPMARSHALERCLASS' must be defined for a particular class name;
//

inline 
void DPMARSHALERCLASS::processSmallInt(OZ_Term siTerm)
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
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
void DPMARSHALERCLASS::processFloat(OZ_Term floatTerm)
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
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
void DPMARSHALERCLASS::processBigInt(OZ_Term biTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  const char *crep = toC(biTerm);

  //
  // Hmm.. limitation: big integers cannot be arbitrary big, but
  // not larger than the buffer size would allow;
  if (bs->availableSpace() >= 
      2*DIFMaxSize + 2*MNumberMaxSize + strlen(crep)) {
    int index;
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_BIGINT].name, DIF_BIGINT, toC(biTerm));
    fflush(dbgout);
#endif

    //
    VISITNODE(biTerm, vIT, bs, index, return);

    //
    if (index) {
      marshalDIFcounted(bs, DIF_BIGINT_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIFcounted(bs, DIF_BIGINT);
    }
    marshalString(bs, crep);
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
void DPMARSHALERCLASS::processLiteral(OZ_Term litTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  // The name may not fit completely;
  if (bs->availableSpace() >= 
      2*DIFMaxSize + 3*MNumberMaxSize + MGNameMaxSize) {
    int index;

    // Note: cycles/co-references check is done only when we'are about
    // to marshal it. Otherwise, we could loose a _DEF;
    VISITNODE(litTerm, vIT, bs, index, return);

    //
    Literal *lit = tagged2Literal(litTerm);
    if (lit->isAtom()) {
      if (index) {
	marshalDIFcounted(bs, DIF_ATOM_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_ATOM);
      }
    } else if (lit->isUniqueName()) {
      if (index) {
	marshalDIFcounted(bs, DIF_UNIQUENAME_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_UNIQUENAME);
      }
    } else if (lit->isCopyableName()) {
      if (index) {
	marshalDIFcounted(bs, DIF_COPYABLENAME_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_COPYABLENAME);
      }
    } else {
      if (index) {
	marshalDIFcounted(bs, DIF_NAME_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_NAME);
      }
      marshalGName(bs, ((Name *) lit)->globalize());
    }

    //
    const char *name = lit->getPrintName();
    const int nameSize = strlen(name);
    marshalNumber(bs, nameSize);

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
      fprintf(dbgout, "> tag: %s(%d) = %.10s %s\n",
	      dif_names[litTag].name, litTag, buf,
	      (desc->getCurrentSize() > 10 ? ".." : ""));
      fflush(dbgout);
    }
#endif
    dpMarshalString(bs, this, desc);
    Assert(bs->availableSpace() >= DIFMaxSize);
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
// Decides whether to put '_DEF' on its own;
inline
void DPMARSHALERCLASS::processNoGood(OZ_Term resTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >=
      2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize) {
    int index;
#if defined(DBG_TRACE)
    {
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_RESOURCE].name, DIF_RESOURCE, toC(resTerm));
      fflush(dbgout);
    }
#endif

    //
    VISITNODE(resTerm, vIT, bs, index, return);
    //
    MarshalRHTentry(resTerm, index);

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(resTerm));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(resTerm);
  }
}

//
inline
void DPMARSHALERCLASS::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  Builtin *bi = (Builtin *) biConst;
  const char *pn = bi->getPrintName();

  //
  if (bs->availableSpace() >= 
      max(2*DIFMaxSize + 2*MNumberMaxSize + strlen(pn),
	  2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize)) {
    int index;

    //
    VISITNODE(biTerm, vIT, bs, index, return);

    //
    if (bi->isSited()) {
      MarshalRHTentry(biTerm, index);
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_BUILTIN].name, DIF_BUILTIN, toC(biTerm));
      fflush(dbgout);
#endif

      //
      if (index) {
	marshalDIFcounted(bs, DIF_BUILTIN_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_BUILTIN);
      }
      //
      marshalString(bs, pn);
    }
    // otherwise fall through and put 'DIF_SUSPEND';

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
void DPMARSHALERCLASS::processExtension(OZ_Term et)
{
  OZ_Extension *oe = tagged2Extension(et);

  //
  if (oe->toBeMarshaledV()) {
    ByteBuffer *bs = (ByteBuffer *) getOpaque();

    //
    if (bs->availableSpace() >= 
	2*DIFMaxSize + 2*MNumberMaxSize + oe->minNeededSpace()) {
      int index;

      //
      VISITNODE(et, vIT, bs, index, return);

#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_EXTENSION].name, DIF_EXTENSION, toC(t));
      fflush(dbgout);
#endif
      if (index) {
	marshalDIFcounted(bs, DIF_EXTENSION_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_EXTENSION);
      }
      marshalNumber(bs, oe->getIdV());
      //
      oe->marshalSuspV(et, bs, this);

      //
      Assert(bs->availableSpace() >= DIFMaxSize);
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) on %s\n",
	      dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(t));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_SUSPEND);
      suspend(et);
    }
  } else {
    // handle it as a "no good", including cycles etc.;
    processNoGood(et);
  }
}

// private methods;
inline 
Bool DPMARSHALERCLASS::marshalObjectStub(OZ_Term term, ConstTerm *objConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 
      2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize + 2*MGNameMaxSize) {
    int index;

    //
    VISITNODE(term, vIT, bs, index, return(OK));

    //
    Object *o = (Object*) objConst;
    Assert(isObject(o));
    //
    if (o->getClass()->isSited()) {
      MarshalRHTentry(term, index);
    } else {
      //
      Assert(o->getTertType() == Te_Local || o->getTertType() == Te_Manager);
      if (o->getTertType() == Te_Local)
	globalizeTert(o);

      //
      ObjectClass *oc = o->getClass();
      GName *gnclass = globalizeConst(oc);
      Assert(gnclass);
      GName *gnobj = globalizeConst(o);
      Assert(o->getGName1());
      Assert(gnobj);
      Assert(o->getTertType() == Te_Manager);
      // No "lazy class" protocol, so it isn't a tertiary:
      // Assert(oc->getTertType() == Te_Manager);
      //

      //
      marshalDIFcounted(bs, index ? DIF_STUB_OBJECT_DEF : DIF_STUB_OBJECT);
      marshalOwnHead(bs, MakeOB_TIndex(o->getTertPointer()));
      marshalGName(bs, gnobj);
      marshalGName(bs, gnclass);
      //
      if (index) marshalTermDef(bs, index);

      //
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_STUB_OBJECT].name, DIF_STUB_OBJECT, toC(term));
      fflush(dbgout);
#endif
    }

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(term));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(term);
  }
  return (TRUE);
}

//
inline 
Bool DPMARSHALERCLASS::marshalFullObject(OZ_Term term, ConstTerm *objConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 
      2*DIFMaxSize + MNumberMaxSize + MGNameMaxSize) {
    int index;
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_OBJECT].name, DIF_OBJECT, toC(term));
    fflush(dbgout);
#endif

    //
    VISITNODE(term, vIT, bs, index, return(OK));

    if (index) {
      marshalDIFcounted(bs, DIF_OBJECT_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIFcounted(bs, DIF_OBJECT);
    }

    //
    Object *o = (Object*) objConst;
    Assert(isObject(o));
    Assert(o->getTertType() == Te_Manager);
    // Assert(o->getClass()->getTertType() == Te_Manager);
    marshalGName(bs, o->getGName1());
    doToplevel = FALSE;

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processObject(OZ_Term term, ConstTerm *objConst)
{
  if (doToplevel)
    return (marshalFullObject(term, objConst));
  else
    return (marshalObjectStub(term, objConst));
}

//
#define DPMHandleTert(string, tert, term, tag, Return)			\
{									\
  ByteBuffer *bs = (ByteBuffer *) getOpaque();				\
  if (bs->availableSpace() >= 						\
      DIFMaxSize + MTertiaryMaxSize + MNumberMaxSize) {			\
    int index;								\
    VISITNODE(term, vIT, bs, index, Return);				\
    marshalTertiary(bs, tert, index, tag);				\
    if (index) marshalTermDef(bs, index);				\
    Assert(bs->availableSpace() >= DIFMaxSize);				\
  } else {								\
    DBG_TRACE_CODE(DBGINIT(););						\
    DBG_TRACE_CODE(fprintf(dbgout, "> tag: %s(%d) on %s\n",		\
      dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(term)););		\
    DBG_TRACE_CODE(fflush(dbgout););					\
    marshalDIFcounted(bs, DIF_SUSPEND);					\
    suspend(term);							\
  }									\
}

//
inline 
void DPMARSHALERCLASS::processLock(OZ_Term term, Tertiary *tert)
{
  DPMHandleTert("lock", tert, term, DIF_LOCK, return);
}
inline 
Bool DPMARSHALERCLASS::processCell(OZ_Term term, Tertiary *tert)
{
  DPMHandleTert("cell", tert, term, DIF_CELL, return(TRUE));
  return (TRUE);
}
inline 
void DPMARSHALERCLASS::processPort(OZ_Term term, Tertiary *tert)
{
  DPMHandleTert("port", tert, term, DIF_PORT, return);
}
inline 
void DPMARSHALERCLASS::processResource(OZ_Term term, Tertiary *tert)
{
  DPMHandleTert("resource", tert, term, DIF_RESOURCE, return);
}

#undef DPMHandleTert


//
// Remaining variables, i.e. those that have not beed exported during
// snapshot construction. Note that such variables need special
// attention in order to avoid run-away marshaling: is it safe today?
inline 
void DPMARSHALERCLASS::processVar(OZ_Term v, OZ_Term *vRef)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize + 
      max(max(MOwnHeadMaxSize, (MBorrowHeadMaxSize + 2*MGNameMaxSize)),
	  MToOwnerMaxSize)) {
    int index;
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> var = %s\n", toC(v));
    fflush(dbgout);
#endif

    //
    OZ_Term vrt = makeTaggedRef(vRef);
    VISITNODE(vrt, vIT, bs, index, return);

    //
    if (oz_isExtVar(v)) {
      ExtVarType evt = oz_getExtVar(v)->getIdV();
      switch (evt) {
      case OZ_EVAR_LAZY:
	oz_getLazyVar(v)->marshal(bs, index);
	// only a "place holding" patch is necessary:
	expVars = new MVarPatch(vrt, expVars);
	break;

      case OZ_EVAR_MANAGER:
	oz_getManagerVar(v)->marshal(bs, index);
	expVars = new MVarPatch(vrt, expVars);
	break;

      case OZ_EVAR_PROXY:
	oz_getProxyVar(v)->marshal(bs, index);
	expVars = new MVarPatch(vrt, expVars);
	break;

      case OZ_EVAR_MGRVARPATCH:
	oz_getMgrVarPatch(v)->marshal(bs, index);
	// is already a patch;
	break;

      case OZ_EVAR_PXYVARPATCH:
	oz_getPxyVarPatch(v)->marshal(bs, index);
	break;

      case OZ_EVAR_MVARPATCH:
      default:
	Assert(0);
	break;
      }

      //
      if (index) marshalTermDef(bs, index);

      //
    } else if (oz_isFree(v) || oz_isFuture(v)) {
      Assert(perdioInitialized);

      // make the variable needed first
      oz_var_makeNeeded(vRef);
      //
      ManagerVar *mvp = globalizeFreeVariable(vRef);
      mvp->marshal(bs, index);
      expVars = new MVarPatch(vrt, expVars);

      //
      if (index) marshalTermDef(bs, index);

      //
    } else {
      // Handle the variable as a resource. 

      // kost@ : this does not work currently in the sense that when a
      // variable is bound, its 'ref' owner entry will never be found
      // again, but instead marshaled as a value [the variable has been
      // bound to];
      OZ_warning("marshaling a variable as a resource!");
      // handle it as a resource. Note that co-references are already
      // taken care of;
      MarshalRHTentry(vrt, 0);
    }

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(v));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(makeTaggedRef(vRef));
  }
}

//
inline 
Bool DPMARSHALERCLASS::processLTuple(OZ_Term ltupleTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
    int index;
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_LIST].name, DIF_LIST, toC(ltupleTerm));
    fflush(dbgout);
#endif

    //
    VISITNODE(ltupleTerm, vIT, bs, index, return(OK));

    if (index) {
      marshalDIFcounted(bs, DIF_LIST_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIFcounted(bs, DIF_LIST);
    }
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processSRecord(OZ_Term srecordTerm)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 2*DIFMaxSize + 2*MNumberMaxSize) {
    int index;

    //
    VISITNODE(srecordTerm, vIT, bs, index, return(OK));

    //
    SRecord *rec = tagged2SRecord(srecordTerm);
    if (rec->isTuple()) {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_TUPLE].name, DIF_TUPLE, toC(srecordTerm));
      fflush(dbgout);
#endif
      if (index) {
	marshalDIFcounted(bs, DIF_TUPLE_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_TUPLE);
      }
      marshalNumber(bs, rec->getTupleWidth());
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_RECORD].name, DIF_RECORD, toC(srecordTerm));
      fflush(dbgout);
#endif
      if (index) {
	marshalDIFcounted(bs, DIF_RECORD_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_RECORD);
      }
    }

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{ 
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >=
      2*DIFMaxSize + MNumberMaxSize + MGNameMaxSize) {
    int index;

    //
    VISITNODE(chunkTerm, vIT, bs, index, return(OK));

    //
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_CHUNK].name, DIF_CHUNK, toC(chunkTerm));
    fflush(dbgout);
#endif
    if (index) {
      marshalDIFcounted(bs, DIF_CHUNK_DEF);
      marshalTermDef(bs, index);
    } else {
      marshalDIFcounted(bs, DIF_CHUNK);
    }

    //
    GName *gname  = globalizeConst((SChunk *) chunkConst);
    Assert(gname);
    marshalGName(bs, gname);

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processFSETValue(OZ_Term fsetvalueTerm)
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

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 
      max(2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize,
	  2*DIFMaxSize + 2*MNumberMaxSize)) {
    int index;

    //
    VISITNODE(dictTerm, vIT, bs, index, return(OK));

    //
    OzDictionary *d = (OzDictionary *) dictConst;
    if (!d->isSafeDict()) {
      MarshalRHTentry(dictTerm, index);
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (OK);
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_DICT].name, DIF_DICT, toC(dictTerm));
      fflush(dbgout);
#endif
      if (index) {
	marshalDIFcounted(bs, DIF_DICT_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_DICT);
      }
      marshalNumber(bs, d->getSize());
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (NO);
    }
    Assert(0);

    //
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
  Assert(0);
}

//
inline 
Bool DPMARSHALERCLASS::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  if (bs->availableSpace() >= 
      2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize) {
    int index;

    //
    VISITNODE(arrayTerm, vIT, bs, index, return(OK));
    //
    MarshalRHTentry(arrayTerm, index);

    //
    Assert(bs->availableSpace() >= DIFMaxSize);
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
inline 
Bool DPMARSHALERCLASS::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  //
  if (bs->availableSpace() >= 
      max(2*DIFMaxSize + 2*MNumberMaxSize + MGNameMaxSize, 
	  2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize)) {
    int index;

    //
    VISITNODE(classTerm, vIT, bs, index, return(OK));

    ObjectClass *cl = (ObjectClass *) classConst;
    // classes are not handled by the lazy protocol specially right
    // now, but they are still sent using SEND_LAZY, so the flag is to
    // be reset:
    doToplevel = FALSE;
    //
    if (cl->isSited()) {
      MarshalRHTentry(classTerm, index);
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (OK);		// done - a leaf;
    } else {

      //
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_CLASS].name, DIF_CLASS, toC(classTerm));
      fflush(dbgout);
#endif
      if (index) {
	marshalDIFcounted(bs, DIF_CLASS_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_CLASS);
      }

      //
      GName *gn = globalizeConst(cl);
      Assert(gn);
      marshalGName(bs, gn);
      marshalNumber(bs, cl->getFlags());
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (NO);
    }
    Assert(0);

    //
  } else {
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
  Assert(0);
}

//
inline 
Bool DPMARSHALERCLASS::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  if (bs->availableSpace() >= 
      max(2*DIFMaxSize + 7*MNumberMaxSize + MGNameMaxSize,
	  2*DIFMaxSize + MNumberMaxSize + MOwnHeadMaxSize)) {
    int index;

    //
    VISITNODE(absTerm, vIT, bs, index, return(OK));

    //
    Abstraction *pp = (Abstraction *) absConst;
    PrTabEntry *pred = pp->getPred();

    //
    if (pred->isSited()) {
      MarshalRHTentry(absTerm, index);
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (OK);		// done - a leaf;
    } else {
      //
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_PROC].name, DIF_PROC, toC(absTerm));
      fflush(dbgout);
#endif

      //
      if (index) {
	marshalDIFcounted(bs, DIF_PROC_DEF);
	marshalTermDef(bs, index);
      } else {
	marshalDIFcounted(bs, DIF_PROC);
      }

      //
      GName* gname = globalizeConst(pp);
      Assert(gname);
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
	new DPMarshalerCodeAreaDescriptor(start, start + nxt, lIT);
      traverseBinary(dpMarshalCode, desc);

      //
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (NO);
    } 
    Assert(0);

    //
  } else {
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
  Assert(0);
}

//
inline 
void DPMARSHALERCLASS::processSync()
{
  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  if (bs->availableSpace() >= 2*DIFMaxSize) {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d)\n", dif_names[DIF_SYNC].name, DIF_SYNC);
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SYNC);
    Assert(bs->availableSpace() >= DIFMaxSize);
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
#define	TRAVERSERCLASS	DPMARSHALERCLASS
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

