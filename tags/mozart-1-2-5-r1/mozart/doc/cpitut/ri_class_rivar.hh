class RIVar : public OZ_CtVar {
private:

  RI * _ref;
  RI _copy, _encap;

  RIProfile _rip;

protected:

  virtual void ctSetValue(OZ_Term t)
  {
    _copy.init(t);
    _ref = &_copy;
  }

  virtual OZ_Ct * ctRefConstraint(OZ_Ct * c)
  {
    return _ref = (RI *) c;
  }

  virtual OZ_Ct * ctSaveConstraint(OZ_Ct * c)
  {
    _copy = *(RI *) c;
    return &_copy;
  }

  virtual OZ_Ct * ctSaveEncapConstraint(OZ_Ct * c)
  {
    _encap = *(RI *) c;
    return &_encap;
  }

  virtual void ctRestoreConstraint(void)
  {
    *_ref = _copy;
  }

  virtual void ctSetConstraintProfile(void)
  {
    _rip = *_ref->getProfile();
  }

  virtual OZ_CtProfile * ctGetConstraintProfile(void)
  {
    return &_rip;
  }

  virtual OZ_Ct * ctGetConstraint(void)
  {
    return _ref;
  }

public:

  RIVar(void) : OZ_CtVar() { }

  RIVar(OZ_Term t) : OZ_CtVar() { read(t); }

  virtual OZ_Boolean isTouched(void) const
  {
    return _ref->isTouched(_rip);
  }

  RI &operator * (void) { return *_ref; }
  RI * operator -> (void) { return _ref; }

};
