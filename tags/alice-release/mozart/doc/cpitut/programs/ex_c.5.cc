OZ_BI_define(fd_element, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD
		   ","OZ_EM_VECT OZ_EM_FD
		   ","OZ_EM_FD);

  ExtendedExpect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectIntVar);

  if (OZ_vectorSize(OZ_in(1)) == 0) 
    return pe.fail();

  return pe.impose(new ElementProp(OZ_in(0), 
				   OZ_in(1), 
				   OZ_in(2)));
}
OZ_BI_end

