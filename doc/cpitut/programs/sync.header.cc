OZ_C_proc_begin(connect, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FD);
  OZ_Expect pe;
  OZ_EXPECT(pe, 0, expectFSetVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new ConnectProp(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_PropagatorProfile ConnectProp::profile;
