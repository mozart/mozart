class ElementProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _n, _v, * _d;
  int _d_size;
public:
  ElementProp(OZ_Term n, OZ_Term d, OZ_Term v) 
    : _n(n), _v(v), _d_size (OZ_vectorSize(d))
  {
    _d =  OZ_hallocOzTerms(_d_size);
    OZ_getOzTermVector(d, _d);
  }

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { 
    return sizeof(ElementProp); 
  }
  virtual OZ_PropagatorProfile *getProfile(void) const { 
    return &profile; 
  }
  virtual OZ_Term getParameters(void) const;
  virtual void gCollect(void);
  virtual void sClone(void);
};

OZ_PropagatorProfile ElementProp::profile;
