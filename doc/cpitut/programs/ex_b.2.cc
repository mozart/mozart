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
  virtual void gCollect(void) { 
    OZ_gCollectTerm(_x); 
    OZ_gCollectTerm(_z); 
  } 
  virtual void sClone(void) { 
    OZ_sCloneTerm(_x); 
    OZ_sCloneTerm(_z); 
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
