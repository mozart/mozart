void ElementProp::gCollect(void) {
  OZ_gCollectTerm(_n);
  OZ_gCollectTerm(_v);
  _d = OZ_gCollectAllocBlock(_d_size, _d);
}

void ElementProp::sClone(void) {
  OZ_sCloneTerm(_n);
  OZ_sCloneTerm(_v);
  _d = OZ_sCloneAllocBlock(_d_size, _d);
}
