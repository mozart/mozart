/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "fdbuilti.hh"


//-----------------------------------------------------------------------------
// BIfdConstrDisj
// Argument structure:
// [P_1..P_n] [B_1..B_n] [V_1..V_m] [|~~~~~~| .. |~~~~~~|]
//                                    Vp_1_1      Vp_n_1
//                                       .           .
//				         .           .
//				      Vp_1_m      Vp_n_m
//                                  [|______| .. |______|]
// P: number of propagators in clause i (expected to be small int)
// V: global variable relevant for disjunction (expected to be finite domain)
// B: variable which reifies true value of clause (expected to be uvar)
// Vp: local variables in clauses (expected to be uvar)
//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfdConstrDisjSetUp, 4)
{ 
  ExpectedTypes("Tuple of SmallInts,Tuple of FiniteDomains,"
		"Tuple of NotCVars,Tuple of NotCVars");
  
  OZ_getCArgDeref(0, p_tuple, p_tupleptr, p_tupletag);
  OZ_getCArgDeref(1, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(2, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(3, vp_tuple, vp_tupleptr, vp_tupletag);

  if (! isSTuple(p_tupletag)) TypeError(0, "");
  if (! isSTuple(b_tupletag)) TypeError(1, "");

  STuple &p = *tagged2STuple(p_tuple);
  STuple &b = *tagged2STuple(b_tuple);

  if (isLiteral(v_tupletag)) {
    // reduce to sum(b) >= 1
    int p_size = p.getSize();
    TaggedRef tone = newSmallInt(1), tmone = newSmallInt(-1);
    STuple * st = STuple::newSTuple(p.getLabel(), p_size);
    
    for (int i = 0; i < p_size; i++) (*st)[i] = tmone;
    
    TaggedRefPtr new_args = allocateRegs(makeTaggedSTuple(st), b_tuple, tone);
    return BIfdGenLinLessEq(3, new_args);
  }
  
  if (! isSTuple(v_tupletag)) TypeError(2, "");
  if (! isSTuple(vp_tupletag)) TypeError(3, "");

  STuple &v = *tagged2STuple(v_tuple);
  STuple &vp = *tagged2STuple(vp_tuple);

  const int clauses = p.getSize();
  if (clauses != b.getSize()) {
    warning("Tuples clauses and b differ in size.");
    return FAILED;
  }

  // constrain Bi to {0..Pi+2} if Bi is an uvar
  int i;
  for (i = clauses; i--; ) {
    TaggedRef bi = makeTaggedRef(&b[i]);
    DEREF(bi, bi_ptr, bi_tag);
    if (isNotCVar(bi_tag)) {
      GenFDVariable * fdvar = new GenFDVariable();
      fdvar->getDom().init(0, smallIntValue(deref(p[i])) + 2);
      doBind(bi_ptr, makeTaggedRef(newTaggedCVar(fdvar)));
    } else {
      error("Unexpected CVar found.");
    }
  } 

  const int variables = v.getSize();

  // constrain Vpij to {fd_inf..fd_sup} if Vpij is an uvar
  for (i = clauses; i--; ) {
    DEREF(vp[i], vp_i_ptr, vp_i_tag);
    if (! isSTuple(vp_i_tag)) TypeError(3, "2-dim-array expected");
    STuple &vp_i = *tagged2STuple(vp[i]);
    
    if (vp_i.getSize() != variables) {
      warning("2-dim-array index incorrect in BIfdConstrDisjSetUp");
      return FAILED;
    }
    
    for (int j = variables; j--; ) {
      TaggedRef vp_i_j = makeTaggedRef(&vp_i[j]);
      DEREF(vp_i_j, vp_i_j_ptr, vp_i_j_tag);
      if (isNotCVar(vp_i_j_tag)) {
	GenFDVariable * fdvar = new GenFDVariable();
	fdvar->getDom().initFull();
	doBind(vp_i_j_ptr, makeTaggedRef(newTaggedCVar(fdvar)));
      }
    }
  }
  return PROCEED; 
}
OZ_C_proc_end 
  
OZ_C_proc_begin(BIfdConstrDisj, 3)
{ 
  ExpectedTypes("Tuple of FiniteDomains,Tuple of NotCVars,Tuple of NotCVars");
  
  OZ_getCArgDeref(0, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(1, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(2, vp_tuple, vp_tupleptr, vp_tupletag);

  // Has already been reduced to sum(b) >= 1
  if (isLiteral(v_tupletag)) return PROCEED;


  if (! isSTuple(b_tupletag)) TypeError(0, "");
  if (! isSTuple(v_tupletag)) TypeError(1, "");
  if (! isSTuple(vp_tupletag)) TypeError(2, "");

  STuple &v = *tagged2STuple(v_tuple);

  const int variables = v.getSize();

  // suspend until global variables are constrained to finite domains
  BIfdHeadManager x_items(variables);

  int suspend, i;
  for (suspend = 0, i = variables; i--; ) 
    if (! x_items.expectFDish(i, makeTaggedRef(&v[i]), suspend))
      TypeError(1, "");

  if (suspend > 0) { 
    return x_items.addSuspFDish(OZ_self, OZ_args, OZ_arity);
  }

  return x_items.spawnPropagator(fd_any, BIfdConstrDisj_body, 3, 
				 b_tuple, v_tuple, vp_tuple);
  //  FDcurrentTaskSusp->markStable();
}
OZ_C_proc_end 


//-----------------------------------------------------------------------------
// BIfdConstrDisj
// Argument structure:    clause 1    clause n
// [B_1..B_n] [V_1..V_m] [|~~~~~~| .. |~~~~~~|]
//                         Vp_1_1      Vp_n_1
//                           .           .
//	                     .           .
//			   Vp_1_m      Vp_n_m
//                       [|______| .. |______|]
// V:global variable relevant for disjunction (expected to be finite domain)
// B:variable which reifies true value of clause (expected to be finite domain)
// Vp: local variables in clauses (expected to be finite domain)
// m: number of variables involved; n: number of clauses
//
// Invariants:
//   o Bs carry suspensions propagators in clauses
//   o Vps carry exclusively suspensions to propagators in the clauses
//   o Vs must not carry suspensions to propagators in the clauses
// Structure of the built-in:
//   o check if variables got unified and if so propagate equality to
//     local variables
//   o constrain local variables with corresponding global ones
//   o check if unit commit, top commit, failure etc. occured and take
//     appropriate action
//   o propagate till you reach a fixpoint
//   o check if unit commit, top commit, failure etc. occured and take
//     appropriate action
//   o constrain global variables with union of corresponding local vars
//-----------------------------------------------------------------------------


OZ_C_proc_begin(BIfdConstrDisj_body, 3)
{
  OZ_getCArgDeref(0, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(1, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(2, vp_tuple, vp_tupleptr, vp_tupletag);
  
  STuple &_b = *tagged2STuple(b_tuple);
  STuple &_v = *tagged2STuple(v_tuple);
  STuple &_vp = *tagged2STuple(vp_tuple);

  const int clauses = _b.getSize(); // number of clauses
  const int variables = _v.getSize(); // number ob variables
  int failed_clauses = 0, not_failed_clause = -1, entailed_clause = -1;
  
  BIfdBodyManager x(0);
  
  // introduce Bs
  x.add(0, clauses);
  int c;
  for (c = clauses; c--; ) {
    x.introduce(idx_b(c), makeTaggedRef(&_b[c]));
  }
  
  // introduce Vs
  x.add(1, variables);
  int v;
  for (v = variables; v--; ) {
    x.introduce(idx_v(v), makeTaggedRef(&_v[v]));
  }
  
  // introduce Vps
  for (c = 0; c < clauses; c += 1) {  // acendingly counting ('cause of x.add)
    x.add(2 + c, variables);

    if (x[idx_b(c)] == 0) {
      for (v = variables; v--; ) 
	x.introduceDummy(idx_vp(c, v));
    } else {
      STuple &vp_c = *tagged2STuple(deref(_vp[c]));
      for (v = variables; v--; ) 
	x.introduce(idx_vp(c, v), makeTaggedRef(&vp_c[v]));
    }
  }
  
// check if vars got unified and if so propagate equality to local vars
  x.propagate_unify_cd(clauses, variables, _vp);

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      continue;
    } else for (v = variables; v--; ) 
      if (x.isNotCDVoid(idx_vp(c, v))) 
	if ((x[idx_vp(c, v)] &= x[idx_v(v)]) == 0) {
	  x[idx_b(c)] &= 0;  // intersection is empty --> clause failed
	  break;
	} 
  }

// propagate till you reach a fixpoint
  localPropStore.backup(0x10);

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      x.process(idx_b(c));
      x.backup();
      if (!localPropStore.do_propagation()) 
	error("local propagation must be empty");
      x.restore();
      continue;
    }

    x.process(idx_b(c));

    Assert(x[idx_b(c)] != 0);
    
#ifndef TM_LP
    localPropStore.setUseIt();
#endif

    for (v = variables; v--; )
      x.process(idx_vp(c, v));

#ifndef TM_LP
    localPropStore.unsetUseIt();
#endif

    x.backup();
    if (!localPropStore.do_propagation()) 
      error("local propagation must be empty");
    x.restore();

    Assert(localPropStore.isEmpty());
  
  }

  localPropStore.restore();

// Note: since Bs and Vps are local, reintroduction is actual superfluous,
// since domains can get singletons and the according variable get disposed,
// we need to reintroduce Bs and Vps
  // reintroduce Bs
  for (c = clauses; c--; ) {
    x.reintroduce(idx_b(c), makeTaggedRef(&_b[c]));
  }
  
  // reintroduce Vps
  for (c = 0; c < clauses; c += 1) {  // acendingly counting ('cause of x.add)
    if (x[idx_b(c)] != 0) {
      STuple &vp_c = *tagged2STuple(deref(_vp[c]));
      for (v = variables; v--; ) { 
	x.reintroduce(idx_vp(c, v), makeTaggedRef(&vp_c[v]));
      }
    }
  }

// check if unit commit, top commit, failure occurs
  failed_clauses = 0;
  not_failed_clause = -1;
  entailed_clause = -1;

  for (c = clauses; c--; ) {
    if (x[idx_b(c)] == 0) {
      failed_clauses += 1;
    } else {
      not_failed_clause = c;

      if (x[idx_b(c)].maxElem() == 2) {
	Bool top_commit = TRUE;
	for (v = variables; v-- && top_commit; ) 
	  if (x.isNotCDVoid(idx_vp(c, v)))
	    top_commit &= (x[idx_vp(c, v)].getSize() == x[idx_v(v)].getSize());
	if (top_commit) {
	  entailed_clause = c;
	  break;
	}
      }
    }
  }
  if (failed_clauses == clauses) {                            // failure
    return FailFD;
  } else if ((clauses - failed_clauses) == 1) {               // unit commit
    
    for (v = variables; v--; )
      x[idx_v(v)] &= x[idx_vp(not_failed_clause, v)];
    x[idx_b(not_failed_clause)] &= 1;
    
    return x.entailmentClause(idx_b(0), idx_b(clauses - 1),
			      idx_v(0), idx_v(variables - 1),
			      idx_vp(not_failed_clause, 0),
			      idx_vp(not_failed_clause, variables - 1));
  } else if (entailed_clause != -1) {                         // top commit
    for (c = clauses; c--; )
      if (c != entailed_clause)
	x[idx_b(c)] &= 0;
    x[idx_b(entailed_clause)] &= 1;

    return x.entailmentClause(idx_b(0), idx_b(clauses - 1));
  }
  
// constrain global variables with union of corresponding local vars
  for (v = variables; v--; ) {
    int maxsize = 0, v_v_size = x[idx_v(v)].getSize();
    for (c = clauses; c-- && v_v_size < maxsize; ) 
      if (x[idx_b(c)] != 0)
	maxsize = max(maxsize, x[idx_vp(c, v)].getSize());

    if (maxsize < v_v_size) {
      LocalFD u;
      u.initEmpty();
      for (c = clauses; c--; ) {
	if (x[idx_b(c)] == 0) continue;
	u = u | x[idx_vp(c, v)];
      }
      x[idx_v(v)] &= u;
    }
  }

  DebugCode(for (c = clauses; c--; ) Assert(x[idx_b(c)] != 1);;)
    
  return x.releaseReify(idx_b(0), idx_b(clauses - 1),
			idx_v(0), idx_v(variables - 1));
}
OZ_C_proc_end 


//-----------------------------------------------------------------------------
// BIfdGenLessEqCD

OZ_Bool cd_wrapper_a(int OZ_arity, OZ_Term OZ_args[], OZ_CFun, OZ_CFun BI_body)
{
  int last_index = OZ_arity - 1;
  
  BIfdBodyManager x(1);
  
  x.introduce(0, OZ_getCArg(last_index));

  Assert(x.allVarsAreLocal());
  
  if (x[0] == 0) {
    return EntailFD;
  }
  if (x[0] == 1) {
    FDcurrentTaskSusp->getCCont()->setCFuncAndArity(BI_body, last_index);
    return BI_body(last_index, OZ_args);
  }

  x.backup();
  OZ_Bool ret_val = BI_body(last_index, OZ_args);
  x.restore();

  Assert(x[0].maxElem() >= 2);
  if (FDcurrentTaskSusp->isPropagated()) {
    if (ret_val == PROCEED) {
      x[0] <= (x[0].maxElem() - 1);
      Assert(x[0].maxElem() >= 2);
    } else {
      x[0] &= 0;
    }
  }

  x.process(0);

  return EntailFD;
}

OZ_C_proc_begin(BIfdGenLinEqCD, 4)
{ 
  return genericHead_a_x_c_b(OZ_arity, OZ_args, OZ_self,
			     BIfdGenLinEqCD_body, fd_bounds);
}
OZ_C_proc_end 

OZ_C_proc_begin(BIfdGenNonLinEqCD, 4)
{ 
  return genericHead_a_x_c_b_nl(OZ_arity, OZ_args, OZ_self,
				BIfdGenLinEqCD_body, fd_bounds);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdGenLinEqCD_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdGenLinEq_body);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdGenLinLessEqCD, 4)
{ 
  return genericHead_a_x_c_b(OZ_arity, OZ_args, OZ_self,
			     BIfdGenLinLessEqCD_body, fd_bounds);
}
OZ_C_proc_end 

OZ_C_proc_begin(BIfdGenNonLinLessEqCD, 4)
{ 
  return genericHead_a_x_c_b_nl(OZ_arity, OZ_args, OZ_self,
				BIfdGenLinLessEqCD_body, fd_bounds);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdGenLinLessEqCD_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdGenLinLessEq_body);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdGenLinNotEqCD, 4)
{ 
  return genericHead_a_x_c_b(OZ_arity, OZ_args, OZ_self,
			     BIfdGenLinNotEqCD_body, fd_bounds);
}
OZ_C_proc_end 

