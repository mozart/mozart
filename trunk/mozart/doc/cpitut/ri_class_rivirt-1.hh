  ...
  virtual char * toString(int);
  virtual size_t sizeOf(void);

  virtual OZ_Ct * copy(void) {
    RI * ri = new (sizeof(ri_float)) RI(_l, _u);
    return ri;
  }

  virtual OZ_Boolean isValue(void) {
    return (getWidth() < ri_precision);
  }

  virtual OZ_Term toValue(void) {
    double val = (_u + _l) / 2.0;
    return OZ_float(val);
  }

  virtual OZ_Boolean isValid(void) {
    return _l <= _u;
  }
  ...
