/* -*- C++ -*-
 *  Authors:
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Konstantin Popov, 2000
 * 
 *  Last change:
 *    $Date$
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

#include "base.hh"
#include "dpBase.hh"
#include "marshalerBase.hh"
#include "dpMarshaler.hh"
#include "dpMarshalExt.hh"
#include "bytedata.hh"

//
// Extensions that are marshaled need further support here: the
// space-conscious marshalers are defined here since they exploit
// ByteBuffer"s;

//
static int ebits = 0;
static int ebytes = 0;

//
void dpmExtInit()
{
  ebits = (int) oz_atom("bitString");
  ebytes = (int) oz_atom("byteString");
}

//
//
class DPMExtDesc : public GTAbstractEntity, public NMMemoryManager {
protected:
  OZ_Term term;
  // these appear to be common:
  int totalSize;
  int currentSize;

  //
public:
  DPMExtDesc(OZ_Term tIn)
    : term(tIn), totalSize(0), currentSize(0) {}
  virtual ~DPMExtDesc() {
    DebugCode(totalSize = currentSize = -1);
    DebugCode(term = (OZ_Term) -1);
  }

  //
  virtual int getType() { return (GT_ExtensionSusp); }
  virtual void gc() {
    // Observe: garbage in the byte arrays is GCed!
    Assert(term);
    oz_gCollectTerm(term, term);
  }

  //
  OZ_Term getTerm() { return (term); }

  //
  int getTotalSize() { return (totalSize); }
  void setTotalSize(int tsIn) {
    Assert(totalSize == 0);
    totalSize = tsIn;
  }

  int getCurrentSize() { return (currentSize); }
  void setCurrentSize(int csIn) {
    Assert(currentSize == 0);
    currentSize = csIn;
  }
  // 'inc' for unmarshaler, 'dec' for marshaler:
  void incCurrentSize(int csIn) {
    Assert(currentSize >= 0);
    currentSize += csIn;
    Assert(currentSize <= totalSize);
  }
  void decCurrentSize(int csIn) {
    Assert(currentSize <= totalSize);
    currentSize -= csIn;
    Assert(currentSize >= 0);
  }

  //
  // 'getExtID()' is needed for the marshaler; unmarshaler just checks
  // consistency with it;
  virtual int getExtID() = 0;
  virtual BYTE *getData() = 0;
  virtual void setData(BYTE *dataIn) = 0;
};

//
class DPMBitStringDesc : public DPMExtDesc {
protected:
  int index;			// into data;

  //
public:
  DPMBitStringDesc(OZ_Term tIn) : DPMExtDesc(tIn), index(0) {}
  virtual ~DPMBitStringDesc() { DebugCode(index = -1); }

  //
  virtual int getExtID() { return (OZ_E_BITSTRING); }

  //
  // Nothing is cached since we have also GC!!
  virtual BYTE *getData() {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == ebits);
    BYTE *data = ((BitString *) bs)->getData();
    return (data+index);
  }
  virtual void setData(BYTE *dataIn) {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == ebits);
    BYTE *data = ((BitString *) bs)->getData();
    Assert(dataIn >= data && dataIn < data + ((BitString *) bs)->getSize());
    index = dataIn - data;
  }
};

//
class DPMByteStringDesc : public DPMExtDesc {
protected:
  int index;			// into data;

  //
public:
  DPMByteStringDesc(OZ_Term tIn) : DPMExtDesc(tIn), index(0) {}
  virtual ~DPMByteStringDesc() { DebugCode(index = -1); }

  //
  virtual int getExtID() { return (OZ_E_BYTESTRING); }

  //
  // Nothing is cached since we have also GC!!
  BYTE *getData() {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == ebytes);
    BYTE *data = ((ByteString *) bs)->getData();
    return (data+index);
  }
  void setData(BYTE *dataIn) {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == ebytes);
    BYTE *data = ((ByteString *) bs)->getData();
    Assert(dataIn >= data);
    // since dataIn points to the first free cell, it points just
    // behind the array when we're done:
    Assert(dataIn <= data + ((ByteString *) bs)->getSize());
    index = dataIn - data;
  }
};

//
#if !defined(DEBUG_CHECK)
static 
#endif
void dpMarshalByteArrayCont(GenTraverser *gt, GTAbstractEntity *cont);
// DPMExtDesc *cont

//
static inline
void marshalByteArray(ByteBuffer *mb, GenTraverser *gt,
		      DPMExtDesc *desc)
{
  int availSpace = mb->availableSpace();
  int size = desc->getCurrentSize();
  BYTE *data = desc->getData();

  //
  // the current fragment's size must fit anyway:
  availSpace -= MNumberMaxSize;
  int ms = min(availSpace, size);

  //
  desc->decCurrentSize(ms);
  marshalNumber(mb, ms);	// current fragment;
  while (ms--)
    marshalByte(mb, *data++);
  desc->setData(data);

  //
  if (size > availSpace) {
    gt->suspendAC(dpMarshalByteArrayCont, desc);
  } else {
    delete desc;
  }
}

//
#if !defined(DEBUG_CHECK)
static
#endif
void dpMarshalByteArrayCont(GenTraverser *gt, GTAbstractEntity *arg)
{
  ByteBuffer *bs = (ByteBuffer *) gt->getOpaque();
  Assert(arg->getType() == GT_ExtensionSusp);
  DPMExtDesc *desc = (DPMExtDesc *) arg;

  // we should advance with marshaling:
  Assert(bs->availableSpace() > 2*DIFMaxSize + 2*MNumberMaxSize);
  marshalDIF(bs, DIF_EXT_CONT);
  marshalNumber(bs, desc->getExtID());

  //
  marshalByteArray(bs, gt, desc);
}

//
OZ_Term unmarshalByteArray(ByteBuffer *mb, DPMExtDesc *desc)
{
#ifdef USE_FAST_UNMARSHALER
  int cWidth = unmarshalNumber(mb);
#else
  // kost@ : TODO: use the 'error' code! (in 'unmarshalBitString'
  // too);
  int error;
  int cWidth = unmarshalNumberRobust(mb, &trash);
  if (error)
    return ((OZ_Term) 0);
#endif

  //
  Assert(cWidth >= 0);
  Assert(cWidth + desc->getCurrentSize() <= desc->getTotalSize());
  desc->incCurrentSize(cWidth);
  //
  BYTE *data = desc->getData();
  while (cWidth--)
    *data++ = mb->get();
  desc->setData(data);

  //
  if (desc->getCurrentSize() == desc->getTotalSize()) {
    OZ_Term bst = desc->getTerm();
    delete desc;
    return (bst);
  } else {
    return ((OZ_Term) -1);
  }
}

//
OZ_Term unmarshalBitString(ByteBuffer *mb, GTAbstractEntity* &bae)
{
#ifdef USE_FAST_UNMARSHALER
  int width = unmarshalNumber(mb);
#else
  int error;
  int width = unmarshalNumberRobust(mb, &error);
  if (error)
    return ((OZ_Term) 0);
#endif

  //
  BitString *s = new BitString(width);
  OZ_Term bst = makeTaggedExtension(s);
  DPMBitStringDesc *desc = new DPMBitStringDesc(bst);
  desc->setTotalSize(s->getSize());
  bae = desc;			// may be not necessary;

  //
  return (unmarshalByteArray(mb, desc));
}

//
OZ_Term unmarshalBitStringCont(ByteBuffer *mb, GTAbstractEntity* bae)
{
  Assert(bae->getType() == GT_ExtensionSusp);
#ifdef USE_FAST_UNMARSHALER
  Assert(((DPMExtDesc *) bae)->getExtID() == OZ_E_BITSTRING);
#else
  if (((DPMExtDesc *) bae)->getExtID() != OZ_E_BITSTRING) 
    return ((OZ_Term) 0);
#endif
  DPMBitStringDesc *desc = (DPMBitStringDesc *) bae;
  return (unmarshalByteArray(mb, desc));
}

//
OZ_Term unmarshalByteString(ByteBuffer *mb, GTAbstractEntity* &bae)
{
#ifdef USE_FAST_UNMARSHALER
  int width = unmarshalNumber(mb);
#else
  int error;
  int width = unmarshalNumberRobust(mb, &error);
  if (error)
    return ((OZ_Term) 0);
#endif

  //
  ByteString *s = new ByteString(width);
  OZ_Term bst = makeTaggedExtension(s);
  DPMByteStringDesc *desc = new DPMByteStringDesc(bst);
  desc->setTotalSize(s->getSize());
  bae = desc;

  //
  return (unmarshalByteArray(mb, desc));
}

//
OZ_Term unmarshalByteStringCont(ByteBuffer *mb, GTAbstractEntity* bae)
{
  Assert(bae->getType() == GT_ExtensionSusp);
#ifdef USE_FAST_UNMARSHALER
  Assert(((DPMExtDesc *) bae)->getExtID() == OZ_E_BYTESTRING);
#else
  if (((DPMExtDesc *) bae)->getExtID() != OZ_E_BYTESTRING) 
    return ((OZ_Term) 0);
#endif
  DPMByteStringDesc *desc = (DPMByteStringDesc *) bae;
  return (unmarshalByteArray(mb, desc));
}

// 
// Distribution marshaler- specific stuff for extensions;
static oz_suspUnmarshalProcType *suspUnmarshalRoutine = 0;
static int suspUnmarshalRoutineArraySize = 0;
static oz_unmarshalContProcType *unmarshalContRoutine = 0;
static int unmarshalContRoutineArraySize = 0;

//
OZ_Term oz_extension_unmarshal(int type, void *bs,
			       GTAbstractEntity* &arg)
{
  oz_suspUnmarshalProcType f = suspUnmarshalRoutine[type];
  Assert(f);
  return (f(bs, arg));
}

//
OZ_Term oz_extension_unmarshalCont(int type, void *bs,
				   GTAbstractEntity *arg)
{
  oz_unmarshalContProcType f = unmarshalContRoutine[type];
  Assert(f);
  return (f(bs, arg));
}

//
void oz_addExtSuspUnmarshaler(int type, oz_suspUnmarshalProcType f)
{
  if (suspUnmarshalRoutineArraySize <= type) {
    int newsize = type + 1;
    oz_suspUnmarshalProcType *n = new oz_suspUnmarshalProcType[newsize];
    for (int i = suspUnmarshalRoutineArraySize; i--; )
      n[i] = suspUnmarshalRoutine[i];
    if (suspUnmarshalRoutine) delete [] suspUnmarshalRoutine;
    suspUnmarshalRoutine = n;
    suspUnmarshalRoutineArraySize = newsize;
  }
  suspUnmarshalRoutine[type] = f;
}

//
void oz_addSuspContUnmarshaler(int type, oz_unmarshalContProcType f)
{
  if (unmarshalContRoutineArraySize <= type) {
    int newsize = type + 1;
    oz_unmarshalContProcType *n = new oz_unmarshalContProcType[newsize];
    for (int i = unmarshalContRoutineArraySize; i--; )
      n[i] = unmarshalContRoutine[i];
    if (unmarshalContRoutine)
      delete [] unmarshalContRoutine;
    unmarshalContRoutine = n;
    unmarshalContRoutineArraySize = newsize;
  }
  unmarshalContRoutine[type] = f;
}

//
void dpAddExtensions()
{
  oz_addExtSuspUnmarshaler(OZ_E_BITSTRING, unmarshalBitString);
  oz_addSuspContUnmarshaler(OZ_E_BITSTRING, unmarshalBitStringCont);
  oz_addExtSuspUnmarshaler(OZ_E_BYTESTRING, unmarshalByteString);
  oz_addSuspContUnmarshaler(OZ_E_BYTESTRING, unmarshalByteStringCont);
}

//
int dpMinNeededSpaceExt(OZ_Extension *oe)
{
  DebugCode(OZ_Term oet = oe->typeV());
  Assert(oet == ebits || oet == ebytes);
  return (MNumberMaxSize);
}

//
Bool dpMarshalExt(ByteBuffer *bs, GenTraverser *gt,
		  OZ_Term oet, OZ_Extension *oe)
{
  OZ_Term oetType = oe->typeV();
  if (oetType == ebits) {
    Assert(bs->availableSpace() >= MNumberMaxSize);
    DebugCode(int id = ((BitString *) oe)->getIdV());
    Assert(id == OZ_E_BITSTRING);

    //
    int width = ((BitString *) oe)->getWidth();
    marshalNumber(bs, width);

    //
    DPMBitStringDesc *desc = new DPMBitStringDesc(oet);
    int bss = ((BitString *) oe)->getSize();
    desc->setTotalSize(bss);
    desc->setCurrentSize(bss);	// to be marshaled;
    //
    marshalByteArray(bs, gt, desc);
  } else if (oetType == ebytes) {
    Assert(bs->availableSpace() >= MNumberMaxSize);
    DebugCode(int id = ((ByteString *) oe)->getIdV());
    Assert(id == OZ_E_BYTESTRING);

    //
    int size = ((ByteString *) oe)->getWidth();
    marshalNumber(bs, size);

    //
    DPMByteStringDesc *desc = new DPMByteStringDesc(oet);
    int bss = ((ByteString *) oe)->getSize();
    desc->setTotalSize(bss);
    desc->setCurrentSize(bss);	// to be marshaled;
    //
    marshalByteArray(bs, gt, desc);
  } else {
    OZ_error("dpMarshalExt: unknown extension!");
  }
  return (OK);
}
