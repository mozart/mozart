OZ_C_proc_begin(fd_element, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD
		   ","OZ_EM_VECT OZ_EM_FD
		   ","OZ_EM_FD);

  ExtendedExpect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectIntVar);

  if (OZ_vectorSize(OZ_args[1]) == 0) 
    return pe.fail();

  return pe.impose(new ElementProp(OZ_args[0], 
				   OZ_args[1], 
				   OZ_args[2]));
}
OZ_C_proc_end

