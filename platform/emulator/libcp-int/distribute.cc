
#include "var_base.hh"
#include "GeIntVar.hh"
#include "distributor.hh"


// Define a builtin operation that tells a constraint in the gecode space.
OZ_BI_proto(BIGfdTellConstraint);

TaggedRef BI_DistributeTell;

void gfd_dist_init(void) {
  BI_DistributeTell = makeTaggedConst(new Builtin("GFD", "distribute (tell)", 2, 0,
						  BIGfdTellConstraint, OK));
}


// Insert a thread to make a tell.
inline
void tell_dom(Board *bb, TaggedRef var, TaggedRef val) {
  oz_newThreadInject(bb)->pushCall(BI_DistributeTell,RefsArray::make(var,val));
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
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
    sel_var = -1;
    sel_val = (TaggedRef) 0;
  }
  
  /**
     \brief Commits branching description \a bd in board bb.
   */
  virtual int commitBranch(Board *bb, TaggedRef bd) {
    // This assumes bd to be in the form: Pos#Val or Pos#compl(Val)
    Assert(OZ_isTuple(bd));
    int pos = OZ_intToC(OZ_getArg(bd,0));
    Assert(pos != -1);
    TaggedRef val = OZ_getArg(bd,1);
    Assert(val != (TaggedRef)0);
    tell_dom(bb, vars[pos], val);

    /* Assume the only method able to communicate the sapce to remove the dist is
       notifyStable. This can be optimized further.
    */
    return 1;
  }

  virtual int notifyStable(Board *bb) {
    // Ok, we are stable so it is time to select variable and values and set a new branch
    int nxt = 0;
    while (nxt < size && !get_IntVarInfo(vars[nxt]).assigned()) nxt++;

    if (nxt == size) {
      // there are no more variables to distribute, we shall return.
      return 0;
    }

    sel_var = nxt;
    
    // choose the minimum of the domain as the next value to distribute
    int val = get_IntVarInfo(vars[sel_var]).min();

    // create the new branch for the space.
    TaggedRef bd = OZ_cons(OZ_int(val),
			   OZ_cons(OZ_mkTuple(OZ_atom("compl"),1,OZ_int(val)), OZ_nil()));
    bb->setBranching(bd);
    return 1;
  }

};




OZ_BI_define(BIGfdTellConstraint, 2, 0)
{
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
    Assert(OZ_width(val) == 1);
    Gecode::rel(bb->getGenericSpace(true),
		get_IntVar(var), Gecode::IRT_NQ, OZ_intToC(OZ_getArg(val,0)));
  } else {
    Assert(OZ_isInt(val));
    Gecode::rel(bb->getGenericSpace(true),
		get_IntVar(var), Gecode::IRT_EQ, OZ_intToC(val));
  }
  // TODO: can i fail in advance?
  return PROCEED;
}
OZ_BI_end
  
  
