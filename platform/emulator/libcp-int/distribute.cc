
#include "var_base.hh"
#include "builtins.hh"
#include "GeIntVar.hh"
#include "distributor.hh"


// Define a builtin operation that tells a constraint in the gecode space.
OZ_BI_proto(BIGfdTellConstraint);

TaggedRef BI_DistributeTell = 0;

void gfd_dist_init(void) {
  printf(">>>tell builtin initialization\n");fflush(stdout);
  BI_DistributeTell = makeTaggedConst(new Builtin("GFD", "distribute (tell)",
						  2, 0, BIGfdTellConstraint, OK));
}


// Insert a thread to make a tell.
inline
void tell_dom2(Board *bb, TaggedRef var, TaggedRef val) {
  printf(">>>Inside tell_dom  \n");fflush(stdout);
  bb->clearStatus();
  bb->ensureLateThread();
  Thread * lt = bb->getLateThread();
  Assert(lt);
  printf(">>>Inside tell_dom thread: %p \n",lt);fflush(stdout);
  lt->printTaskStack(1000);
  lt->pushCall(BI_DistributeTell,RefsArray::make(var,val));
  printf(">>>Inside tell_dom after add tell \n");fflush(stdout);
  lt->printTaskStack(1000);
  
  //oz_newThreadInject(bb)->pushCall(BI_DistributeTell,RefsArray::make(var,val));
}



class GFDDistributor : public Distributor {
protected:
  /**
     \brief Sinchronization variable. This variable is bound when
     the distributor work is finished. 
   */
  TaggedRef sync;

  /// Vector of variables to distribute
  TaggedRef *vars;

  /// Number of variables to distribute
  int size;

  /// Position of the next variable to distribute
  int sel_var;

  /// Selected value for the next distribution
  TaggedRef sel_val;

public:

  /**
     \brief Creates a distributor object for a variable vector \a vs of size n
   */
  GFDDistributor(Board *bb, TaggedRef *vs, int n) {
    printf("constructor\n");fflush(stdout);
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
    sel_var = -1;
    sel_val = (TaggedRef) 0;
  }
  
  TaggedRef getSync() { return sync; }
  /**
     \brief Commits branching description \a bd in board bb.
   */
  virtual int commitBranch(Board *bb, TaggedRef bd) {
    printf("commit branch: %s\n",OZ_toC(bd,10,10));fflush(stdout);

    // This assumes bd to be in the form: Pos#Value or Pos#compl(Value)
    Assert(OZ_isTuple(bd));
    int pos = OZ_intToC(OZ_getArg(bd,0));
    
    printf("CommitBranch: possition to commit: %d\n",pos);fflush(stdout);

    Assert(0 <= pos && pos < size);
    TaggedRef value = OZ_getArg(bd,1);
    //DEREF(val,val_ptr);
    //printf("CommitBranch: val to commit: %s\n",OZ_toC(value,10,10));fflush(stdout);

    //printf(">>>Starting tell Var : %s val: %s\n", OZ_toC(vars[pos],10,10), OZ_toC(value,10,10));fflush(stdout);
    
    tell_dom2(bb, vars[pos], value);

    printf("CommitBranch: completed\n");fflush(stdout);
    /* Assume the only method able to communicate the sapce to remove the dist is
       notifyStable. This can be optimized further.
    */
    return 1;
  }

