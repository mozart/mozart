/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "standard.hh"

OZ_C_proc_begin(fsp_intersection, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new FSetIntersectionPropagator(OZ_args[0],
						  OZ_args[1],
						  OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFun FSetIntersectionPropagator::spawner = fsp_intersection;

OZ_C_proc_begin(fsp_union, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 2, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new FSetUnionPropagator(OZ_args[0],
					   OZ_args[1],
					   OZ_args[2]));
} 
OZ_C_proc_end

OZ_CFun FSetUnionPropagator::spawner = fsp_union;

OZ_C_proc_begin(fsp_subsume, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new FSetSubsumePropagator(OZ_args[0],
					     OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetSubsumePropagator::spawner = fsp_subsume;


OZ_C_proc_begin(fsp_disjoint, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET "," OZ_EM_FSET);

  PropagatorExpect pe;

  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe, 0, expectFSetVarBounds, susp_count);
  OZ_EXPECT_SUSPEND(pe, 1, expectFSetVarBounds, susp_count);
  
  if (susp_count > 1) return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new FSetDisjointPropagator(OZ_args[0],
					      OZ_args[1]));
} 
OZ_C_proc_end

OZ_CFun FSetDisjointPropagator::spawner = fsp_disjoint;

//*****************************************************************************

OZ_Return FSetIntersectionPropagator::propagate(void) 
{
  OZ_DEBUGPRINT("in " << *this);

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;
    
    if (z->isEmpty()) {
      OZ_DEBUGPRINT("replace: (z empty)" << *this);
      P.vanish();
      return replaceBy(new FSetDisjointPropagator(_x, _y));
    }
    if (x->isSubsumedBy(*y)) {
      OZ_DEBUGPRINT("replace: (x subsumbed by y)" << *this);
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_x, _z));
    }
    if (y->isSubsumedBy(*x)) {
      OZ_DEBUGPRINT("replace: (y subsumbed by xy)" << *this);
      P.vanish();
      return OZ_DEBUGRETURNPRINT(replaceBy(_y, _z));
    }
    
    FailOnInvalid(*x <= -(- *z & *y)); // lub
    OZ_DEBUGPRINT("x=" << *x);
    FailOnInvalid(*y <= -(- *z & *x)); // lub
    OZ_DEBUGPRINT("y=" << *y);
    
    FailOnInvalid(*z <<= (*x & *y)); // glb
    OZ_DEBUGPRINT("z=" << *z);
    FailOnInvalid(*x >= *z); // glb
    OZ_DEBUGPRINT("x=" << *x);
    FailOnInvalid(*y >= *z); // glb
    OZ_DEBUGPRINT("y=" << *y);
    
  } while (xt <= x || yt <= y || zt <= z);
    
    return OZ_DEBUGRETURNPRINT(P.leave1()); 
  
failure:
  OZ_DEBUGPRINT("failed " << *this);
  return P.fail();
}

OZ_Return FSetUnionPropagator::propagate(void) 
{
  OZ_DEBUGPRINT("in " << *this);

  OZ_FSetVar x(_x), y(_y), z(_z);
  PropagatorController_S_S_S P(x, y, z);
  FSetTouched xt, yt, zt;

  do {
    xt = x;  yt = y;  zt = z;
    
    if (z->isEmpty()) {
      OZ_FSetConstraint aux(fs_empty);
      FailOnInvalid(*x <<= aux);
      FailOnInvalid(*y <<= aux);
      P.vanish();
      return ENTAILED;
    }
    if (x->isSubsumedBy(*y)) {
      P.vanish();
      return replaceBy(_y, _z);
    }
    if (y->isSubsumedBy(*x)) {
      P.vanish();
      return replaceBy(_x, _z);
    }
    
    FailOnInvalid(*x >= (*z & - *y)); // glb
    OZ_DEBUGPRINT("x=" << *x);
    FailOnInvalid(*y >= (*z & - *x)); // glb
    OZ_DEBUGPRINT("y=" << *y);
    
    FailOnInvalid(*z <<= (*x | *y)); // lub
    OZ_DEBUGPRINT("z=" << *z);
    FailOnInvalid(*x <= *z); // lub
    OZ_DEBUGPRINT("x=" << *x);
    FailOnInvalid(*y <= *z); // lub
    OZ_DEBUGPRINT("y=" << *y);
    
  } while (xt <= x || yt <= y || zt <= z);
  
  OZ_DEBUGPRINT("out " << *this);
  return P.leave1();
  
failure:
  OZ_DEBUGPRINT("failed " << *this);
  return P.fail();
}

OZ_Return FSetSubsumePropagator::propagate(void) 
{
  OZ_DEBUGPRINT("int " << *this);
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FailOnInvalid(*x <= *y);
  FailOnInvalid(*y >= *x);

  OZ_DEBUGPRINT("out " << *this);
  return P.leave1(); /* is entailed if only 
			one var is left */
  
failure:
  OZ_DEBUGPRINT("failed " << *this);
  return P.fail();
}

OZ_Return FSetDisjointPropagator::propagate(void) 
{
  OZ_DEBUGPRINT("in " << *this);
  OZ_FSetVar x(_x), y(_y);
  PropagatorController_S_S P(x, y);

  FailOnInvalid(*x != *y);
  FailOnInvalid(*y != *x);

  OZ_DEBUGPRINT("out " << *this);
  return P.leave1(); /* is entailed if only 
			one var is left */
  
failure:
  OZ_DEBUGPRINT("failed " << *this);
  return P.fail();
}

//--------------------------------------------------------------------
// Set Difference Propagator (DENYS)

OZ_C_proc_begin(fsp_diff, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FSET","OZ_EM_FSET);
  PropagatorExpect pe;
  int susp_count=0;
  OZ_EXPECT_SUSPEND(pe,0,expectFSetVarBounds,susp_count);
  OZ_EXPECT_SUSPEND(pe,1,expectFSetVarBounds,susp_count);
  OZ_EXPECT_SUSPEND(pe,2,expectFSetVarBounds,susp_count);
  if (susp_count>1) return pe.suspend(OZ_makeSelfSuspendedThread());
  return pe.impose(new FSetDiffPropagator(OZ_args[0],OZ_args[1],OZ_args[2]));
}
OZ_C_proc_end

OZ_CFun FSetDiffPropagator::header = fsp_diff;

OZ_Return FSetDiffPropagator::propagate(void)
{
  OZ_FSetVar x(_x),y(_y),z(_z);
  PropagatorController_S_S_S P(x,y,z);
  FSetTouched xt,yt,zt;

  do {
    xt=x; yt=y; zt=z;
    // x-y=z
    //	disjoint(y,z)
    //	union(x,z)=union(y,z)
    //
    // disjoint(y,z)
    //
    FailOnInvalid(*y != *z);
    FailOnInvalid(*z != *y);
    //
    // union(x,y)=union(y,z)
    //	x subset union(y,z)
    //	z = x-y
    //	x supset z
    //	y supset x-z
    //
    FailOnInvalid(*x <= (*y | *z));
    FailOnInvalid(*z <<= (*x & - *y));
    FailOnInvalid(*x >= *z);
    FailOnInvalid(*y >= (*x & - *z));
  } while (xt <= x || yt <= y || zt <= z);

  return P.leave1();
failure:
  return P.fail();
}
