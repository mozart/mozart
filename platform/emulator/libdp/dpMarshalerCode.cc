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
 *    Boriss Mejias <bmc@info.ucl.ac.be>
 *    Raphael Collet <raph@info.ucl.ac.be>
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
    marshalDIFindex(bs, DIF_BIGINT, DIF_BIGINT_DEF, index);
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
      marshalDIFindex(bs, DIF_ATOM, DIF_ATOM_DEF, index);
    } else if (lit->isUniqueName()) {
      marshalDIFindex(bs, DIF_UNIQUENAME, DIF_UNIQUENAME_DEF, index);
    } else if (lit->isCopyableName()) {
      marshalDIFindex(bs, DIF_COPYABLENAME, DIF_COPYABLENAME_DEF, index);
    } else {
      marshalDIFindex(bs, DIF_NAME, DIF_NAME_DEF, index);
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
  processGlue(resTerm);
}

//
inline
void DPMARSHALERCLASS::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  Builtin *bi = (Builtin *) biConst;

  if (bi->isSited())
    return processGlue(biTerm);     // sited builtins are handled by the Glue

  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  const char *pn = bi->getPrintName();

  if (bs->availableSpace() >= 2*DIFMaxSize + 2*MNumberMaxSize + strlen(pn)) {
    int index;
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_BUILTIN].name, DIF_BUILTIN, toC(biTerm));
      fflush(dbgout);
#endif
    //
    VISITNODE(biTerm, vIT, bs, index, return);
    //
    marshalDIFindex(bs, DIF_BUILTIN, DIF_BUILTIN_DEF, index);
    marshalString(bs, pn);
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
      marshalDIFindex(bs, DIF_EXTENSION, DIF_EXTENSION_DEF, index);
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

//
inline 
Bool DPMARSHALERCLASS::processObject(OZ_Term term, ConstTerm *objConst)
{
  return processGlue(term);     // handled by the Glue
}

//
inline 
Bool DPMARSHALERCLASS::processObjectState(OZ_Term term, ConstTerm *stateConst)
{
  return processGlue(term);     // handled by the Glue
}

//
inline 
void DPMARSHALERCLASS::processLock(OZ_Term term, ConstTerm *lockConst)
{
  processGlue(term);     // handled by the Glue
}

inline 
Bool DPMARSHALERCLASS::processCell(OZ_Term term, ConstTerm *cellConst)
{
  return processGlue(term);     // handled by the Glue
}

inline 
void DPMARSHALERCLASS::processPort(OZ_Term term, ConstTerm *portConst)
{
  processGlue(term);     // handled by the Glue
}

inline 
void DPMARSHALERCLASS::processResource(OZ_Term term, ConstTerm *unusConst)
{
  processGlue(term);     // handled by the Glue
}

//
// Remaining variables, i.e. those that have not beed exported during
// snapshot construction. Note that such variables need special
// attention in order to avoid run-away marshaling: is it safe today?
inline 
Bool DPMARSHALERCLASS::processVar(OZ_Term v, OZ_Term *vRef)
{
  // v == *vRef && oz_isVar(v)
  OZ_Term term = makeTaggedRef(vRef);

  if (oz_isExtVar(v) &&
      oz_getExtVar(v)->getIdV() == OZ_EVAR_DISTRIBUTEDVARPATCH) {
    // a patch, marshal it
    return processGlue(term);

  } else if (oz_isFree(v) || oz_isReadOnly(v)) {
    // marshal it
    processGlue(term);
    // patch it
    expVars = new MarshaledVarPatch(term, expVars);
    return (OK);

  } else if (oz_isFailed(v)) {
    // specific marshaling for failed values
    ByteBuffer *bs = (ByteBuffer *) getOpaque();

    if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize) {
      int index;
      VISITNODE(term, vIT, bs, index, return(OK));
      // marshal tag
      marshalDIFindex(bs, DIF_FAILEDVALUE, DIF_FAILEDVALUE_DEF, index);
      //
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (NO);     // recurse through the exception

    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) on %s\n",
	      dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(v));
      fflush(dbgout);
#endif
      marshalDIFcounted(bs, DIF_SUSPEND);
      suspend(term);
      return (OK);
    }

  } else {
    // error: this variable cannot be marshaled!
    OZ_error("cannot marshal variable!\n");
    return (OK);
  }
  Assert(0);
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

    marshalDIFindex(bs, DIF_LIST, DIF_LIST_DEF, index);
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
      marshalDIFindex(bs, DIF_TUPLE, DIF_TUPLE_DEF, index);
      marshalNumber(bs, rec->getTupleWidth());
    } else {
#if defined(DBG_TRACE)
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	      dif_names[DIF_RECORD].name, DIF_RECORD, toC(srecordTerm));
      fflush(dbgout);
#endif
      marshalDIFindex(bs, DIF_RECORD, DIF_RECORD_DEF, index);
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
  if (!(isImmediate() || glue_isImmediate(chunkTerm)))
    return processGlue(chunkTerm);     // handled by the Glue

  // immediate marshaling: DIF_CHUNK, index, gname
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  if (bs->availableSpace() >= 2*DIFMaxSize + MNumberMaxSize + MGNameMaxSize) {
    int index;

    VISITNODE(chunkTerm, vIT, bs, index, return(OK));
    //
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_CHUNK].name, DIF_CHUNK, toC(chunkTerm));
    fflush(dbgout);
