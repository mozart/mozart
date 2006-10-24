class RIDefinition : public OZ_CtDefinition {
private:
  static int _kind; 

public: 
  virtual int getKind(void) { return _kind; }
  virtual char * getName(void) { return "real interval"; }
  virtual int getNoOfWakeUpLists(void) { return 2; }
  virtual char ** getNamesOfWakeUpLists(void) {
    static char * names[2] = {"lower", "upper"};
    return names;
  }
  virtual OZ_Ct * leastConstraint(void) {
    return RI::leastConstraint();
  }
  virtual OZ_Boolean isValidValue(OZ_Term f) {
    return RI::isValidValue(f);
  }				  
};

int RIDefinition::_kind = OZ_getUniqueId();
