class ExtendedExpect : public OZ_Expect {
public:
  OZ_expect_t expectIntVarSingl(OZ_Term t) {
    return expectIntVar(t, fd_prop_singl);
  }
