OZ_Return TwiceProp::propagate(void)
{
  OZ_FDIntVar x(_x), z(_z);
  
  OZ_FiniteDomain x_aux(fd_empty), z_aux(fd_empty);

  for (int i = x->getMinElem(); i != -1; 
       i = x->getNextLargerElem(i)) {
    int i2 = 2 * i;
    if (z->isIn(i2)) {
      x_aux += i; z_aux += i2;
    }
  }

  FailOnEmpty(*x &= x_aux);
  FailOnEmpty(*z &= z_aux);
  
  return (x.leave() | z.leave())
    ? OZ_SLEEP : OZ_ENTAILED;

failure: 
  x.fail(); z.fail();
  return OZ_FAILED;
}

