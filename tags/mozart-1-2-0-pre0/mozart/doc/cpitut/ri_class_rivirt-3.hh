  ...
  virtual OZ_Boolean isWeakerThan(OZ_Ct * r) {
    RI * ri = (RI *) r;
    return (ri->getWidth() < getWidth());
  }

  virtual OZ_Ct * unify(OZ_Ct * r) {
    RI * x = this, * y = (RI *) r;
    static RI z; 

    z._l = max(x->_l, y->_l);
    z._u = min(x->_u, y->_u);

    return &z;
  }

  virtual OZ_Boolean unify(OZ_Term rvt) {
    if (isValidValue(rvt)) {
      double rv = OZ_floatToC(rvt);
      
      return (_l <= rv) && (rv <= _u);
    } 
    return 0;
  }
  ...
