class RIExpect : public OZ_Expect {
public:
  OZ_expect_t expectRIVarMin(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMin()); 
  }
  OZ_expect_t expectRIVarMax(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMax()); 
  }
  OZ_expect_t expectRIVarMinMax(OZ_Term t) { 
    return expectGenCtVar(t, ri_definition, 
			  RIWakeUp::wakeupMinMax()); 
  }
  ...
};
