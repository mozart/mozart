class RIProfile : public OZ_CtProfile {
private:
  ri_float _l, _u;

public:
  RIProfile(void) {}
  virtual void init(OZ_Ct * c) {
    RI * ri = (RI *) c;
    _l = ri->_l;
    _u = ri->_u;
  }
};
