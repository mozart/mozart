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
#include "bits.hh"

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
    return ((int *) oz_heapMalloc(n));
  }
public:
  virtual
  int getIdV() { return OZ_E_BITARRAY; }

  virtual
  OZ_Term printV(int depth = 10) { return oz_atom("<BitArray>"); }

  virtual
  OZ_Term typeV() { return (AtomBitArray); }

  virtual
  OZ_Term printLongV(int depth = 10, int offset = 0) {
    return 
      OZ_mkTupleC("#",4,
		  oz_atom("bit array: "), oz_int(upperBound - lowerBound - 1),
		  oz_atom(" bits at "),   oz_int((int)this));
  }

  virtual OZ_Return getFeatureV(OZ_Term,OZ_Term&);
  virtual OZ_Return putFeatureV(OZ_Term,OZ_Term );

  virtual
  OZ_Extension *gCollectV(void);
  OZ_Extension *sCloneV(void);
  virtual void sCloneRecurseV(void) {}
  virtual void gCollectRecurseV(void) {}

  BitArray operator=(const BitArray &);  // fake for compiler
  BitArray(int lower, int upper): OZ_Extension() {
    Assert(lower <= upper);
    lowerBound = lower;
    upperBound = upper;
    int size = getSize();
    array = allocate(size);
    for (int i = size; i--; )
      array[i] = 0;
  }
  BitArray(const BitArray *b): OZ_Extension() {
    lowerBound = b->lowerBound;
    upperBound = b->upperBound;
    int size = getSize();
    array = allocate(size);
    memcpy(array, b->array, size * sizeof(array[0]));
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
  void bor(const BitArray *);
  void band(const BitArray *);
  Bool disjoint(const BitArray *);
  Bool subsumes(BitArray *);
  int card(void);
  void nimpl(const BitArray *);
  TaggedRef toList(void);
  TaggedRef complementToList(void);
};

inline
Bool oz_isBitArray(TaggedRef term) {
  return oz_isExtension(term) &&
    tagged2Extension(term)->getIdV() == OZ_E_BITARRAY;
}

inline
BitArray *tagged2BitArray(TaggedRef term) {
  Assert(oz_isBitArray(term));
  return (BitArray *) tagged2Extension(term);
}

/*===================================================================
 * Bit Arrays
 *=================================================================== */

OZ_Extension *BitArray::gCollectV(void) {
  BitArray *ret = new BitArray(this);
  return ret;
}

OZ_Extension *BitArray::sCloneV(void) {
  BitArray *ret = new BitArray(this);
  return ret;
}

inline
void BitArray::set(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  array[relative / BITS_PER_INT] |= 1 << (relative % BITS_PER_INT);
}

inline
void BitArray::clear(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  array[relative / BITS_PER_INT] &= ~(1 << (relative % BITS_PER_INT));
}

inline
Bool BitArray::test(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  return array[relative / BITS_PER_INT] & 1 << (relative % BITS_PER_INT);
}

inline
void BitArray::bor(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  for (int i = getSize(); i--; )
    array[i] |= b->array[i];
}

inline
void BitArray::band(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  for (int i = getSize(); i--; )
    array[i] &= b->array[i];
}

inline
void BitArray::nimpl(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  for (int i = getSize(); i--; )
    array[i] &= ~b->array[i];
}

inline
Bool BitArray::disjoint(const BitArray *b) {
  Assert(lowerBound == b->lowerBound && upperBound == b->upperBound);
  for (int i = getSize(); i--; )
    if ((array[i] & b->array[i]) != 0)
      return NO;
  return OK;
}

inline
Bool BitArray::subsumes(BitArray *b) {
  const int l = b->lowerBound;
  const int u = b->upperBound;
  if ((lowerBound > l) || (upperBound < u))
    return NO;
  for (int i = l; i <= u; i++) 
    if (b->test(i) && !test(i))
      return NO;
  return OK;
}

inline
int BitArray::card(void) {
  return get_num_of_bits(getSize(),array);
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
	list = oz_cons(makeTaggedSmallInt(offset + j),list);
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
	list = oz_cons(makeTaggedSmallInt(offset + j),list);
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
  if (!oz_isBitArray(_VAR)) {                   \
    oz_typeError(ARG,"BitArray");               \
  } else {                                      \
    VAR = tagged2BitArray(_VAR);                \
  }                                             \
}

OZ_BI_define(BIbitArray_new,2,1)
{
  oz_declareIntIN(0,l);
  oz_declareIntIN(1,h);
  if (l <= h)
    OZ_RETURN(makeTaggedExtension(new BitArray(l, h)));
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

OZ_Return BitArray::getFeatureV(OZ_Term f,OZ_Term& v)
{
  if (!OZ_isInt(f)) { oz_typeError(1,"int"); }
  int i = OZ_intToC(f);
  if (checkBounds(i)) {
    v = test(i)? OZ_true(): OZ_false();
    return PROCEED;
  } else {
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,
		    makeTaggedExtension(this),f);
  }
}

OZ_Return BitArray::putFeatureV(OZ_Term f,OZ_Term v)
{
  if (!OZ_isInt(f)) { oz_typeError(1,"int"); }
  int i = OZ_intToC(f);
  if (!checkBounds(i)) {
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,
		    makeTaggedExtension(this),f);
  }
  if (OZ_isVariable(v)) { OZ_suspendOn(v); }
  v = oz_deref(v);
  if (v==OZ_true()) set(i);
  else if (v==OZ_false()) clear(i);
  else { oz_typeError(2,"bool"); }
  return PROCEED;
}

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
  OZ_RETURN(makeTaggedExtension(new BitArray(b)));
} OZ_BI_end

OZ_BI_define(BIbitArray_or,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->bor(b2);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.binop",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_and,2,0)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  if (b1->checkBounds(b2)) {
    b1->band(b2);
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


OZ_BI_define(BIbitArray_subsumes,2,1)
{
  oz_declareBitArrayIN(0,b1);
  oz_declareBitArrayIN(1,b2);
  OZ_RETURN(oz_bool(b1->subsumes(b2)));
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

OZ_BI_define(BIbitArray_fromList,1,1)
{
  TaggedRef is_t  = OZ_in(0);
  TaggedRef is_td = oz_deref(is_t);

  int n  = 0;
  int mi = OzMaxInt;
  int ma = OzMinInt;

  BitArray * b; 

  while (oz_isLTuple(is_td)) {
    LTuple * is = tagged2LTuple(is_td);
    TaggedRef i_t = oz_deref(is->getHead());
    Assert(!oz_isRef(i_t));
    if (oz_isVarOrRef(i_t)) {
      oz_suspendOn(is->getHead());
    }
    if (!oz_isSmallInt(i_t))
      goto type_error;
    const int i = tagged2SmallInt(i_t);
    if (i<mi)
      mi = i;
    if (i>ma)
      ma = i;
    n++;
    is_t  = is->getTail();
    is_td = oz_deref(is_t);
  }

  Assert(!oz_isRef(is_td));
  if (oz_isVarOrRef(is_td)) {
    oz_suspendOn(is_t);
  }

  if (!oz_isNil(is_td) || (n==0))
    goto type_error;

  b = new BitArray(mi,ma);

  is_t = OZ_in(0);

  while (n--) {
    LTuple * is = tagged2LTuple(oz_deref(is_t));
    const int i = tagged2SmallInt(oz_deref(is->getHead()));
    b->set(i);
    is_t = is->getTail();
  }
  
  OZ_RETURN(makeTaggedExtension(b));

 type_error:
  oz_typeError(0,"Non-empty list of small integers");
  
} OZ_BI_end