OZ_C_proc_begin(BIfdGenNonLinNotEqCD, 4)
{ 
  return genericHead_a_x_c_b_nl(OZ_arity, OZ_args, OZ_self,
				BIfdGenLinNotEqCD_body, fd_bounds);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdGenLinNotEqCD_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdGenLinNotEq_body);
}
OZ_C_proc_end 


OZ_C_proc_begin(BIfdPlusCD_rel, 4)
{
  return genericHead_x_y_z_b(OZ_arity, OZ_args, OZ_self, BIfdPlusCD_rel_body);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdPlusCD_rel_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdPlus_body);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdMultCD_rel, 4)
{
  return genericHead_x_y_z_b(OZ_arity, OZ_args, OZ_self, BIfdMultCD_rel_body);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdMultCD_rel_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdMult_body);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdLessEqOffCD, 4)
{
  return genericHead_x_y_c_b(OZ_arity, OZ_args, OZ_self, BIfdLessEqOffCD_body,
			     fd_bounds);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdLessEqOffCD_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdLessEqOff_body);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdNotEqOffCD, 4)
{
  return genericHead_x_y_c_b(OZ_arity, OZ_args, OZ_self, BIfdNotEqOffCD_body,
			     fd_det);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdNotEqOffCD_body, 4)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdNotEqOff_body);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdNotEqCD, 3)
{
  return genericHead_x_y_c_b(OZ_arity, OZ_args, OZ_self, BIfdNotEqCD_body,
			     fd_det);
}
OZ_C_proc_end


