OZ_Return RILessEq::propagate(void)
{
  RIVar x(_x), y(_y);
  
  // step (1)
  if (x->upperBound() <= y->lowerBound()) {
    x.leave(); y.leave();
    return OZ_ENTAILED;
  }

  // step (2)
  if((*x <= y->upperBound()) < 0.0) 
    goto failure;

  // step (3)
  if((*y >= x->lowerBound()) < 0.0) 
    goto failure;

  return (x.leave() | y.leave()) 
    ? OZ_SLEEP : OZ_ENTAILED;

failure:
  x.fail(); y.fail();
  return OZ_FAILED;
}
