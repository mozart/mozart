OZ_Return ReifiedLessEqProp::propagate()
{
  OZ_FDIntVar r(_r);

  if(*r == fd_singl) {
    r.leave();
    return replaceBy((r->getSingleElem() == 1) 
		     ? new LessEqProp(_x, _y) 
		     : new GreaterProp(_x, _y));
  }
  
  OZ_FDIntVar x, y;
  x.readEncap(_x); y.readEncap(_y);
  int r_val = 0;

  // entailed by store?
  if (x->getMaxElem() <= y->getMinElem()) {
    r_val = 1;
    goto quit;
  }

  if (0 == (*x <= y->getMaxElem())) goto quit;
  if (0 == (*y >= x->getMinElem())) goto quit;
  
  r.leave(); x.leave(); y.leave();
  return OZ_SLEEP;

quit:
  if(0 == (*r &= r_val)) {
    r.fail(); x.fail(); y.fail();
    return OZ_FAILED;
  }

  r.leave(); x.leave(); y.leave();
  return OZ_ENTAILED;
}
