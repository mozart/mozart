class TwiceProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _x, _z;
public:
  TwiceProp(OZ_Term a, OZ_Term b) 
   : _x(a), _z(b) {}
  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { 
    return sizeof(TwiceProp); 
  }
  virtual void updateHeapRefs(OZ_Boolean) { 
    OZ_updateHeapTerm(_x); 
    OZ_updateHeapTerm(_z); 
  } 
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, 
		   OZ_cons(_z,
			   OZ_nil()));
  }
  virtual OZ_PropagatorProfile *getProfile(void) const { 
    return &profile;
  }
};

OZ_PropagatorProfile TwiceProp::profile;
