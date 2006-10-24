class Iterator_OZ_FDIntVar {
private:
  int _l_size;
  OZ_FDIntVar * _l;
public:
  Iterator_OZ_FDIntVar(int s, OZ_FDIntVar * l)
    : _l_size(s), _l(l) { }

  OZ_Boolean leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = _l_size; i--; ) 
      vars_left |= _l[i].leave();
    return vars_left;
  }
  void fail(void) {
    for (int i = _l_size; i--; _l[i].fail());
  }
};
