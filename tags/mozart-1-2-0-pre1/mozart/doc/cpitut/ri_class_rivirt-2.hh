  ...
  virtual RIProfile * getProfile(void) {
    static RIProfile rip;
    rip.init(this);
    return &rip;
  }

  virtual 
  OZ_CtWakeUp getWakeUpDescriptor(OZ_CtProfile * p) {
    OZ_CtWakeUp d;
    d.init();
    
    RIProfile * rip = (RIProfile *) p;
    if (_l > rip->_l) d.setWakeUp(0);
    if (_u < rip->_u) d.setWakeUp(1);

    return d;
  }
  ...

