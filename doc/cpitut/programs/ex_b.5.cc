OZ_C_proc_begin(fd_add_nestable, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe,0,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,1,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,2,expectIntVar,susp_count);

  if (susp_count > 1)
    return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new AddProp(OZ_args[0],
                               OZ_args[1],
                               OZ_args[2]));
}
OZ_C_proc_end
