class RI : public OZ_Ct {
private:
  ri_float _l, _u;

public:
  RI(void) {}
  RI(ri_float l, ri_float u) : _l(l), _u(u) {}

  void init(OZ_Term t) { _l = _u = OZ_floatToC(t);  }

  ri_float getWidth(void) { return _u - _l; }

  static OZ_Ct * leastConstraint(void) {
    static RI ri(RI_FLOAT_MIN, RI_FLOAT_MAX);
    return &ri;
  }

  static OZ_Boolean isValidValue(OZ_Term f) {
    return OZ_isFloat(f);
  }

  OZ_Boolean isTouched(RIProfile rip) {
    return (rip._l < _l) || (rip._u > _u);
  }
  ...
