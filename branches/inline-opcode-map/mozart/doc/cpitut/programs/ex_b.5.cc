OZ_BI_define(fd_add_nestable, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe,0,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,1,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,2,expectIntVar,susp_count);

  if (susp_count > 1) 
    return pe.suspend();

  return pe.impose(new AddProp(OZ_in(0),
			       OZ_in(1),
			       OZ_in(2)));
}
OZ_BI_end