OZ_C_proc_begin(BIfdNotEqCD_body, 3)
{
  return cd_wrapper_a(OZ_arity, OZ_args, OZ_self, BIfdNotEq_body);
}
OZ_C_proc_end


OZ_Bool cd_wrapper_b(int OZ_arity, OZ_Term OZ_args[], OZ_CFun, OZ_CFun BI_body)
{
  int last_index = OZ_arity - 1;
  
  BIfdBodyManager x;
  
  x.introduce(OZ_getCArg(last_index));

  Assert(x[0] != 1); // clause cannot already be entailed
  
  if (x[0] == 0) {
    return EntailFD;
  }

  x.backup();
  OZ_Bool ret_val = BI_body(last_index, OZ_args);
  x.restore();

  Assert(x[0].maxElem() >= 2);
  if (ret_val == PROCEED) {
    x[0] <= (x[0].maxElem() - 1);
    Assert(x[0].maxElem() >= 2);
  } else {
    x[0] &= 0;
  }

  x.process(0);

  return EntailFD;
}

OZ_C_proc_begin(BIfdPutLeCD, 3)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdPutLe);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdPutGeCD, 3)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdPutGe);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdPutListCD, 4)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdPutList);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdPutIntervalCD, 4)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdPutInterval);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdPutNotCD, 3)
{
  return cd_wrapper_b(OZ_arity, OZ_args, OZ_self, BIfdPutNot);
}
OZ_C_proc_end


