OZ_BI_define(connect, 2, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FD);
  OZ_Expect pe;
  OZ_EXPECT(pe, 0, expectFSetVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new ConnectProp(OZ_in(0), OZ_in(1)));
}
OZ_BI_end

OZ_PropagatorProfile ConnectProp::profile;
