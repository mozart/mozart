OZ_C_proc_begin(fd_twice, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new TwiceProp(OZ_args[0], 
				 OZ_args[1]));
}
OZ_C_proc_end

