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

#if defined(INTERFACE)
#pragma implementation "dpMarshaler.hh"
#endif

#include "base.hh"
#include "dpMarshaler.hh"
#include "dpInterface.hh"
#include "var_readonly.hh"
#include "gname.hh" 
#include "boot-manager.hh"

#include "dss_object.hh"

#include "glue_buffer.hh"
#include "glue_marshal.hh"



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

// The count of marshaled diffs should be done for the distributed
// messages and not for other marshaled structures.  Erik
inline
void marshalDIFcounted(MarshalerBuffer *bs, MarshalTag tag) {
  dif_counter[tag].send();
  marshalDIF(bs,tag);
}

// combined with an index: marshal either 'tag', or 'tagdef' and 'index'
inline
void marshalDIFindex(MarshalerBuffer *bs,
		     MarshalTag tag, MarshalTag tagdef, int index) {
  if (index) {
    marshalDIFcounted(bs, tagdef);
    marshalTermDef(bs, index);
  } else {
    marshalDIFcounted(bs, tag);
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
  Assert(bs->availableSpace() >= MNumberMaxSize);

  //
  int ms = min(nameSize, bs->availableSpace() - MNumberMaxSize);
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
static
void dpMarshalLitCont(GenTraverser *gt, GTAbstractEntity *arg)
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
#include "dpMarshalcode.cc"

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

#if defined(DEBUG_CHECK)
void DPMarshaler2ndP::vHook()
{}
#endif

//
#define DPMARSHALERCLASS DPMarshaler1stP
#define VISITNODE VisitNodeM1stP
#include "dpMarshalerCode.cc"
#undef VISITNODE
#undef DPMARSHALERCLASS

//
#define DPMARSHALERCLASS DPMarshaler2ndP
#define VISITNODE VisitNodeM2ndP
#include "dpMarshalerCode.cc"
#undef VISITNODE
#undef DPMARSHALERCLASS

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

//
// 
inline 
void VSnapshotBuilder::processSmallInt(OZ_Term siTerm) {}
inline 
void VSnapshotBuilder::processFloat(OZ_Term floatTerm) {}

inline 
void VSnapshotBuilder::processLiteral(OZ_Term litTerm)
{
  VisitNodeTrav(litTerm, vIT, return);
}
inline 
void VSnapshotBuilder::processBigInt(OZ_Term biTerm)
{
  VisitNodeTrav(biTerm, vIT, return);
}

inline 
void VSnapshotBuilder::processBuiltin(OZ_Term biTerm, ConstTerm *biConst)
{
  VisitNodeTrav(biTerm, vIT, return);
}
inline 
void VSnapshotBuilder::processExtension(OZ_Term et)
{
  VisitNodeTrav(et, vIT, return);
}

//
inline 
Bool VSnapshotBuilder::processObject(OZ_Term objTerm, ConstTerm *objConst)
{
  VisitNodeTrav(objTerm, vIT, return(TRUE));
  return (TRUE);
}
inline 
Bool VSnapshotBuilder::processObjectState(OZ_Term objTerm, ConstTerm *objConst)
{
  VisitNodeTrav(objTerm, vIT, return(TRUE));
  return (TRUE);
}
inline 
void VSnapshotBuilder::processNoGood(OZ_Term resTerm)
{
  Assert(!oz_isVar(resTerm));
  VisitNodeTrav(resTerm, vIT, return);
}
inline 
void VSnapshotBuilder::processLock(OZ_Term lockTerm, ConstTerm *lockConst)
{
  VisitNodeTrav(lockTerm, vIT, return);
}
inline 
Bool VSnapshotBuilder::processCell(OZ_Term cellTerm, ConstTerm *cellConst)
{
  VisitNodeTrav(cellTerm, vIT, return(TRUE));
  return (TRUE);
}
inline 
void VSnapshotBuilder::processPort(OZ_Term portTerm, ConstTerm *portConst)
{
  VisitNodeTrav(portTerm, vIT, return);
}
inline 
void VSnapshotBuilder::processResource(OZ_Term rTerm, ConstTerm *unusConst)
{
  VisitNodeTrav(rTerm, vIT, return);
}

//
inline 
Bool VSnapshotBuilder::processVar(OZ_Term v, OZ_Term *vRef)
{
  Assert(oz_isVar(v));
  OZ_Term vrt = makeTaggedRef(vRef);

  // Note: a variable is identified by its *location*.  Note that
  // patched variables, by construction, have already been visited, so
  // they return immediately here.
  VisitNodeTrav(vrt, vIT, return(OK));

  // failed values are not handled as variables here, just go through it
  if (oz_isFailed(v)) return NO;

  // globalize the variable if needed, and patch it
  glue_globalizeEntity(vrt);
  expVars = new DistributedVarPatch(vrt, expVars, false);
  return OK;
}

//
inline
Bool VSnapshotBuilder::processLTuple(OZ_Term ltupleTerm)
{
  VisitNodeTrav(ltupleTerm, vIT, return(TRUE));
  return (NO);
}
inline 
Bool VSnapshotBuilder::processSRecord(OZ_Term srecordTerm)
{
  VisitNodeTrav(srecordTerm, vIT, return(TRUE));
  return (NO);
}
inline 
Bool VSnapshotBuilder::processChunk(OZ_Term chunkTerm, ConstTerm *chunkConst)
{
  VisitNodeTrav(chunkTerm, vIT, return(TRUE));
  // return false if the chunk is marshaled immediately
  return !(isImmediate() || glue_isImmediate(chunkTerm));
}

//
inline 
Bool VSnapshotBuilder::processFSETValue(OZ_Term fsetvalueTerm)
{
  return (NO);
}

//
inline Bool
VSnapshotBuilder::processDictionary(OZ_Term dictTerm, ConstTerm *dictConst)
{
  VisitNodeTrav(dictTerm, vIT, return(TRUE));
  OzDictionary *d = (OzDictionary *) dictConst;
  return (d->isSafeDict() ? NO : OK);
}

//
inline Bool
VSnapshotBuilder::processArray(OZ_Term arrayTerm, ConstTerm *arrayConst)
{
  VisitNodeTrav(arrayTerm, vIT, return(TRUE));
  return (OK);
}

//
inline Bool
VSnapshotBuilder::processClass(OZ_Term classTerm, ConstTerm *classConst)
{ 
  VisitNodeTrav(classTerm, vIT, return(TRUE));
  //
  if (((OzClass *) classConst)->isSited()) return (OK);
  // return false if the class is marshaled immediately
  return !(isImmediate() || glue_isImmediate(classTerm));
}

//
inline Bool
VSnapshotBuilder::processAbstraction(OZ_Term absTerm, ConstTerm *absConst)
{
  VisitNodeTrav(absTerm, vIT, return(TRUE));

  // return OK if handled by the Glue
  if (!(isImmediate() || glue_isImmediate(absTerm))) return (OK);

  //
  Abstraction *pp = (Abstraction *) absConst;
  if (!pp->isComplete()) return OK;     // not nice...

  PrTabEntry *pred = pp->getPred();
  //
  if (pred->isSited()) {
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
      new DPMarshalerCodeAreaDescriptor(start, start + nxt, 
					(AddressHashTableO1Reset *) 0);
    traverseBinary(traverseCode, desc);
    return (NO);
  }
  Assert(0);
}

//
inline 
void VSnapshotBuilder::processSync() {}

//
#define	TRAVERSERCLASS	VSnapshotBuilder
#include "gentraverserLoop.cc"
#undef	TRAVERSERCLASS

//
// Not in use since we take a snapshot of a value ahead of
// marshaling now;

static void contProcNOOP(GenTraverser *gt, GTAbstractEntity *cont)
{}

//
void VSnapshotBuilder::copyStack(DPMarshaler *dpm)
{
  StackEntry *copyTop = dpm->getTop();
  StackEntry *copyBottom = dpm->getBottom();
  StackEntry *top;
  int entries;

  //
  entries = copyTop - copyBottom;
  top = extendBy(entries);
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
	  StackEntry arg = *(--copyTop);
	  TraverserBinaryAreaProcessor proc =
	    (TraverserBinaryAreaProcessor) *(--copyTop);
	  Assert(proc == dpMarshalCode);

	  //
	  DPMarshalerCodeAreaDescriptor *desc =
	    (DPMarshalerCodeAreaDescriptor *) arg;
	  DPMarshalerCodeAreaDescriptor *descCopy;
	  if (desc)
	    descCopy = new DPMarshalerCodeAreaDescriptor(*desc);
	  else
	    descCopy = (DPMarshalerCodeAreaDescriptor *) 0;
	  *(--top) = (StackEntry) descCopy;
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


//
//
VSnapshotBuilder vsb;

/* *********************************************************************/
/*   interface to Oz-core                                  */
/* *********************************************************************/

static 
char *tagToComment(MarshalTag tag)
{
  switch(tag){
  case DIF_PORT:
  case DIF_PORT_DEF:
    return "port";
  case DIF_CELL:
  case DIF_CELL_DEF:
    return "cell";
  case DIF_LOCK:
  case DIF_LOCK_DEF:
    return "lock";
  case DIF_OBJECT:
  case DIF_OBJECT_DEF:
  case DIF_VAR_OBJECT:
  case DIF_VAR_OBJECT_DEF:
  case DIF_STUB_OBJECT:
  case DIF_STUB_OBJECT_DEF:
    return "object";
  case DIF_RESOURCE:
  case DIF_RESOURCE_DEF:
    return "resource";
  default:
    Assert(0);
    return "";
}}

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
OZ_Term dpUnmarshalTerm(ByteBuffer *bs, Builder *b)
{
  Assert(oz_onToplevel());
#if defined(DBG_TRACE)
  DBGINIT();
  fprintf(dbgout, "<bs: %p (getptr=%p,posMB=%p,endMB=%p)\n",
	  bs, bs->getGetptr(), bs->getPosMB(), bs->getEndMB());
  fflush(dbgout);
#endif

  while(1) {
    MarshalTag tag = (MarshalTag) bs->get();
    Assert(tag < DIF_LAST);
    dif_counter[tag].recv();	// kost@ : TODO: needed?
#if defined(DBG_TRACE)
    fprintf(dbgout, "< tag: %s(%d)", dif_names[tag].name, tag);
    fflush(dbgout);
#endif
    switch (tag) {
	
    case DIF_SMALLINT: 
      {
	OZ_Term ozInt = OZ_int(unmarshalNumber(bs));
#if defined(DBG_TRACE)
	fprintf(dbgout, " = %d\n", ozInt);
	fflush(dbgout);
#endif
	b->buildValue(ozInt);
	break;
      }

    case DIF_FLOAT:
      {
	double f = unmarshalFloat(bs);
#if defined(DBG_TRACE)
	fprintf(dbgout, " = %f\n", f);
	fflush(dbgout);
#endif
	b->buildValue(OZ_float(f));
	break;
      }

    case DIF_NAME_DEF:
      {
	OZ_Term value;
	int refTag = unmarshalRefTag(bs);
	GName *gname = unmarshalGName(&value, bs);
	int nameSize = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
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
	  b->setTerm(value, refTag);
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	  break;
	}
      }

    case DIF_NAME:
      {
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	int nameSize = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
	int strLen = strlen(printname);

	//
	// May be, we don't have a complete print name yet:
	if (nameSize > strLen) {
	  DPUnmarshalerNameSusp *ns =
	    new DPUnmarshalerNameSusp(0, nameSize, value,
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
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	  break;
	}
      }

    case DIF_COPYABLENAME_DEF:
      {
	int refTag      = unmarshalRefTag(bs);
	int nameSize  = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
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
	  b->setTerm(value, refTag);
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_COPYABLENAME:
      {
	int nameSize  = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
	int strLen = strlen(printname);

	//
	// May be, we don't have a complete print name yet:
	if (nameSize > strLen) {
	  DPUnmarshalerCopyableNameSusp *cns =
	    new DPUnmarshalerCopyableNameSusp(0, nameSize,
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
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_UNIQUENAME_DEF:
      {
	int refTag      = unmarshalRefTag(bs);
	int nameSize  = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
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
	  b->setTerm(value, refTag);
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_UNIQUENAME:
      {
	int nameSize  = unmarshalNumber(bs);
	char *printname = unmarshalString(bs);
	int strLen = strlen(printname);

	//
	if (nameSize > strLen) {
	  DPUnmarshalerUniqueNameSusp *uns = 
	    new DPUnmarshalerUniqueNameSusp(0, nameSize,
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
	  delete [] printname;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_ATOM_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int nameSize  = unmarshalNumber(bs);
	char *aux  = unmarshalString(bs);
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
	  b->setTerm(value, refTag);
	  delete [] aux;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_ATOM:
      {
	int nameSize  = unmarshalNumber(bs);
	char *aux  = unmarshalString(bs);
	int strLen = strlen(aux);

	//
	if (nameSize > strLen) {
	  DPUnmarshalerAtomSusp *as = 
	    new DPUnmarshalerAtomSusp(0, nameSize, aux, strLen);
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
	  delete [] aux;
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	}
	break;
      }

      //
    case DIF_LIT_CONT:
      {
	char *aux  = unmarshalString(bs);
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
	      int refTag = ns->getRefTag();
	      if (refTag)
		b->setTerm(value, refTag);
	      delete ns;
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
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
	      int refTag = as->getRefTag();
	      if (refTag)
		b->setTerm(value, refTag);
	      delete as;
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
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
	      int refTag = uns->getRefTag();
	      if (refTag)
		b->setTerm(value, refTag);
	      delete uns;
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
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
	      int refTag = cns->getRefTag();
	      if (refTag)
		b->setTerm(value, refTag);
	      delete cns;
#if defined(DBG_TRACE)
	      fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	      fflush(dbgout);
#endif
	    }
	    break;
	  }

	default:
	  OZ_error("Illegal GTAbstractEntity (builder) for a LitCont!");
	  b->buildValue(oz_nil());
	}
	break;
      }

    case DIF_BIGINT:
      {
	char *aux  = unmarshalString(bs);
#if defined(DBG_TRACE)
	fprintf(dbgout, " %s\n", aux);
	fflush(dbgout);
#endif
	b->buildValue(OZ_CStringToNumber(aux));
	delete [] aux;
	break;
      }

    case DIF_BIGINT_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	char *aux  = unmarshalString(bs);
#if defined(DBG_TRACE)
	fprintf(dbgout, " %s\n", aux);
	fflush(dbgout);
#endif
	OZ_Term value = OZ_CStringToNumber(aux);
	b->buildValue(value);
	b->setTerm(value, refTag);
	delete [] aux;
	break;
      }

    case DIF_LIST_DEF:
      {
	int refTag = unmarshalRefTag(bs);
#if defined(DBG_TRACE)
	fprintf(dbgout, " (at %d)\n", refTag);
	fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	fprintf(dbgout, " (at %d)\n", refTag);
	fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	fprintf(dbgout, " (at %d)\n", refTag);
	fflush(dbgout);
#endif
	b->buildRecordRemember(refTag);
	break;
      }

    case DIF_RECORD:
      b->buildRecord();
      break;

    case DIF_REF:
      {
	int i = unmarshalNumber(bs);
#if defined(DBG_TRACE)
	fprintf(dbgout, " (from %d)\n", i);
	fflush(dbgout);
#endif
	b->buildValueRef(i);
	break;
      }

    case DIF_OWNER:
    case DIF_OWNER_DEF:
    case DIF_PORT:
    case DIF_PORT_DEF:
    case DIF_CELL:
    case DIF_CELL_DEF:
    case DIF_LOCK:
    case DIF_LOCK_DEF:
    case DIF_ARRAY:
    case DIF_ARRAY_DEF:
    case DIF_VAR_OBJECT:
    case DIF_VAR_OBJECT_DEF:
    case DIF_STUB_OBJECT:
    case DIF_STUB_OBJECT_DEF:
    case DIF_RESOURCE_DEF:
    case DIF_RESOURCE:
      {
        OZ_error("Unmarshaling tags from the old system");
        break;
      }

    // Unmmarshaling stateless dictionaries.
    case DIF_DICT_DEF:
      {
        int refTag = unmarshalRefTag(bs);
        int size   = unmarshalNumber(bs);
        Assert(oz_onToplevel());
        b->buildDictionaryRemember(size,refTag);
#if defined(DBG_TRACE)
        fprintf(dbgout, " size=%d (at %d)\n", size, refTag);
        fflush(dbgout);
#endif
        break;
      }

    case DIF_DICT:
      {
        int size   = unmarshalNumber(bs);
        Assert(oz_onToplevel());
        b->buildDictionary(size);
#if defined(DBG_TRACE)
        fprintf(dbgout, " size=%d\n", size);
        fflush(dbgout);
#endif
        break;
      }

    case DIF_CHUNK_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	if (gname) {
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (at %d)\n", refTag);
	  fflush(dbgout);
#endif
	  b->buildChunkRemember(gname, refTag);
	} else if (!tagged2SChunk(value)->getValue()) {   // lazy protocol
	  b->buildChunkRemember(tagged2SChunk(value)->getGName(), refTag);
	} else {
	  b->knownChunk(value);
	  b->setTerm(value, refTag);
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_CHUNK:
      {
	OZ_Term value;
	GName *gname = unmarshalGName(&value, bs);
	if (gname) {
	  b->buildChunk(gname);
	} else if (!tagged2SChunk(value)->getValue()) {   // lazy protocol
	  b->buildChunk(tagged2SChunk(value)->getGName());
	} else {
	  b->knownChunk(value);
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_CLASS_DEF:
      {
	OZ_Term value;
	int refTag = unmarshalRefTag(bs);
	GName *gname = unmarshalGName(&value, bs);
	int flags = unmarshalNumber(bs);
	//
	if (gname) {
#if defined(DBG_TRACE)
	  fprintf(dbgout, " (at %d)\n", refTag);
	  fflush(dbgout);
#endif
	  b->buildClassRemember(gname, flags, refTag);
	} else if (!tagged2OzClass(value)->isComplete()) {
	  b->buildClassRemember(tagged2OzClass(value)->getGName(),
				flags, refTag);
	} else {
	  // optimization: we already have the class
	  b->knownClass(value);
	  b->setTerm(value, refTag);
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
	GName *gname = unmarshalGName(&value, bs);
	int flags = unmarshalNumber(bs);
	//
	if (gname) {
	  b->buildClass(gname, flags);
	} else if (!tagged2OzClass(value)->isComplete()) {
	  b->buildClass(tagged2OzClass(value)->getGName(), flags);
	} else {
	  // optimization: we already have the class
	  b->knownClass(value);
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	}
	break;
      }

    case DIF_VAR_DEF:
    case DIF_VAR:
    case DIF_READONLY_DEF:
    case DIF_READONLY:
    case DIF_OBJECT_DEF:
    case DIF_OBJECT:
      {
	OZ_error("Unmarshaling tags from the old system");
	break;
      }

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

	// in case of an incomplete abstraction, pretend it is unknonw
	if (!gname && !tagged2Abstraction(value)->isComplete())
	  gname = tagged2Abstraction(value)->getGName();

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
	  Assert(codesize > 0);
	  DPBuilderCodeAreaDescriptor *desc =
	    new DPBuilderCodeAreaDescriptor(start, start+codesize, code);
	  b->buildBinary(desc);

	  //
	  b->buildProc(gname, arity, gsize, maxX, line, column, pc);
#if defined(DBG_TRACE)
	  fprintf(dbgout,
		  " do arity=%d,line=%d,column=%d,codesize=%d\n",
		  arity, line, column, codesize);
	  fflush(dbgout);
#endif
	} else {
	  Assert(oz_isAbstraction(oz_deref(value)));
	  // ('zero' descriptions are not allowed;)
	  DPBuilderCodeAreaDescriptor *desc =
	    new DPBuilderCodeAreaDescriptor(0, 0, 0);
	  b->buildBinary(desc);

	  //
	  b->knownProc(value);
#if defined(DBG_TRACE)
	  fprintf(dbgout,
		  " skip %s (arity=%d,line=%d,column=%d,codesize=%d)\n",
		  toC(value), arity, line, column, codesize);
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
	break;
      }

/*    case DIF_DICT_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int size   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildDictionaryRemember(size,refTag);
#if defined(DBG_TRACE)
	fprintf(dbgout, " size=%d (at %d)\n", size, refTag);
	fflush(dbgout);
#endif
	break;
      }

    case DIF_DICT:
      {
	int size   = unmarshalNumber(bs);
	Assert(oz_onToplevel());
	b->buildDictionary(size);
#if defined(DBG_TRACE)
	fprintf(dbgout, " size=%d\n", size);
	fflush(dbgout);
#endif
	break;
      }
*/
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
#if defined(DBG_TRACE)
	fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	fflush(dbgout);
#endif
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
#if defined(DBG_TRACE)
	fprintf(dbgout, " = %s\n", toC(value));
	fflush(dbgout);
#endif
	break;
      }

    case DIF_EXTENSION_DEF:
      {
	int refTag = unmarshalRefTag(bs);
	int type = unmarshalNumber(bs);

	//
	GTAbstractEntity *bae;
	OZ_Term value = oz_extension_unmarshal(type, bs, b, bae);
	switch (value) {
	case UnmarshalEXT_Susp:
	  Assert(bae);
	  b->getAbstractEntity(bae);
	  b->suspend();
#if defined(DBG_TRACE)
	  fprintf(dbgout, " suspended (for %d)\n", refTag);
	  fflush(dbgout);
#endif
	  return ((OZ_Term) -1);

	case UnmarshalEXT_Error:
	  OZ_error("Trouble with unmarshaling an extension!");
	  b->buildValue(oz_nil());
	  break;

	default:		// got it!
	  b->buildValue(value);
	  b->setTerm(value, refTag);
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s (at %d)\n", toC(value), refTag);
	  fflush(dbgout);
#endif
	  break;
	}
	break;
      }

    case DIF_EXTENSION:
      {
	int type = unmarshalNumber(bs);

	//
	GTAbstractEntity *bae;
	OZ_Term value = oz_extension_unmarshal(type, bs, b, bae);
	switch (value) {
	case UnmarshalEXT_Susp:
	  Assert(bae);
	  b->getAbstractEntity(bae);
	  b->suspend();
#if defined(DBG_TRACE)
	  fprintf(dbgout, " suspended\n");
	  fflush(dbgout);
#endif
	  return ((OZ_Term) -1);

	case UnmarshalEXT_Error:
	  OZ_error("Trouble with unmarshaling an extension!");
	  b->buildValue(oz_nil());
	  break;

	default:		// got it!
	  b->buildValue(value);
#if defined(DBG_TRACE)
	  fprintf(dbgout, " = %s\n", toC(value));
	  fflush(dbgout);
#endif
	  break;
	}
	break;
      }

      // Continuation for "extensions";
    case DIF_EXT_CONT:
      {
	int type = unmarshalNumber(bs);

	//
	GTAbstractEntity *bae = b->buildAbstractEntity();
	Assert(bae->getType() == GT_ExtensionSusp);
	OZ_Term value = oz_extension_unmarshalCont(type, bs, b, bae);
	switch (value) {
	case UnmarshalEXT_Susp:
	  b->getAbstractEntity(bae);
	  b->suspend();
#if defined(DBG_TRACE)
	  fprintf(dbgout, " suspended\n");
	  fflush(dbgout);
#endif
	  return ((OZ_Term) -1);

	case UnmarshalEXT_Error:
	  OZ_error("Trouble with unmarshaling an extension!");
	  b->buildValue(oz_nil());
	  break;

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

    case DIF_FAILEDVALUE:
      b->buildFailedValue();
      break;

    case DIF_FAILEDVALUE_DEF: {
      int refTag = unmarshalRefTag(bs);
      b->buildFailedValueRemember(refTag);
      break;
    }

    case DIF_GLUE: {
      OZ_Term t;
      bool immediate = glue_unmarshalEntity(bs, t);
      Assert(!immediate);     // not handled here
#if defined(DBG_TRACE)
	fprintf(dbgout, " = %s\n", toC(t));
	fflush(dbgout);
#endif
      b->buildValue(t);
      break;
    }

    case DIF_GLUE_DEF: {
      int refTag = unmarshalRefTag(bs);
      OZ_Term t;
      bool immediate = glue_unmarshalEntity(bs, t);
      Assert(!immediate);     // not handled here
#if defined(DBG_TRACE)
      fprintf(dbgout, " = %s (at %d)\n", toC(t), refTag);
      fflush(dbgout);
#endif
      b->buildValueRemember(t, refTag);
      break;
    }

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

    default:
      OZ_error("unmarshal: unexpected tag: %d\n",tag);
      b->buildValue(oz_nil());
      break;
    }
  }
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
  for (int i = musNum; i--; ) {
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
  for (int i = musNum; i--; ) {
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
  for (int i = musNum; i--; ) {
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
  for (int i = musNum; i--; ) {
    if (mus[i].b == dpb) {
      Assert(mus[i].flags & MUBuilderBusy);
      mus[i].flags = mus[i].flags & ~MUBuilderBusy;
      return;
    }
  }
  OZ_error("dpReturnMarshaler got an unallocated builder!!");
}
