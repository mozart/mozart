OZ_C_proc_begin(fd_add, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);
  OZ_EXPECT(pe, 2, expectIntVar);

  return pe.impose(new AddProp(OZ_args[0],
                               OZ_args[1],
                               OZ_args[2]));
}
OZ_C_proc_end