  virtual int notifyStable(Board *bb) {
    printf("notifyStable called size: %d\n",size);fflush(stdout);
    // Ok, we are stable so it is time to select variable and values and set a new branch
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
	// printf(">>>Found int or non var pos: %d val: %s\n", i, OZ_toC(vars[i],100,100));fflush(stdout);
      } else if(OZ_isGeIntVar(vars[i])) {
	// printf(">>>Found Var pos: %d val: %s\n", i, OZ_toC(vars[i],100,100));fflush(stdout);
	break;
      } else {
	//printf(">>>Found Nothing pos: %d val: %s\n", i, OZ_toC(vars[i],100,100));fflush(stdout);
	Assert(false);
      }
    }
    //printf(">>>End selection  Var pos: %d val: %s\n", i, OZ_toC(vars[i],100,100));fflush(stdout);


    if (i == size) {
      printf("finished\n");fflush(stdout);
      (void) oz_unify(sync,AtomNil);
      // there are no more variables to distribute, we shall return.
      return 0;
    }

    sel_var = i;
    
    Assert(!OZ_isInt(vars[sel_var]));

    // choose the minimum of the domain as the next value to distribute
    int val = get_IntVarInfo(vars[sel_var]).min();
    printf("selected val\n");fflush(stdout);

    // create the new branch for the space.
    // first possible branching: sel_var#val
    TaggedRef fb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),OZ_int(val));
    // second possible branching: sel_var#compl(val)
    TaggedRef sb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),
			      OZ_mkTuple(OZ_atom("compl"),1,OZ_int(val)));

    TaggedRef bd = OZ_cons(fb,OZ_cons(sb,OZ_nil()));
			  
    printf("notifyStable setBranch %s\n",OZ_toC(bd,100,100));fflush(stdout);
    bb->setBranching(bd);
    printf("finished\n");fflush(stdout);
    return 1;
  }

  virtual Distributor * gCollect(void) {
    GFDDistributor * t = (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_gCollectTerm(t->sync);
    OZ_gCollectTerm(t->sel_val);
    t->vars = OZ_gCollectAllocBlock(size, t->vars);
    return t;
  }
  
  virtual Distributor * sClone(void) {
    GFDDistributor * t = (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_sCloneTerm(t->sync);
    OZ_sCloneTerm(t->sel_val);
    t->vars = OZ_sCloneAllocBlock(size, t->vars);
    return t;
  }

};




OZ_BI_define(BIGfdTellConstraint, 2, 0)
{

  printf(">>>Running tell thread\n");fflush(stdout);


  Board *bb = oz_currentBoard();
  Assert(bb->getGenericSpace(true));

  // Get the variable
  TaggedRef var = OZ_in(0);
  Assert(OZ_isGeIntVar(var));

  // Get the value
  TaggedRef val = OZ_in(1);
  
  // Post the real constraint in the gecode space
  if (OZ_isTuple(val)) {
    // case compl(v)
    printf(">>>Inside tell_thread compl(v)  Var: %s val: %s\n", 
	   OZ_toC(var,10,10),OZ_toC(val,10,10));fflush(stdout);

    Assert(OZ_width(val) == 1);
    Gecode::rel(bb->getGenericSpace(true),
		get_IntVar(var), Gecode::IRT_NQ, OZ_intToC(OZ_getArg(val,0)));
  } else {
    printf(">>>Inside tell_thread v  Var: %s val: %s\n", 
	   OZ_toC(var,10,10),OZ_toC(val,10,10));fflush(stdout);
    Assert(OZ_isInt(val));
    Gecode::rel(bb->getGenericSpace(true),
		get_IntVar(var), Gecode::IRT_EQ, OZ_intToC(val));
  }
  // TODO: can i fail in advance?
  return PROCEED;
}
OZ_BI_end
  
  
OZ_BI_define(gfd_distribute, 1, 1) {
  oz_declareNonvarIN(0,vv);


  printf("called gfd_distribute\n");fflush(stdout);
  int n = 0;
  TaggedRef * vars;

  // Assume vv is a tuple (list) of gfd variables
  Assert(oz_isTuple(vv));
  TaggedRef vs = vv;
  while (oz_isLTuple(vs)) {
    TaggedRef v = oz_head(vs);
    //TestElement(v);
    n++;
    vs = oz_tail(vs);
    DEREF(vs, vs_ptr);
    Assert(!oz_isRef(vs));
    if (oz_isVarOrRef(vs))
      oz_suspendOnPtr(vs_ptr);
  }

  // If there are no variables in the input then return unit
  if (n == 0)
    OZ_RETURN(NameUnit);
  
  // This is inverse order!
  vars = (TaggedRef *) oz_freeListMalloc(sizeof(TaggedRef) * n);
  
  // fill in the vars vector 
  Assert(!oz_isRef(vv));
  if (oz_isLTupleOrRef(vv)) {
    TaggedRef vs = vv;
    int i = n;
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      //if (!oz_isSmallInt(oz_deref(v)))
      vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  printf("creating real object\n");fflush(stdout);
  GFDDistributor * gfdd = new GFDDistributor(bb,vars,n);
  printf("associating it to the board.\n");fflush(stdout);
  bb->setDistributor(gfdd);

  OZ_RETURN(gfdd->getSync());
  
}
OZ_BI_end
