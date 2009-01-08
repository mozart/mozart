/*
 *  Authors:
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *
 *  Copyright:
 *    Denys Duchier (1998)
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

#include "bytedata.hh"
#include "pickle.hh"
// kost@ : distribution's byte buffer. It is needed for "suspendable"
//         marshaling only. The byteBuffer.hh does not contain/should
//         not contain nothing distribution-specific. Just the byte
//         buffer abstraction..
#include "byteBuffer.hh"

#include "string.h"

// -------------------------------------------------------------------
// BitData
// -------------------------------------------------------------------

void BitData::bitPrintStream(ostream& out) {
  int w = getWidth();
  for (int i=0; i<w; i++) out << ((get(i))?"1":"0");
}

Bool BitData::equal(BitData *s) {
  if (width != s->width) return 0;
  int size = getSize();
  for (int i=0; i<size; i++)
    if (data[i] != s->data[i]) return 0;
  return 1;
}

int BitData::get(int i) {
  Assert(checkIndex(i));
  return data[i/BITS_PER_BYTE] &  (1<<(i%BITS_PER_BYTE));
}

void BitData::put(int i,Bool on) {
  Assert(checkIndex(i));
  if (on)
    data[i/BITS_PER_BYTE] |=  (1<<(i%BITS_PER_BYTE));
  else
    data[i/BITS_PER_BYTE] &= ~(1<<(i%BITS_PER_BYTE));
}

void BitData::conj(BitData* s) {
  Assert(width==s->width);
  int size = getSize();
  for (int i=0; i<size; i++) data[i] &= s->data[i];
}

void BitData::disj(BitData* s) {
  Assert(width==s->width);
  int size = getSize();
  for (int i=0; i<size; i++) data[i] |= s->data[i];
}

// after a negation, we must zero out the highest
// (non-significant) leftover bits

#define ZERO_LEFT_OVER  {                       \
  int spill = width % BITS_PER_BYTE;            \
  if (spill==0) return;                         \
  data[size-1] &= ~(((BYTE) ~0) << spill);      \
}

void BitData::nega() {
  int size = getSize();
  for (int i=0; i<size; i++) data[i] = ~ data[i];
  ZERO_LEFT_OVER;
}

void BitData::nimpl(BitData* s) {
  Assert(width==s->width);
  int size = getSize();
  for (int i=0; i<size; i++) data[i] &= ~ s->data[i];
  ZERO_LEFT_OVER;
}

Bool BitData::disjoint(BitData* s) {
  Assert(width==s->width);
  int size = getSize();
  for (int i=0; i<size; i++)
    if ((data[i] & s->data[i]) != 0) return 0;
  return 1;
}

int BitData::card() {
  int n=0;
  int size = getSize();
  for (int i=0; i<size; i++) {
    BYTE b = data[i];
    while (b) {
      if (b&1) n++;
      b >>= 1;
    }
  }
  return n;
}

// -------------------------------------------------------------------
// BitString
// -------------------------------------------------------------------

OZ_Term BitString::typeV()
{
  return (AtomBitString);
}

OZ_Return BitString::eqV(OZ_Term t)
{
  return (oz_isBitString(t) && equal(tagged2BitString(t)))
    ? PROCEED : FAILED;
}

int BitString::toBePickledV()
{
  return (OK);
}

void BitString::pickleV(MarshalerBuffer *mb, GenTraverser *gt)
{
  marshalNumber(mb, getWidth());
  for (int i = 0; i < getSize(); i++)
    marshalByte(mb, data[i]);
}

static
OZ_Term unmarshalBitString(MarshalerBuffer *mb, Builder*)
{
  int width = unmarshalNumber(mb);
  BitString *s = new BitString(width);
  int size = s->getSize();
  for (int i = 0; i < size; i++)
    s->getByte(i) = mb->get();
  return (makeTaggedExtension(s));
}

OZ_Term BitString::printV(int depth) {
  int w = getWidth();
  OZ_Term tup=OZ_tupleC("#",w);
  for (int i=0; i<w; i++) OZ_putArg(tup,i,oz_int(get(i)?1:0));
  return oz_pair2(oz_atom("<BitString \""),
                  oz_pair2(tup,oz_atom("\">")));
}

BitString* BitString::clone()
{
  BitString* s = new BitString();
  s->width = width;
  s->data  = cloneData();
  return s;
}

// -------------------------------------------------------------------
// BitString - suspendable marshaling
// -------------------------------------------------------------------

//
//
class DPMExtDesc : public GTAbstractEntity, public CppObjMemory {
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
static
void dpMarshalByteArrayCont(GenTraverser *gt, GTAbstractEntity *cont);

//
static inline
void marshalByteArray(ByteBuffer *mb, GenTraverser *gt,
                      DPMExtDesc *desc)
{
  // we should advance with marshaling:
  int size = desc->getCurrentSize();
  BYTE *data = desc->getData();

  //
  Assert(mb->availableSpace() > MNumberMaxSize);
  int availSpace = mb->availableSpace();
  // the current fragment's size must fit anyway:
  availSpace -= MNumberMaxSize;
  int ms = min(availSpace, size);

  //
  desc->decCurrentSize(ms);
  marshalNumber(mb, ms);        // current fragment;
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
static
void dpMarshalByteArrayCont(GenTraverser *gt, GTAbstractEntity *arg)
{
  ByteBuffer *bs = (ByteBuffer *) gt->getOpaque();
  Assert(arg->getType() == GT_ExtensionSusp);
  DPMExtDesc *desc = (DPMExtDesc *) arg;

  // we should advance with marshaling:
  Assert(bs->availableSpace() > DIFMaxSize + 2*MNumberMaxSize);
  marshalDIF(bs, DIF_EXT_CONT);
  marshalNumber(bs, desc->getExtID());

  //
  marshalByteArray(bs, gt, desc);
}

//
static
OZ_Term unmarshalByteArray(ByteBuffer *mb, DPMExtDesc *desc)
{
  int cWidth = unmarshalNumber(mb);

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
    return (UnmarshalEXT_Susp);
  }
}

//
class DPMBitStringDesc : public DPMExtDesc {
protected:
  int index;                    // into data;

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
    Assert(bs->typeV() == AtomBitString);
    BYTE *data = ((BitString *) bs)->getData();
    return (data+index);
  }
  virtual void setData(BYTE *dataIn) {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == AtomBitString);
    BYTE *data = ((BitString *) bs)->getData();
    Assert(dataIn >= data && dataIn < data + ((BitString *) bs)->getSize());
    index = dataIn - data;
  }
};

//
OZ_Boolean BitString::toBeMarshaledV()
{
  return (OK);
}

//
void
BitString::marshalSuspV(OZ_Term oet, ByteBuffer *bs, GenTraverser *gt)
{
  Assert(bs->availableSpace() > 2*MNumberMaxSize);

  //
  marshalNumber(bs, getWidth());

  //
  DPMBitStringDesc *desc = new DPMBitStringDesc(oet);
  int bss = getSize();
  desc->setTotalSize(bss);
  desc->setCurrentSize(bss);    // to be marshaled;
  //
  marshalByteArray(bs, gt, desc);
}

//
int BitString::minNeededSpace()
{
  return (2*MNumberMaxSize + 1);
}

//
static
OZ_Term suspUnmarshalBitString(ByteBuffer *mb, Builder*,
                               GTAbstractEntity* &bae)
{
  int width = unmarshalNumber(mb);

  //
  BitString *s = new BitString(width);
  OZ_Term bst = makeTaggedExtension(s);
  DPMBitStringDesc *desc = new DPMBitStringDesc(bst);
  desc->setTotalSize(s->getSize());
  bae = desc;                   // may be not necessary;

  //
  return (unmarshalByteArray(mb, desc));
}

//
static
OZ_Term unmarshalBitStringCont(ByteBuffer *mb, Builder*,
                               GTAbstractEntity* bae)
{
  Assert(bae->getType() == GT_ExtensionSusp);
  Assert(((DPMExtDesc *) bae)->getExtID() == OZ_E_BITSTRING);
  DPMBitStringDesc *desc = (DPMBitStringDesc *) bae;
  return (unmarshalByteArray(mb, desc));
}


// -------------------------------------------------------------------
// BitString INIT
// -------------------------------------------------------------------

//
void BitString_init()
{
  static int done = 0;
  if (!done) {
    done = 1;
    oz_registerExtension(OZ_E_BITSTRING, unmarshalBitString,
                         suspUnmarshalBitString, unmarshalBitStringCont);
  }
}

// -------------------------------------------------------------------
// BitString Builtins
// -------------------------------------------------------------------

OZ_BI_define(BIBitString_is,1,1)
{
  oz_declareNonvarIN(0,x);
  OZ_RETURN(oz_isBitString(x)?OZ_true():OZ_false());
} OZ_BI_end

OZ_BI_define(BIBitString_make,2,1)
{
  oz_declareIntIN(0,w);
  oz_declareNonvarIN(1,list);
  if (w<0) { oz_typeError(0,"Int>0"); }
  // wait for a fully determined list
  OZ_Term tail;
  if (!OZ_isList(list,&tail)) {
    if (tail==0) {
      oz_typeError(1,"list of ints");
    } else {
      oz_suspendOn(tail);
    }
  }
  // allocate the BitString
  BitString* bs = new BitString(w);
  // loop through the elements
  // check that each element is an integer in [0,w[
  // and setthe corresponding bit
  tail = list;
  while (!OZ_isNil(tail)) {
    OZ_Term elt = OZ_head(tail);
    int i;
    if (!OZ_isSmallInt(elt) || (i=OZ_intToC(elt))<0 || i>=w) {
      char buffer[100];
      sprintf(buffer,"list of small ints in range [0,%d)",w);
      oz_typeError(1,buffer);
    }
    else {
      bs->put(i,OK);
      tail = OZ_tail(tail);
    }
  }
  OZ_RETURN(makeTaggedExtension(bs));
} OZ_BI_end

OZ_BI_define(BIBitString_conj,2,1)
{
  oz_declareBitStringIN(0,b1);
  oz_declareBitStringIN(1,b2);
  if (b1->getWidth() != b2->getWidth())
    return oz_raise(E_ERROR,E_KERNEL,"BitString.conj",3,
                    oz_atom("widthMismatch"),
                    OZ_in(0),OZ_in(1));
  BitString*b3 = b1->clone();
  b3->conj(b2);
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIBitString_disj,2,1)
{
  oz_declareBitStringIN(0,b1);
  oz_declareBitStringIN(1,b2);
  if (b1->getWidth() != b2->getWidth())
    return oz_raise(E_ERROR,E_KERNEL,"BitString.disj",3,
                    oz_atom("widthMismatch"),
                    OZ_in(0),OZ_in(1));
  BitString*b3 = b1->clone();
  b3->disj(b2);
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIBitString_nega,1,1)
{
  oz_declareBitStringIN(0,b1);
  BitString*b3 = b1->clone();
  b3->nega();
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIBitString_get,2,1)
{
  oz_declareBitStringIN(0,b1);
  oz_declareIntIN(1,i);
  if (!b1->checkIndex(i))
    return oz_raise(E_SYSTEM,E_KERNEL,"BitString.get",3,
                    oz_atom("indexOutOfBound"),
                    OZ_in(0),OZ_in(1));
  OZ_RETURN((b1->get(i))?OZ_true():OZ_false());
} OZ_BI_end

OZ_BI_define(BIBitString_put,3,1)
{
  oz_declareBitStringIN(0,b1);
  oz_declareIntIN(1,i);
  oz_declareNonvarIN(2,on);
  if (!OZ_isTrue(on) && !OZ_isFalse(on)) {
    oz_typeError(2,"bool");
  }
  if (!b1->checkIndex(i))
    return oz_raise(E_SYSTEM,E_KERNEL,"BitString.put",3,
                    oz_atom("indexOutOfBound"),
                    OZ_in(0),OZ_in(1));
  BitString *b3 = b1->clone();
  b3->put(i,on==OZ_true());
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIBitString_width,1,1)
{
  oz_declareBitStringIN(0,b);
  OZ_RETURN_INT(b->getWidth());
} OZ_BI_end

OZ_BI_define(BIBitString_toList,1,1)
{
  oz_declareBitStringIN(0,b);
  int i = b->getWidth();
  OZ_Term list = oz_nil();
  while (i-- > 0)
    if (b->get(i)) list = oz_cons(oz_int(i),list);
  OZ_RETURN(list);
} OZ_BI_end

// -------------------------------------------------------------------
// ByteData
// -------------------------------------------------------------------

#include <ctype.h>

void ByteData::bytePrintStream(ostream& out) {
  int w = getWidth();
  for (int i=0; i<w; i++) {
    BYTE b = get(i);
    if (isalnum(b) || ispunct(b)) out << get(i);
    else if (b==' ')  out << " ";
    else if (b=='\n') out << "\\n";
    else if (b=='\t') out << "\\t";
    else if (b=='\r') out << "\\r";
    else {
      // output in octal notation
      int d1 = b / 64;
      b = b % 64;
      int d2 = b / 8;
      int d3 = b % 8;
      out << "\\" << d1 << d2 << d3;
    }
  }
}

Bool ByteData::equal(ByteData *s) {
  if (width != s->width) return 0;
  int size = width;
  for (int i=0; i<size; i++)
    if (data[i] != s->data[i]) return 0;
  return 1;
}

// -------------------------------------------------------------------
// ByteString
// -------------------------------------------------------------------

OZ_Term ByteString::typeV() {
  return (AtomByteString);
}

OZ_Return ByteString::eqV(OZ_Term t) {
  return (oz_isByteString(t) && equal(tagged2ByteString(t)))
    ? PROCEED : FAILED;
}

int ByteString::toBePickledV()
{
  return (OK);
}

void ByteString::pickleV(MarshalerBuffer *mb, GenTraverser *gt)
{
  int size = getWidth();
  marshalNumber(mb, size);
  for (int i = 0; i < size; i++)
    marshalByte(mb, data[i]);
}

static
OZ_Term unmarshalByteString(MarshalerBuffer *mb, Builder*)
{
  int width = unmarshalNumber(mb);
  ByteString *s = new ByteString(width);
  for (int i = 0; i < width; i++)
    s->getByte(i) = mb->get();
  return (makeTaggedExtension(s));
}

OZ_Term ByteString::printV(int depth) {
  ostrstream *out = new ostrstream;
  bytePrintStream(*out);
  int len = out->pcount();
  OZ_Term t = OZ_mkByteString(out->str(),len);
  delete out;
  return oz_pair2(oz_atom("<ByteString \""),
                  oz_pair2(t,oz_atom("\">")));
}

ByteString* ByteString::clone() {
  ByteString* s = new ByteString();
  s->width = width;
  s->data  = cloneData();
  return s;
}

// -------------------------------------------------------------------
// BitString - suspendable marshaling
// -------------------------------------------------------------------

//
class DPMByteStringDesc : public DPMExtDesc {
protected:
  int index;                    // into data;

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
    Assert(bs->typeV() == AtomByteString);
    BYTE *data = ((ByteString *) bs)->getData();
    return (data+index);
  }
  void setData(BYTE *dataIn) {
    OZ_Extension *bs = tagged2Extension(term);
    Assert(bs->typeV() == AtomByteString);
    BYTE *data = ((ByteString *) bs)->getData();
    Assert(dataIn >= data);
    // since dataIn points to the first free cell, it points just
    // behind the array when we're done:
    Assert(dataIn <= data + ((ByteString *) bs)->getSize());
    index = dataIn - data;
  }
};

//
OZ_Boolean ByteString::toBeMarshaledV()
{
  return (OK);
}

//
void
ByteString::marshalSuspV(OZ_Term oet, ByteBuffer *bs, GenTraverser *gt)
{
  Assert(bs->availableSpace() > 2*MNumberMaxSize);

  //
  marshalNumber(bs, getWidth());

  //
  DPMByteStringDesc *desc = new DPMByteStringDesc(oet);
  int bss = getSize();
  desc->setTotalSize(bss);
  desc->setCurrentSize(bss);    // to be marshaled;
  //
  marshalByteArray(bs, gt, desc);
}

//
int ByteString::minNeededSpace()
{
  return (2*MNumberMaxSize + 1);
}

//
static
OZ_Term suspUnmarshalByteString(ByteBuffer *mb, Builder*,
                                GTAbstractEntity* &bae)
{
  int width = unmarshalNumber(mb);

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
static
OZ_Term unmarshalByteStringCont(ByteBuffer *mb, Builder*,
                                GTAbstractEntity* bae)
{
  Assert(bae->getType() == GT_ExtensionSusp);
  Assert(((DPMExtDesc *) bae)->getExtID() == OZ_E_BYTESTRING);
  DPMByteStringDesc *desc = (DPMByteStringDesc *) bae;
  return (unmarshalByteArray(mb, desc));
}


// -------------------------------------------------------------------
// ByteString INIT
// -------------------------------------------------------------------

//
void ByteString_init()
{
  static int done = 0;
  if (! done) {
    done = 1;
    oz_registerExtension(OZ_E_BYTESTRING, unmarshalByteString,
                         suspUnmarshalByteString, unmarshalByteStringCont);
  }
}


// -------------------------------------------------------------------
// ByteString Builtins
// -------------------------------------------------------------------

OZ_BI_define(BIByteString_is,1,1)
{
  oz_declareNonvarIN(0,x);
  OZ_RETURN(oz_isByteString(x)?OZ_true():OZ_false());
} OZ_BI_end

OZ_BI_define(BIByteString_make,1,1)
{
  oz_declareNonvarIN(0,list);
  // wait for a fully determined string
  OZ_Term tail;
  if (!OZ_isList(list,&tail)) {
    if (tail==0) {
      oz_typeError(1,"list of chars");
    } else {
      oz_suspendOn(tail);
    }
  }
  int w = OZ_length(list);
  // allocate the ByteString
  ByteString* bs = new ByteString(w);
  // loop through the elements
  for(int i=0;!OZ_isNil(list);i++,list=OZ_tail(list)) {
    OZ_Term elt = OZ_head(list);
    int c;
    // verify that each is ok as a byte
    if (!OZ_isSmallInt(elt) || (c=OZ_intToC(elt))<0 || c>255) {
      oz_typeError(0,"list of bytes");
    }
    // initialize the corresponding byte of the ByteString
    bs->put(i,(BYTE)c);
  }
  OZ_RETURN(makeTaggedExtension(bs));
} OZ_BI_end

OZ_BI_define(BIByteString_get,2,1)
{
  oz_declareByteStringIN(0,b1);
  oz_declareIntIN(1,i);
  if (!b1->checkIndex(i))
    return oz_raise(E_SYSTEM,E_KERNEL,"ByteString.get",3,
                    oz_atom("indexOutOfBound"),
                    OZ_in(0),OZ_in(1));
  OZ_RETURN_INT(b1->get(i));
} OZ_BI_end

OZ_BI_define(BIByteString_append,2,1)
{
  oz_declareByteStringIN(0,b1);
  oz_declareByteStringIN(1,b2);
  int w = b1->getWidth() + b2->getWidth();
  ByteString *b3 = new ByteString(w);
  b3->copy(b1,0);
  b3->copy(b2,b1->getWidth());
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIByteString_slice,3,1)
{
  oz_declareByteStringIN(0,b1);
  oz_declareIntIN(1,from);
  oz_declareIntIN(2,to);
  int w = b1->getWidth();
  if (from<0 || to<0 || from>w || to>w || from>to)
    return oz_raise(E_SYSTEM,E_KERNEL,"ByteString.slice",4,
                    oz_atom("indexOutOfBound"),
                    OZ_in(0),OZ_in(1),OZ_in(2));
  ByteString *b3 = new ByteString(to-from);
  b3->slice(b1,from,to);
  OZ_RETURN(makeTaggedExtension(b3));
} OZ_BI_end

OZ_BI_define(BIByteString_width,1,1)
{
  oz_declareByteStringIN(0,b);
  OZ_RETURN_INT(b->getWidth());
} OZ_BI_end

OZ_BI_define(BIByteString_toString,1,1)
{
  oz_declareByteStringIN(0,b1);
  int i = b1->getWidth();
  OZ_Term list = oz_nil();
  while (i-- > 0) list = oz_cons(oz_int(b1->get(i)),list);
  OZ_RETURN(list);
} OZ_BI_end

OZ_BI_define(BIByteString_toStringWithTail,2,1)
{
  oz_declareByteStringIN(0,b1);
  OZ_declareTerm(1,list);
  int i = b1->getWidth();
  while (i-- > 0) list = oz_cons(oz_int(b1->get(i)),list);
  OZ_RETURN(list);
} OZ_BI_end

OZ_Term OZ_mkByteString(const char*s,int n) {
  ByteString* bs = new ByteString(n);
  memcpy((void*)bs->getData(),(const void*)s,n);
  return makeTaggedExtension(bs);
}

inline unsigned char* find_char(unsigned char*s,int c,int n)
{
  while (n-- > 0) {
    if (*s==c) return s;
    else s++;
  }
  return NULL;
}

OZ_BI_define(BIByteString_strchr,3,1)
{
  oz_declareByteStringIN(0,bs);
  OZ_declareInt(1,OFF);
  OZ_declareInt(2,c);
  int n = bs->getWidth();
  if (OFF<0 || OFF>=n)
    return oz_raise(E_SYSTEM,E_KERNEL,"BitString.strchr",3,
                    oz_atom("indexOutOfBound"),
                    OZ_in(0),OZ_in(1));
  n -= OFF;
  if (c<0 || c>255) oz_typeError(2,"char");
  unsigned char* s1 = (unsigned char*) bs->getData();
  unsigned char* s2 = find_char(s1+OFF,c,n);
  if (s2==NULL) OZ_RETURN(OZ_false());
  else OZ_RETURN_INT(s2-s1);
} OZ_BI_end

OZ_BI_define(BIByteString_cmp,2,1)
{
  oz_declareByteStringIN(0,bs1);
  oz_declareByteStringIN(1,bs2);
  int n1 = bs1->getWidth();
  int n2 = bs2->getWidth();
  char* s1 = (char*) bs1->getData();
  char* s2 = (char*) bs2->getData();
  for (int i=(n1<n2)?n1:n2;i!=0;i--,s1++,s2++) {
    char c1 = *s1;
    char c2 = *s2;
    if (c1 < c2) OZ_RETURN_INT(-1);
    if (c1 > c2) OZ_RETURN_INT( 1);
  }
  OZ_RETURN_INT((n1<n2)?-1:((n1>n2)?1:0));
} OZ_BI_end
