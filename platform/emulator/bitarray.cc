/*
 *  Authors:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *
 *  Contributors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
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

#include "base.hh"
#include "builtins.hh"

/*===================================================================
 * BitArray
 *=================================================================== */

#define BITS_PER_INT (sizeof(int) * 8)

class BitArray: public OZ_Extension {
private:
  int lowerBound, upperBound;
  int *array;
  int getSize() {
    return (upperBound - lowerBound) / BITS_PER_INT + 1;
  }
  int *allocate(int size) {
    size_t n = size * sizeof(int);
    COUNT1(sizeBitArrays,n);
    return (int *) alignedMalloc(n, sizeof(double));
  }
public:
  virtual
  int getIdV() { return OZ_E_BITARRAY; }

  virtual
  OZ_Term printV(int depth = 10) { return oz_atom("<BitArray>"); }

  virtual
  OZ_Term typeV() { return oz_atom("bitArray"); }

  virtual
  OZ_Term printLongV(int depth = 10, int offset = 0) {
    return
      OZ_mkTupleC("#",4,
                  oz_atom("bit array: "), oz_int(upperBound - lowerBound - 1),
                  oz_atom(" bits at "),   oz_int((int)this));
  }

  virtual
  OZ_Extension *gcV(void);
  BitArray operator=(const BitArray &);  // fake for compiler
  BitArray(int lower, int upper): OZ_Extension() {
    Assert(lower <= upper);
    lowerBound = lower;
    upperBound = upper;
    int size = getSize();
    array = allocate(size);
    for (int i = 0; i < size; i++)
      array[i] = 0;
    COUNT1(sizeBitArrays, sizeof(BitArray));
  }
  BitArray(const BitArray *b): OZ_Extension() {
    lowerBound = b->lowerBound;
    upperBound = b->upperBound;
    int size = getSize();
    array = allocate(size);
    memcpy(array, b->array, size * sizeof(array[0]));
    COUNT1(sizeBitArrays, sizeof(BitArray));
  }
  Bool checkBounds(int i) {
    return lowerBound <= i && i <= upperBound;
  }
  Bool checkBounds(const BitArray *b) {
    return lowerBound == b->lowerBound && upperBound == b->upperBound;
  }
  void set(int);
  void clear(int);
  Bool test(int);
  int getLower(void) { return lowerBound; }
  int getUpper(void) { return upperBound; }
  void or(const BitArray *);
  void and(const BitArray *);
  Bool disjoint(const BitArray *);
  int card();
  void nimpl(const BitArray *);
  TaggedRef toList(void);
  TaggedRef complementToList(void);
};

inline
Bool oz_isBitArray(TaggedRef term)
{
  return oz_isExtension(term) &&
    oz_tagged2Extension(term)->getIdV() == OZ_E_BITARRAY;
}

inline
BitArray *tagged2BitArray(TaggedRef term)
{
  Assert(oz_isBitArray(term));
  return (BitArray *) oz_tagged2Extension(term);
}

/*===================================================================
 * Bit Arrays
 *=================================================================== */

OZ_Extension *BitArray::gcV(void) {
  BitArray *ret = new BitArray(this);
  return ret;
}

void BitArray::set(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  array[relative / BITS_PER_INT] |= 1 << (relative % BITS_PER_INT);
}

void BitArray::clear(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  array[relative / BITS_PER_INT] &= ~(1 << (relative % BITS_PER_INT));
}

Bool BitArray::test(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  return array[relative / BITS_PER_INT] & 1 << (relative % BITS_PER_INT);
}

void BitArray::or(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  int size = getSize();
  for (int i = 0; i < size; i++)
    array[i] |= b->array[i];
}

void BitArray::and(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  int size = getSize();
  for (int i = 0; i < size; i++)
    array[i] &= b->array[i];
}

void BitArray::nimpl(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  int size = getSize();
  for (int i = 0; i < size; i++)
    array[i] &= ~b->array[i];
}

