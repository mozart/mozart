class AddProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _x, _y, _z;
public:
  AddProp(OZ_Term a, OZ_Term b, OZ_Term c) 
   : _x(a), _y(b), _z(c) {}
  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { 
    return sizeof(AddProp); 
  }
  virtual void updateHeapRefs(OZ_Boolean) { 
    OZ_updateHeapTerm(_x); 
    OZ_updateHeapTerm(_y); 
    OZ_updateHeapTerm(_z);
  } 
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, 
		   OZ_cons(_y, 
			   OZ_cons(_z, 
				   OZ_nil())));
  }
  virtual OZ_PropagatorProfile *getProfile(void) const { 
    return &profile; 
  }
};

OZ_PropagatorProfile AddProp::profile;

