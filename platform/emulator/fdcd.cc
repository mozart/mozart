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
  ExpectedTypes("tuple of small int,tuple of finite domain,"
		"tuple of tuple var,tuple of var");
  
  OZ_getCArgDeref(0, p_tuple, p_tupleptr, p_tupletag);
  OZ_getCArgDeref(1, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(2, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(3, vp_tuple, vp_tupleptr, vp_tupletag);

  if (! isSTuple(p_tuple)) TypeError(0, "");
  if (! isSTuple(b_tuple)) TypeError(1, "");

  SRecord &p = *tagged2SRecord(p_tuple);
  SRecord &b = *tagged2SRecord(b_tuple);

  const int clauses = p.getWidth();
  if (clauses != b.getWidth()) {
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
      fdvar->getDom().init(0, OZ_intToC(p[i]) + 2);
      doBind(bi_ptr, makeTaggedRef(newTaggedCVar(fdvar)));
    } else {
      error("Unexpected CVar found.");
    }
  } 

  // Has already been reduced to sum(b) >= 1
  if (isLiteral(v_tupletag)) return PROCEED;

  if (! isSTuple(v_tuple)) TypeError(2, "");
  if (! isSTuple(vp_tuple)) TypeError(3, "");

  SRecord &v = *tagged2SRecord(v_tuple);
  SRecord &vp = *tagged2SRecord(vp_tuple);

  const int variables = v.getWidth();

  // constrain Vpij to {fd_inf..fd_sup} if Vpij is an uvar
  for (i = clauses; i--; ) {
    DEREF(vp[i], vp_i_ptr, vp_i_tag);
    if (! isSTuple(vp[i])) TypeError(3, "2-dim-array expected");
    SRecord &vp_i = *tagged2SRecord(vp[i]);
    
    if (vp_i.getWidth() != variables) {
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

//  by kost@ 9.04.96: usage of fdaux.hh in fdcd.cc is eliminated;
//#include "../FDLib/fdaux.hh"

class CDPropagator : public OZ_Propagator {
protected:
  OZ_Term b_tuple, v_tuple, vp_tuple;
public:
  CDPropagator(OZ_Term b, OZ_Term v, OZ_Term vp) 
    : b_tuple(b), v_tuple(v), vp_tuple(vp) {}

  virtual void gcRecurse(void) {
    OZ_gcTerm(b_tuple);
    OZ_gcTerm(v_tuple);
    OZ_gcTerm(vp_tuple);
  }
  virtual size_t sizeOf(void) { return sizeof(CDPropagator); }
  virtual OZ_Return run(void);
  virtual ostream &print(ostream& o) const {
    return o << "cd manager";
  }
};

OZ_C_proc_begin(BIfdConstrDisj, 3)
{ 
  EXPECTED_TYPE("tuple of finite domain,tuple of finite domain,"
		"tuple of of tuple of finite domain");
  
  OZ_getCArgDeref(0, b_tuple, b_tupleptr, b_tupletag);
  OZ_getCArgDeref(1, v_tuple, v_tupleptr, v_tupletag);
  OZ_getCArgDeref(2, vp_tuple, vp_tupleptr, vp_tupletag);

  if (isLiteral(v_tupletag)) {
    SRecord &b = *tagged2SRecord(b_tuple);
    int b_size = b.getWidth();
    int ones = 0;

    for (int i = 0; i < b_size; i++) {
      OZ_Term b_val = deref(b[i]);
      Assert(isSmallInt(b_val)); 
      if (smallIntValue(b_val) == 1) ones += 1;
    }
    return ones > 0 ? PROCEED : FAILED;
  }


  if (! isSTuple(b_tuple) || ! isSTuple(v_tuple) || 
      ! isSTuple(vp_tuple)) {
    warning("Unexpected type in cd manager");
    return FAILED;
  }

  OZ_PropagatorExpect pe;
  EXPECT(pe, 1, expectVectorIntVarAny);

  return pe.spawn(new CDPropagator(OZ_args[0], OZ_args[1], OZ_args[2]),
		  OZMAX_PRIORITY - 1);
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

OZ_Return CDPropagator::run(void)
{
  DEREF(b_tuple, b_tupleptr, b_tupletag);
  DEREF(v_tuple, v_tupleptr, v_tupletag);
  DEREF(vp_tuple, vp_tupleptr, vp_tupletag);
  
  SRecord &_b = *tagged2SRecord(b_tuple);
  SRecord &_v = *tagged2SRecord(v_tuple);
  SRecord &_vp = *tagged2SRecord(vp_tuple);

  const int clauses = _b.getWidth(); // number of clauses
  const int variables = _v.getWidth(); // number ob variables
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
      SRecord &vp_c = *tagged2SRecord(deref(_vp[c]));
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

#ifndef TM_LP
  Assert(localPropStore.isEmpty());
  localPropStore.setUseIt();
#endif

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

    for (v = variables; v--; )
      x.process(idx_vp(c, v));

    x.backup();
    if (!localPropStore.do_propagation()) 
      error("local propagation must be empty");
    x.restore();

    Assert(localPropStore.isEmpty());
  
  }

#ifndef TM_LP
  localPropStore.unsetUseIt();
#endif

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
      SRecord &vp_c = *tagged2SRecord(deref(_vp[c]));
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
      OZ_FiniteDomain u;
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

//-----------------------------------------------------------------------------
// Built-ins

OZ_Return cd_wrapper_b(int OZ_arity, OZ_Term OZ_args[], 
		       OZ_CFun, OZ_CFun BI_body)
{
  int last_index = OZ_arity - 1;
  
  BIfdBodyManager x;
  
  x.introduce(OZ_getCArg(last_index));

  Assert(x[0] != 1); // clause cannot already be entailed
  
  if (x[0] == 0) {
    return EntailFD;
  }

  x.backup();
  OZ_Return ret_val = BI_body(last_index, OZ_args);
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
// Propagators

#undef FailOnEmpty
#undef SimplifyOnUnify

#include "../FDLib/cd.hh"

CDSuppl::CDSuppl(OZ_Propagator * p, OZ_Term b) : reg_b(b) 
{
  thr = (OZ_Thread) new Thread(am.currentBoard, OZ_getPropagatorPrio() + 1, p);
}

void CDSuppl::gcRecurse(void) {
  thr = (OZ_Thread) ((Thread *)thr)->gcThread();
  OZ_gcTerm(reg_b);
}

ostream& CDSuppl::print(ostream& o) const 
{
  return o << '(' << *((Thread *)thr)->getPropagator() 
    << ") ><" <<OZ_toStream(reg_b);
}

OZ_Return CDSuppl::run(void)
{
  OZ_FDIntVar b(reg_b);
  PropagatorController_V P(b);

  if (*b == 0) {
    ((Thread *) thr)->closeDonePropagatorCD();	
    return PROCEED;
  } 

  if (*b == 1) {
    OZ_Propagator * p = ((Thread *) thr)->swapPropagator(this);
    ((Thread *) thr)->closeDonePropagatorThreadCD();	
    return replaceBy(p);
  }

  Thread * backup_currentThread = am.currentThread;
  am.currentThread = (Thread *) thr;
  OZ_Return ret_val = ((Thread *) thr)->runPropagator();
  am.currentThread = backup_currentThread;

  OZ_ASSERT(b->maxElem() >= 2);

  if (ret_val == PROCEED) {
    ((Thread *) thr)->closeDonePropagatorCD();	
    *b <= (b->maxElem() - 1);
    OZ_ASSERT(b->maxElem() >= 2);
  } else if (ret_val == FAILED) {
    ((Thread *) thr)->closeDonePropagatorCD();	
    *b &= 0;
  }

  P.vanish();
  return ret_val == FAILED ? PROCEED : ret_val;
}

