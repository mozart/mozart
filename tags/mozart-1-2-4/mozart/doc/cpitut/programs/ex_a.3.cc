  OZ_FDIntVar x(_x), y(_y), z(_z);
  
  OZ_FiniteDomain x_aux(fd_empty), 
                  y_aux(fd_empty), 
                  z_aux(fd_empty);

  for (int i = x->getMinElem(); i != -1; 
       i = x->getNextLargerElem(i))
    for (int j = y->getMinElem(); j != -1; 
	 j = y->getNextLargerElem(j))
      if (z->isIn(i + j)) {
	x_aux += i;
	y_aux += j; 
	z_aux += (i + j);
      }
  
  FailOnEmpty(*x &= x_aux);
  FailOnEmpty(*y &= y_aux);
  FailOnEmpty(*z &= z_aux);
  
  return (x.leave() | y.leave() | z.leave()) 
    ? OZ_SLEEP : OZ_ENTAILED;

failure: 
  x.fail();
  y.fail();
  z.fail();
  return OZ_FAILED;
}

