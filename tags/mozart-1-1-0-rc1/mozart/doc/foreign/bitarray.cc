class BitArray: public OZ_Extension {

private:
  int lowerBound, upperBound;
  int *array;
public:
  virtual
  int getIdV() { return OZ_E_BITARRAY; }

  virtual
  OZ_Term printV(int depth = 10) { return typeV(); }

  virtual
  OZ_Term typeV() { return OZ_atom("bitArray"); }

  virtual
  OZ_Extension *gCollectV(void) { return new BitArray(*this); }
  BitArray(int lower, int upper): OZ_Extension() {
    ...
  }
  BitArray(const BitArray &amp;b): OZ_Extension() {
    ...
  }

  void set(int);
  void clear(int);
  Bool test(int);
  ...
};

inline
Bool OZ_isBitArray(TaggedRef term)
{
  return OZ_isExtension(term) &amp;&amp;
    OZ_getExtension(term)->getIdV() == OZ_E_BITARRAY;
}

inline
BitArray *tagged2BitArray(TaggedRef term)
{
  Assert(OZ_isBitArray(term));
  return (BitArray *) OZ_getExtension(term);
}


#define OZ_declareBitArray(ARG,VAR)           \
BitArray *VAR;                                  \
{                                               \
  OZ_declareNonvar(ARG,_VAR);                 \
  if (!OZ_isBitArray(OZ_deref(_VAR))) {         \
    OZ_typeError(ARG,"BitArray");               \
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

OZ_BI_define(BIbitArray_set,2,0)
{
  OZ_declareBitArray(0,b);
  OZ_declareInt(1,i);
  if (b->checkBounds(i)) {
    b->set(i);
    return PROCEED;
  } else
    return oz_raise(E_ERROR,E_KERNEL,"BitArray.index",2,OZ_in(0),OZ_in(1));
} OZ_BI_end
