OZ_Return ElementProp::propagate(void)
{
  OZ_FDIntVar n(_n), v(_v), d[_d_size];
  Iterator_OZ_FDIntVar D(_d_size, d);
  
  for (int i = _d_size; i--; )
    d[i].read(_d[i]);

  { /* propagation rule for n */
    OZ_FiniteDomain aux_n(fd_empty);
    
    for (int i = _d_size; i --; )
      if ((*(d[i]) & *v) != fd_empty) 
	aux_n += (i + 1);
    
    FailOnEmpty(*n &= aux_n);
  }
  { /* propagation rule for v */
    OZ_FiniteDomain aux_v(fd_empty);
    
    for (int i = n->getMinElem(); 
	 i != -1;
	 i = n->getNextLargerElem(i))
      aux_v = aux_v | *(d[i - 1]);
    
    FailOnEmpty(*v &= aux_v);
  }
  { /* propagation rule for d[n] */
    if (n->getSize() == 1) {
      int o = n->getSingleElem();
      D.leave(); n.leave(); v.leave();
      return replaceBy(_v, _d[o - 1]);
    }
  }
  return (D.leave() | n.leave() | v.leave()) 
    ? OZ_SLEEP : OZ_ENTAILED;

failure:
  D.fail(); n.fail(); v.fail();
  return OZ_FAILED;
}