#endif
    marshalDIFindex(bs, DIF_CHUNK, DIF_CHUNK_DEF, index);
    //
    GName *gname = ((SChunk *)chunkConst)->globalize();
    Assert(gname);
    marshalGName(bs, gname);
    //
    setImmediate(false);     // back to normal mode
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
  Assert(0);
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
  OzDictionary *d = (OzDictionary *) dictConst;

  if (!d->isSafeDict())
    return processGlue(dictTerm);     // handled by the Glue

  ByteBuffer *bs = (ByteBuffer *) getOpaque();
  //
  if (bs->availableSpace() >= 2*DIFMaxSize + 2*MNumberMaxSize) {
    int index;
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_DICT].name, DIF_DICT, toC(dictTerm));
    fflush(dbgout);
#endif
    VISITNODE(dictTerm, vIT, bs, index, return(OK));
    //
    marshalDIFindex(bs, DIF_DICT, DIF_DICT_DEF, index);
    marshalNumber(bs, d->getSize());
    Assert(bs->availableSpace() >= DIFMaxSize);
    return (NO);

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
  return processGlue(arrayTerm);     // handled by the Glue
}

//
inline 
Bool DPMARSHALERCLASS::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  if (!(isImmediate() || glue_isImmediate(classTerm)))
    return processGlue(classTerm);

  // immediate marshaling: DIF_CLASS, index, gname, number
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  if (bs->availableSpace() >=
      2*DIFMaxSize + 2*MNumberMaxSize + MGNameMaxSize) {

    int index;
    VISITNODE(classTerm, vIT, bs, index, return(OK));

    OzClass *cl = (OzClass *) classConst;
    //
    if (cl->isSited()) {   // stupidness check: should not happen
      OZ_error("DPMARSHALERCLASS::processClass : MRHTentry\n");
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (OK);		// done - a leaf;
    }
    //
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_CLASS].name, DIF_CLASS, toC(classTerm));
    fflush(dbgout);
#endif
    marshalDIFindex(bs, DIF_CLASS, DIF_CLASS_DEF, index);
    //
    GName *gn = cl->globalize();
    Assert(gn);
    marshalGName(bs, gn);
    marshalNumber(bs, cl->getFlags());
    //
    setImmediate(false);     // back to normal mode
    //
    Assert(bs->availableSpace() >= DIFMaxSize);
    return (NO);

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
}

//
inline 
Bool DPMARSHALERCLASS::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  if (!(isImmediate() || glue_isImmediate(absTerm)))
    return processGlue(absTerm);     // handled by the Glue

  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  // immediate marshaling: DIF_PROC, index, gname, arity, gsize, maxX,
  // line, column, bytecode size
  if (bs->availableSpace() >=
      2*DIFMaxSize + 7*MNumberMaxSize + MGNameMaxSize) {
    int index;
    //
    VISITNODE(absTerm, vIT, bs, index, return(OK));
    //
    Abstraction *pp = (Abstraction *) absConst;
    Assert(pp->isComplete());
    PrTabEntry *pred = pp->getPred();
    //
    if (pred->isSited()) {   // stupidness check: should not happen
      OZ_error("DPMARSHALERCLASS::processAbstract : MRHTentry\n");
      Assert(bs->availableSpace() >= DIFMaxSize);
      return (OK);		// done - a leaf;
    }
    //
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_PROC].name, DIF_PROC, toC(absTerm));
    fflush(dbgout);
#endif
    marshalDIFindex(bs, DIF_PROC, DIF_PROC_DEF, index);
    //
    GName *gname = pp->globalize();
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

// for entities managed by the Glue
inline
Bool DPMARSHALERCLASS::processGlue(OZ_Term entity) {
  ByteBuffer *bs = (ByteBuffer *) getOpaque();

  // marshaling: DIF_GLUE, index, mediator, and a possible DIF_SUSPEND
  if (bs->availableSpace() >=
      2*DIFMaxSize + MNumberMaxSize + glue_getMarshaledSize(entity)) {
    int index;
#if defined(DBG_TRACE)
    {
      DBGINIT();
      fprintf(dbgout, "> tag: %s(%d) = %s\n",
	    dif_names[DIF_GLUE].name, DIF_GLUE, toC(entity));
      fflush(dbgout);
    }
#endif
    //
    VISITNODE(entity, vIT, bs, index, return);
    //
    marshalDIFindex(bs, DIF_GLUE, DIF_GLUE_DEF, index);
    (void) glue_marshalEntity(entity, bs);
    //
    Assert(bs->availableSpace() >= DIFMaxSize);
  } else {
#if defined(DBG_TRACE)
    DBGINIT();
    fprintf(dbgout, "> tag: %s(%d) on %s\n",
	    dif_names[DIF_SUSPEND].name, DIF_SUSPEND, toC(entity));
    fflush(dbgout);
#endif
    marshalDIFcounted(bs, DIF_SUSPEND);
    suspend(entity);
  }
  return (OK);
}

//
#define	TRAVERSERCLASS	DPMARSHALERCLASS
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