Bool BitArray::disjoint(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  int size = getSize();
  for (int i = 0; i < size; i++) {
    if ((array[i] & b->array[i]) != 0)
      return NO;
  }
  return OK;
}

int BitArray::card() {
  int ret = 0;
  int size = getSize();
  for (int i = 0; i < size; i++) {
    unsigned int aux = array[i];
    while(aux) {
      if (aux&1) ret++;
      aux = aux>>1;
    }
  }
  return ret;
}



TaggedRef BitArray::toList(void) {
  TaggedRef list = AtomNil;
  int offset =
    ((upperBound - lowerBound) / BITS_PER_INT) * BITS_PER_INT + lowerBound;
  int i, j, word;
  for (i = getSize() - 1; i >= 0; i--) {
    word = array[i];
    for (j = BITS_PER_INT - 1; j >= 0; j--)
      if (word & (1 << j))
        list = OZ_cons(OZ_int(offset + j),list);
    offset -= BITS_PER_INT;
  }
  return list;
}

TaggedRef BitArray::complementToList(void) {
  TaggedRef list = AtomNil;
  int offset =
    ((upperBound - lowerBound) / BITS_PER_INT) * BITS_PER_INT + lowerBound;
  int i, j, word;
  for (i = getSize() - 1; i >= 0; i--) {
    word = array[i];
    for (j = BITS_PER_INT - 1; j >= 0; j--)
      if (!(word & (1 << j)))
        list = OZ_cons(OZ_int(offset + j),list);
    offset -= BITS_PER_INT;
  }
  return list;
}

/* -----------------------------------------------------------------
   Bit Arrays
   ----------------------------------------------------------------- */

#define oz_declareBitArrayIN(ARG,VAR)           \
BitArray *VAR;                                  \
{                                               \
  oz_declareNonvarIN(ARG,_VAR);                 \
  if (!oz_isBitArray(oz_deref(_VAR))) {         \
    oz_typeError(ARG,"BitArray");               \
  } else {                                      \
    VAR = tagged2BitArray(oz_deref(_VAR));      \
  }                                             \
}

OZ_BI_define(BIbitArray_new,2,1)
{
  oz_declareIntIN(0,l);
  oz_declareIntIN(1,h);
  if (l <= h)
    OZ_RETURN(oz_makeTaggedExtension(new BitArray(l, h)));
  else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.new",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_is,1,1)
{
  oz_declareNonvarIN(0,x);
  OZ_RETURN(oz_isBitArray(oz_deref(x))? OZ_true(): OZ_false());
} OZ_BI_end

OZ_BI_define(BIbitArray_set,2,0)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i)) {
    b->set(i);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_clear,2,0)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i)) {
    b->clear(i);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_test,2,1)
{
  oz_declareBitArrayIN(0,b);
  oz_declareIntIN(1,i);
  if (b->checkBounds(i))
    OZ_RETURN(b->test(i)? OZ_true(): OZ_false());
  else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_low,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN_INT(b->getLower());
} OZ_BI_end

OZ_BI_define(BIbitArray_high,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN_INT(b->getUpper());
} OZ_BI_end

OZ_BI_define(BIbitArray_clone,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(oz_makeTaggedExtension(new BitArray(b)));
} OZ_BI_end

OZ_BI_define(BIbitArray_or,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->or(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_and,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->and(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_nimpl,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->nimpl(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_disjoint,2,1)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    OZ_RETURN(oz_bool(b1->disjoint(b2)));
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end


OZ_BI_define(BIbitArray_card,1,1)
{
  oz_declareBitArrayIN(0,b1);
  OZ_RETURN_INT(b1->card());
} OZ_BI_end



OZ_BI_define(BIbitArray_toList,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(b->toList());
} OZ_BI_end

OZ_BI_define(BIbitArray_complementToList,1,1)
{
  oz_declareBitArrayIN(0,b);
  OZ_RETURN(b->complementToList());
} OZ_BI_end
