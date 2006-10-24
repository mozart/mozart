#define BITS_PER_INT (sizeof(int) * 8)

class BitArray: public OZ_Extension {
private:
  int lowerBound, upperBound;
  int *array;
  ...
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
                  OZ_atom("bit array: "), OZ_int(upperBound - lowerBound - 1),
                  OZ_atom(" bits at "),   OZ_int((int)this));
  }

  virtual OZ_Return getFeatureV(OZ_Term,OZ_Term&);
  virtual OZ_Return putFeatureV(OZ_Term,OZ_Term );

  virtual OZ_Extension *gCollectV(void);
  virtual OZ_Extension *sCloneV(void);
  virtual void sCloneRecurseV(void) {}
  virtual void gCollectRecurseV(void) {}

  ...
  BitArray(int lower, int upper): OZ_Extension() {
    ...
  }
  BitArray(const BitArray *b): OZ_Extension() {
    ...
  }
  Bool checkBounds(int i) {
    return lowerBound <= i && i <= upperBound;
  }
  ...
  void set(int);
  ...
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

OZ_Extension *BitArray::gCollectV(void) {
  BitArray *ret = new BitArray(this);
  return ret;
}

OZ_Extension *BitArray::sCloneV(void) {
  BitArray *ret = new BitArray(this);
  return ret;
}

void BitArray::set(int i) {
  Assert(checkBounds(i));
  int relative = i - lowerBound;
  array[relative / BITS_PER_INT] |= 1 << (relative % BITS_PER_INT);
}

#define oz_declareBitArray(ARG,VAR)             \
BitArray *VAR;                                  \
{                                               \
  OZ_declareDetTerm(ARG,_VAR);                  \
  if (!OZ_isBitArray(oz_deref(_VAR))) {         \
    return OZ_typeError(ARG,"BitArray");        \
  } else {                                      \
    VAR = tagged2BitArray(OZ_deref(_VAR));      \
  }                                             \
}

OZ_BI_define(BIbitArray_new,2,1)
{
  OZ_declareInt(0,l);
  OZ_declareInt(1,h);
  if (l <= h)
    OZ_RETURN(OZ_extension(new BitArray(l, h)));
  else
    return OZ_raise(E_ERROR,E_KERNEL,"BitArray.new",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_BI_define(BIbitArray_is,1,1)
{
  OZ_declareDetTerm(0,x);
  OZ_RETURN_BOOL(oz_isBitArray(oz_deref(x)));
} OZ_BI_end

OZ_BI_define(BIbitArray_set,2,0)
{
  OZ_declareBitArray(0,b);
  OZ_declareInt(1,i);
  if (b->checkBounds(i)) {
    b->set(i);
    return PROCEED;
  } else
    return OZ_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end

OZ_Return BitArray::getFeatureV(OZ_Term f,OZ_Term& v)
{
  if (!OZ_isInt(f)) { oz_typeError(1,"int"); }
  int i = OZ_intToC(f);
  if (checkBounds(i)) {
    v = test(i)? OZ_true(): OZ_false();
    return PROCEED;
  } else {
    return OZ_raise(E_ERROR,E_KERNEL,"BitArray.index",2,
                    OZ_extension(this),f);
  }
}

OZ_Return BitArray::putFeatureV(OZ_Term f,OZ_Term v)
{
  if (!OZ_isInt(f)) { oz_typeError(1,"int"); }
  int i = OZ_intToC(f);
  if (!checkBounds(i)) {
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,
                    OZ_extension(this),f);
  }
  if (OZ_isVariable(v)) { OZ_suspendOn(v); }
  v = oz_deref(v);
  if (v==OZ_true()) set(i);
  else if (v==OZ_false()) clear(i);
  else { return OZ_typeError(2,"bool"); }
  return PROCEED;
}