//-----------------------------------------------------------------------------
// Special scheduling constructive disjunctions
//-----------------------------------------------------------------------------
// Constructive Disjunction for scheduling only

OZ_C_proc_begin(BIfdCDSched, 4)
{
  return genericHead_x_y_c_d(OZ_arity, OZ_args, OZ_self, BIfdCDSched_body,
			     fd_bounds, fd_bounds);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdCDSched_body, 4)
{
  BIfdBodyManager a(2); //number of FDs
  
  for (int i = 2; i--; )
    a.introduce(i, OZ_getCArg(i));
  
  const int x = 0, y = 1;
  
  int xd = smallIntValue(deref(OZ_getCArg(2)));
  int yd = smallIntValue(deref(OZ_getCArg(3)));
  
  int xl = a[x].minElem(), xu = a[x].maxElem();
  int yl = a[y].minElem(), yu = a[y].maxElem();
  
  if (xu + xd <= yl) return a.entailment();
  if (yu + yd <= xl) return a.entailment();
  
  if (xl + xd > yu){
    killPropagatedCurrentTaskSusp();
    return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(y),
					 OZ_getCArg(x),
					 newSmallInt(-yd)));
  }
    
  if (yl + yd > xu){
    killPropagatedCurrentTaskSusp();
    return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(x),
					 OZ_getCArg(y),
					 newSmallInt(-xd)));
  }

  LocalFD la, lb, lc, ld, l1, l2;

  la.init(0, yu - xd);
  lb.init(yl + yd, fd_sup);
  lc.init(0, xu - yd);
  ld.init(xl + xd, fd_sup);

  l1 = (la | lb);
  l2 = (lc | ld);
  FailOnEmpty(a[x] &= l1);
  FailOnEmpty(a[y] &= l2);
    
  return a.release();
}
OZ_C_proc_end

