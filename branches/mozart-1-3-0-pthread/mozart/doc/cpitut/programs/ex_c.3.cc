OZ_Term ElementProp::getParameters(void) const {
  OZ_Term list = OZ_nil();
  
  for (int i = _d_size; i--; )
    list = OZ_cons(_d[i], list); 
  
  return OZ_cons(_n, 
		 OZ_cons(list, 
			 OZ_cons(_v, OZ_nil())));
}
