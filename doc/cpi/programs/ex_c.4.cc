void ElementProp::updateHeapRefs(OZ_Boolean) {
  OZ_updateHeapTerm(_n); OZ_updateHeapTerm(_v);

  OZ_Term * new_d = OZ_hallocOzTerms(_d_size);
  for (int i = _d_size; i--; ) {
    new_d[i] = _d[i];
    OZ_updateHeapTerm(new_d[i]);
  }
  _d = new_d;
}
