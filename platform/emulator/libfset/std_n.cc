/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "std_n.hh"
#include "standard.hh"

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_disjointN, 1)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetVarBounds);
  
  return pe.impose(new FSetDisjointNPropagator(OZ_args[0]));
} 
OZ_C_proc_end

OZ_CFun FSetDisjointNPropagator::header = fsp_disjointN;

OZ_Return FSetDisjointNPropagator::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  DECL_DYN_ARRAY(FSetTouchedGlb, vst, _vs_size);
  PropagatorController_VS P(_vs_size, vs);
  int i;

  for (i = _vs_size; i--; ) {
    vs[i].read(_vs[i]);
    vst[i] = vs[i];
  }

  OZ_FSetValue u(fs_empty);
  
  for (i = _vs_size; i--; ) { 
    OZ_FSetValue vsi_glb(vs[i]->getGlbSet());
    
    if ((u & vsi_glb).getCard() > 0)
      goto failure;

    u |= vsi_glb;
  }
  
  if (u.getCard() > 0) {
    OZ_Boolean doagain;
    
    do {
      doagain = OZ_FALSE;
      
      for (i = _vs_size; i--; ) {
	OZ_FSetValue vsi_glb(vs[i]->getGlbSet());
	
	FailOnInvalid(*vs[i] != (u - vsi_glb));
	
	if (vst[i] <= vs[i]) {
	  doagain = OZ_TRUE;
	  vst[i] = vs[i];
	  u |= vsi_glb;
	}
      }
      
    } while (doagain);
  }

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_unionN, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetVarBounds);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);
  
  return pe.impose(new FSetUnionNPropagator(OZ_args[0],
					    OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetUnionNPropagator::header = fsp_unionN;

OZ_FSetConstraint unionN(OZ_FSetVar x[], int high, int except)
{
  OZ_FSetConstraint z(fs_empty);

  for (int i = high; i--; ) 
    if (i != except) 
      z = z | *x[i];

  OZ_DEBUGPRINT(("unionN: expect=%d u=%s\n", except, z.toString()));
  return z;
}

OZ_Boolean union3(OZ_FSetConstraint &x, 
		  OZ_FSetConstraint &y, 
		  OZ_FSetConstraint &z,
		  OZ_Boolean &touched)
{
  FSetTouched xt, yt, zt;

  xt = x;  yt = y;  zt = z;
  
  FailOnInvalid(x >= (z - y)); // glb
  OZ_DEBUGPRINT(("x=%s",x.toString()));
  
  FailOnInvalid(y >= (z - x)); // glb
  OZ_DEBUGPRINT(("y=%s",y.toString()));
    
  FailOnInvalid(z <<= (x | y)); // lub
  OZ_DEBUGPRINT(("z=%s",z.toString()));
  
  FailOnInvalid(x <= z); // lub
  OZ_DEBUGPRINT(("x=%s",x.toString()));
  
  FailOnInvalid(y <= z); // lub
  OZ_DEBUGPRINT(("y=%s",y.toString()));
  
  touched |= (xt <= x || yt <= y || zt <= z);
  
  return OZ_TRUE;

failure: 
  return OZ_FALSE;
} 

#define LINEAR

OZ_Return FSetUnionNPropagator::propagate(void)
{
#ifdef LINEAR
  _OZ_DEBUGPRINTTHIS("in ");
  
  if (_vs_size == 0) {
    _OZ_DEBUGPRINTTHIS("_vs_size == 0");
    return OZ_FAILED;
  } else if (_vs_size == 1) {
    _OZ_DEBUGPRINTTHIS("_vs_size == 1");
    return OZ_DEBUGRETURNPRINT(("%d",replaceBy(_s, _vs[0])));
  } else if (_vs_size == 2) {
    return replaceBy(new FSetUnionPropagator(_vs[0], _vs[1], _s));
  }

  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  OZ_FSetVar s(_s);
  OZ_FSetConstraint * aux = _aux;
  DECL_DYN_ARRAY(FSetTouched, vst, _vs_size);
  PropagatorController_VS_S P(_vs_size, vs, s);
  int i;

  for (i = _vs_size; i--; ) 
    vs[i].read(_vs[i]);

   printf("."); fflush(stdout);

  OZ_Boolean doagain;

  do {
    doagain = OZ_FALSE;

    union3(*vs[0], *vs[1], aux[0], doagain);

    for (i = 0; i < _vs_size-3; i += 1)
      if (!union3(aux[i], *vs[i+2], aux[i+1], doagain))
	goto failure;
    
    union3(aux[_vs_size-3], *vs[_vs_size-1], *s, doagain);
      
  } while (doagain);
  
  {
    _OZ_DEBUGPRINTTHIS("out ");
    OZ_Return r = P.leave();
    /*
    if (r == OZ_SLEEP) {
      int j = 0;
      for (i = 0; i < _vs_size; i += 1) {
	if (vs[i]->isValue())
	  continue;
	_vs[j] = _vs[i];
	j += 1;
      }
      _vs_size = j;
    }
    */
    return r;
  }

failure:
  _OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  

#else /* LINEAR ----------------------------------------------------------- */

  OZ_DEBUGPRINTTHIS("in ");
  
  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  DECL_DYN_ARRAY(FSetTouched, vst, _vs_size);
  OZ_FSetVar s(_s);
  FSetTouched st(s);
  PropagatorController_VS_S P(_vs_size, vs, s);
  int i;

  for (i = _vs_size; i--; ) {
    vs[i].read(_vs[i]);
    vst[i] = vs[i];
  }

  OZ_Boolean doagain;
  do {
    doagain = OZ_FALSE;

    FailOnInvalid(*s <<= unionN(vs, _vs_size, -1));

    if (st <= s) {
      doagain = OZ_TRUE;
      st = s;
    }
		  
    for (i = _vs_size; i--; ) {

      OZ_DEBUGPRINTTHIS("in loop ");

      FailOnInvalid(*vs[i] <= *s);

      FailOnInvalid(*vs[i] >= (*s - unionN(vs, _vs_size, i)));
      
      if (vst[i] <= vs[i]) {
	doagain = OZ_TRUE;
	vst[i] = vs[i];
      }
    }

  } while (doagain);

  OZ_DEBUGPRINTTHIS("out ");
  return P.leave();

failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  
#endif /* LINEAR */
}
