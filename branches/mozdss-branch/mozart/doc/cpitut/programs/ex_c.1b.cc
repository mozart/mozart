private:
  OZ_expect_t _expectIntVarAny(OZ_Term t) {
    return expectIntVar(t, fd_prop_any);
  }
public:
  OZ_expect_t expectVectorIntVarAny(OZ_Term t) {
    return expectVector(t, 
              (OZ_ExpectMeth) &_expectIntVarAny);
  }
  OZ_expect_t expectVectorIntVarSingl(OZ_Term t) {
    return expectVector(t, 
	      (OZ_ExpectMeth) &expectIntVarSingl);
  }