//-----------------------------------------------------------------------------
// Constructive Disjunction for scheduling only

OZ_C_proc_begin(BIfdCDSchedControl, 5)
{
  return genericHead_x_y_c_d_b(OZ_arity, OZ_args, OZ_self, BIfdCDSchedControl_body,
			     fd_bounds);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfdCDSchedControl_body, 5)
{
  BIfdBodyManager a(3); //number of FDs
  
  for (int i = 2; i--; )
    a.introduce(i, OZ_getCArg(i));
  a.introduce(2, OZ_getCArg(4));
  
  const int x = 0, y = 1, control = 2;
  
  int xd = smallIntValue(deref(OZ_getCArg(2)));
  int yd = smallIntValue(deref(OZ_getCArg(3)));
  
  int xl = a[x].minElem(), xu = a[x].maxElem();
  int yl = a[y].minElem(), yu = a[y].maxElem();
  
  if (xu + xd <= yl) {
    FailOnEmpty(a[control] &= 0);
    return a.entailment();
  }
  if (yu + yd <= xl) {
    FailOnEmpty(a[control] &= 1);
    return a.entailment();
  }
  
  if (xl + xd > yu){
    FailOnEmpty(a[control] &= 1);
    a.process(control);
    killPropagatedCurrentTaskSusp();
    return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(y),
					 OZ_getCArg(x),
					 newSmallInt(-yd)));
  }
    
  if (yl + yd > xu){
    FailOnEmpty(a[control] &= 0);
    a.process(control);
    killPropagatedCurrentTaskSusp();
    return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(x),
					 OZ_getCArg(y),
					 newSmallInt(-xd)));
  }

  if (a[control] == fd_singleton) {
    if (a[control].singl() == 0) {
      killPropagatedCurrentTaskSusp();
      return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(x),
					   OZ_getCArg(y),
					   newSmallInt(-xd)));
    }
    else {
      killPropagatedCurrentTaskSusp();
      return BIfdLessEqOff(3, allocateRegs(OZ_getCArg(y),
					   OZ_getCArg(x),
					   newSmallInt(-yd)));
    }
  }

  
  LocalFD la, lb, lc, ld, l1, l2;

  la.init(0, yu - xd);
  lb.init(yl + yd, fd_sup);
  lc.init(0, xu - yd);
  ld.init(xl + xd, fd_sup);

  l1 = (la | lb);
  l2 = (lc | ld);
  FailOnEmpty(a[x] &= l1);
  FailOnEmpty(a[y] &= l2);
    
  return a.release();
}
OZ_C_proc_end
