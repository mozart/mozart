OZ_BI_define(fd_twice, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new TwiceProp(OZ_in(0), 
				 OZ_in(1)));
}
OZ_BI_end

