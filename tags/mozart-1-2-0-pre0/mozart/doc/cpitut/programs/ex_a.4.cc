OZ_BI_define(fd_add, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);
  OZ_EXPECT(pe, 2, expectIntVar);

  return pe.impose(new AddProp(OZ_in(0), 
			       OZ_in(1), 
			       OZ_in(2)));
}
OZ_BI_end
