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

  OZ_FSetValue u = _u;
  
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

  {
    OZ_Return r = P.leave();
    
    if (r == OZ_SLEEP) {
      
      int j = 0;
      for (i = 0; i < _vs_size; i += 1) {
        if (vs[i]->isValue()) {
          _u |= vs[i]->getGlbSet();
          continue;
        }
        _vs[j] = _vs[i];
        j += 1;
      }
      _vs_size = j;
    }
    
    OZ_DEBUGPRINTTHIS("out ");
    return r;
  }

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

inline
OZ_FSetConstraint unionN(OZ_FSetVar x[], int high, int except)
{
  OZ_FSetConstraint z(fs_empty);

  for (int i = high; i--; )
    if (i != except) 
      z = z | *x[i];

  OZ_DEBUGPRINT(("unionN: expect=%d u=%s\n", except, z.toString()));
  return z;
}

inline
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
  OZ_DEBUGPRINTTHIS("in ");
  
  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  OZ_FSetVar s(_s);
  OZ_FSetConstraint * aux = _aux;
  DECL_DYN_ARRAY(FSetTouched, vst, _vs_size);
  PropagatorController_VS_S P(_vs_size, vs, s);
  int i;

  for (i = _vs_size; i--; ) 
    vs[i].read(_vs[i]);

  OZ_Boolean doagain;

  if (_vs_size == 0) {
    OZ_DEBUGPRINTTHIS("_vs_size == 0");
    FailOnInvalid(*s <<= _u);
    return P.vanish();
  } else if (_vs_size == 1) {
    OZ_DEBUGPRINTTHIS("_vs_size == 1");
    do {
      doagain = OZ_FALSE;
      
      if (!union3(_u, *vs[0], *s, doagain))
	goto failure;
      
    } while (doagain);
    
    return P.leave();
  } 


  do {
    doagain = OZ_FALSE;

    if (!union3(_u, *vs[0], aux[0], doagain))
      goto failure;

    for (i = 0; i < _aux_size-1; i += 1)
      if (!union3(aux[i], *vs[i+1], aux[i+1], doagain))
        goto failure;
    
    if (!union3(*vs[_vs_size-1], aux[_aux_size-1], *s, doagain))
      goto failure;

  } while (doagain);
  
  {
    OZ_Return r = P.leave();
    
    if (r == OZ_SLEEP) {
      
      int j = 0;
      for (i = 0; i < _vs_size; i += 1) {
        if (vs[i]->isEmpty() && i != _vs_size) {
          _u = _u | *vs[i];
	  continue;
        }
        _vs[j] = _vs[i];
	_aux[j] = _aux[i];
	  
        j += 1;
      }
      if (_vs_size != j) {
        _vs_size = j;
        _aux_size = max(0,_vs_size - 1);
      }
    }
    
    OZ_DEBUGPRINTTHIS("out ");
    return r;
  }

failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  

#else /* LINEAR ----------------------------------------------------------- */

  OZ_DEBUGPRINTTHIS("in ");
  
  //printf("."); fflush(stdout);

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

    OZ_FSetConstraint u = _u | unionN(vs, _vs_size, -1);

    FailOnInvalid(*s <<= u);
    //FailOnInvalid(*s <<= unionN(vs, _vs_size, -1));

    if (st <= s) {
      doagain = OZ_TRUE;
      st = s;
    }

    for (i = _vs_size; i--; ) {

      OZ_DEBUGPRINTTHIS("in loop ");

      FailOnInvalid(*vs[i] <= *s);

      OZ_FSetConstraint a = (unionN(vs, _vs_size, i) | _u);
      OZ_FSetConstraint b = (u - *vs[i]);


      if (((*s-a).getGlbSet()-(*s-b).getGlbSet()).getCard() > 0) {
	printf("%s\n\n", this->toString());
	printf("unionN=%s ",  unionN(vs, _vs_size, i).toString());
	printf("_u=%s ", _u.toString());
	printf("a=%s\n", a.toString());
	printf("sa=%s\n\n", (*s-a).toString());

	printf("u=%s ", u.toString());
	printf("vs[i]=%s", vs[i]->toString());
	printf("b=%s\n", b.toString());
	printf("sb=%s\n\n", (*s-b).toString());
	printf("a-b=%s\n", (a.getGlbSet()-b.getGlbSet()).toString());
	printf("sa-sb=%s\n\n", ((*s-a).getGlbSet()-(*s-b).getGlbSet()).toString());
	printf("-------------------------------------------------------------\n");
	fflush(stdout);
      }
      
      //FailOnInvalid(*vs[i] >= (*s - a));
      FailOnInvalid(*vs[i] >= (*s - b));

      if (vst[i] <= vs[i]) {
	doagain = OZ_TRUE;
	vst[i] = vs[i];
      }
    }

  } while (doagain);

  {
    OZ_Return r = P.leave();
        
    if (r == OZ_SLEEP) {
      
      int j = 0;
      for (i = 0; i < _vs_size; i += 1) {
        if (vs[i]->isValue()) {
          _u = _u | *vs[i];
          continue;
        }
        _vs[j] = _vs[i];
        j += 1;
      }
      _vs_size = j;
    }
    
    OZ_DEBUGPRINTTHIS("out ");
    return r;
  }
failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  
#endif /* LINEAR */
}

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fsp_partition, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorFSetVarBounds);
  OZ_EXPECT(pe, 1, expectFSetVarBounds);
  
  return pe.impose(new FSetPartitionPropagator(OZ_args[0],
					       OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetPartitionPropagator::header = fsp_partition;

OZ_Return FSetPartitionPropagator::propagate(void)
{
  _OZ_DEBUGPRINTTHIS("in ");
  
  DECL_DYN_ARRAY(OZ_FSetVar, vs, _vs_size);
  OZ_FSetVar s(_s);
  OZ_FSetConstraint * aux = _aux;
  DECL_DYN_ARRAY(FSetTouched, vst, _vs_size);
  PropagatorController_VS_S P(_vs_size, vs, s);
  int i;

  for (i = _vs_size; i--; ) 
    vs[i].read(_vs[i]);

  OZ_Boolean doagain;

  if (_vs_size == 0) {
    OZ_DEBUGPRINTTHIS("_vs_size == 0");
    FailOnInvalid(*s <<= _u);
    return P.vanish();
  } else if (_vs_size == 1) {
    OZ_DEBUGPRINTTHIS("_vs_size == 1");
    do {
      doagain = OZ_FALSE;
      
      if (!union3(_u, *vs[0], *s, doagain))
	goto failure;
      
    } while (doagain);
    
    return P.leave();
  } 

  {
    OZ_FSetValue u(fs_empty);
    
    for (i = _vs_size; i--; ) { 
      OZ_FSetValue vsi_glb(vs[i]->getGlbSet());
      
      if ((u & vsi_glb).getCard() > 0)
	goto failure;
      
      u |= vsi_glb;
    }
    
    OZ_Boolean dodisjoint = u.getCard() > 0;
    
    do {
      doagain = OZ_FALSE;
      
      if (dodisjoint)
	for (i = _vs_size; i--; ) {
	  OZ_FSetValue vsi_glb(vs[i]->getGlbSet());
	  
	  FailOnInvalid(*vs[i] != (u - vsi_glb));
	  
	  if (vst[i] <= vs[i]) {
	    doagain = OZ_TRUE;
	    vst[i] = vs[i];
	    u |= vsi_glb;
	  }
	}
      
      if (!union3(_u, *vs[0], aux[0], doagain))
	goto failure;
      
      for (i = 0; i < _aux_size-1; i += 1)
	if (!union3(aux[i], *vs[i+1], aux[i+1], doagain))
	  goto failure;
      
      if (!union3(*vs[_vs_size-1], aux[_aux_size-1], *s, doagain))
	goto failure;
      
    } while (doagain);
  }
  {
    OZ_Return r = P.leave();
    
    if (r == OZ_SLEEP) {
      
      int j = 0;
      for (i = 0; i < _vs_size; i += 1) {
        if (vs[i]->isEmpty() && i != _vs_size) {
	  continue;
        }
        _vs[j] = _vs[i];
	_aux[j] = _aux[i];
	  
        j += 1;
      }
      if (_vs_size != j) {
        _vs_size = j;
        _aux_size = max(0,_vs_size - 1);
      }
    }
    
    _OZ_DEBUGPRINTTHIS("out ");
    return r;
  }

failure:
  _OZ_DEBUGPRINTTHIS("failed");
  return P.fail();  

}
