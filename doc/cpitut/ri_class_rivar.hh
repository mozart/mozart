class RIVar : public OZ_CtVar {
private:
  RI * _ri_ref, _ri;

  RIProfile _rip;

protected:
  virtual void ctSetValue(OZ_Term t) {
    _ri.init(t);
    _ri_ref = &_ri;
  }
  virtual OZ_Ct * ctRefConstraint(OZ_Ct * c) {
    return _ri_ref = (RI *) c;
  }
  virtual OZ_Ct * ctSaveConstraint(OZ_Ct * c) {
    _ri = *(RI *) c;
    return c;
  }
  virtual void ctRestoreConstraint(void) {
    *_ri_ref = _ri;
  }
  virtual void ctSetConstraintProfile(void) {
    _rip = *_ri_ref->getProfile();
  }
  virtual OZ_CtProfile * ctGetConstraintProfile(void) {
    return &_rip;
  }
  virtual OZ_Ct * ctGetConstraint(void) {
    return _ri_ref;
  }

public:
  RIVar(void) : OZ_CtVar() { }
  RIVar(OZ_Term t) : OZ_CtVar() { read(t); }

  virtual OZ_Boolean isTouched(void) const  {
    return _ri_ref->isTouched(_rip);
  }
  RI &operator * (void) { return *_ri_ref; }
  RI * operator -> (void) { return _ri_ref; }
};
