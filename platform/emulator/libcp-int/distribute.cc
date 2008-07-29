/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Raphael Collet, 2008
 *    Gustavo Gutierrez, 2008
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "var_base.hh"
#include "builtins.hh"
#include "GeIntVar.hh"
#include "distributor.hh"


// Define a builtin operation that tells a constraint in the gecode space.
OZ_BI_proto(BIGfdTellConstraint);

TaggedRef BI_DistributeTell = 0;

void gfd_dist_init(void) {
  BI_DistributeTell = 
    makeTaggedConst(new Builtin("GFD", "distribute (tell)",
				2, 0, BIGfdTellConstraint, OK));
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

  GFDDistributor(Board *bb, TaggedRef *vs, int n) {
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
    sel_var = -1;
    sel_val = (TaggedRef) 0;
  }


  virtual int select_var() = 0;
  virtual int select_val() = 0;
  
  virtual void finish() {
    (void) oz_unify(sync,AtomNil);
    dispose();
  }

  virtual void make_branch(Board *bb,int val){
    // first possible branching: sel_var#val
    TaggedRef fb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),OZ_int(val));
    
    // second possible branching: sel_var#compl(val)
    TaggedRef sb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),
			      OZ_mkTuple(OZ_atom("compl"),1,OZ_int(val)));
    
    bb->setBranching(OZ_cons(fb,OZ_cons(sb,OZ_nil())));
  }

  /*
    \brief As the distributor injects tell operations in the board
    that not take place inmediately, termination is notified by
    binding \a sync to an atom.
  */
  TaggedRef getSync() { return sync; }
  
  
  void dispose(void) { oz_freeListDispose(this, sizeof(GFDDistributor)); }

  /**
     \brief Commits branching description \a bd in board bb. This
     operation is performed from the search engine. The prepareTell
     operation push the tell on the top of the thread that performs
     propagation. This allows all tell operations to be performed
     *before* the propagation of the gecode space. Also, as a side
     effect, tell operations are lazy, this is, are posted in the
     gecode space on space status demand.
  */
  virtual int commitBranch(Board *bb, TaggedRef bd) {
    
    // This assumes bd to be in the form: Pos#Value or Pos#compl(Value)
    Assert(OZ_isTuple(bd));
    int pos = OZ_intToC(OZ_getArg(bd,0));
    
    Assert(0 <= pos && pos < size);
    TaggedRef value = OZ_getArg(bd,1);
     
    bb->prepareTell(BI_DistributeTell,RefsArray::make(vars[pos],value));
 
    /* 
       Assumes the only method able to tell the sapce to remove the
       distributor is notifyStable. This can be optimized further.
    */
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

class GFDNaiveDistributor: public GFDDistributor{
public:
	
  /**
     \brief Creates a distributor object for a variable vector \a vs
     of size n
  */
  GFDNaiveDistributor(Board *bb, TaggedRef *vs, int n)
    : GFDDistributor(bb,vs,n) { }
  
  virtual int select_var(){
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
      } else if(OZ_isGeIntVar(vars[i])) {
	break;
      } else {
	Assert(false);
      }
    }

    if (i == size) {
      return -1;
    }
    return i;
  }
	
  virtual int select_val(){
    return get_IntVarInfo(vars[sel_var]).min();
  }
	
  virtual int notifyStable(Board *bb) {
    /*
      The space is stable and now it is safe to select a new variable
      with a new value for the next distribution step.
    */
    sel_var = select_var();
    if (sel_var == -1)	{
      finish();
      /*
	There are no more variables to distribute in the array.  This
	distributor must be removed from the board.
      */
      return 0;
    }

    Assert(!OZ_isInt(vars[sel_var]));

    /* 
       Value selection. For this purpose get_IntVarInfo is used to not
       generate gecode space unstability.
    */
    int val = select_val();

    /*
      After variable and value selections a branching must be created
      for the space.  This will allow Space.ask to be notified about
      the possibles branching descritpions that can be commited to
      this distributor.
    */
    make_branch(bb,val);
    // This distributor should be preserved.
    return 1;
  }
};


/*
  This builtin performs the tell operation. We need a builtin because
  the tell is generated by the search engine and in that place the
  board installed may not correspond to the board of the tell. When
  the built in is executed both, the board installed and the target
  board are the same.

  To access gecode space's variable use get_IntVar instead of
  get_IntVarInfo to make the space unstable. Space unstability is fine
  because after a tell the space will be unstable.
*/
OZ_BI_define(BIGfdTellConstraint, 2, 0)
{

  //printf(">>>Running tell thread\n");fflush(stdout);


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
  
  
OZ_BI_define(gfd_distribute, 1, 1) {
  oz_declareNonvarIN(0,vv);

  //printf("called gfd_distribute\n");fflush(stdout);
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
      vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  //make the decision
  GFDNaiveDistributor * gfdd = new GFDNaiveDistributor(bb,vars,n);
  //printf("associating it to the board.\n");fflush(stdout);
  bb->setDistributor(gfdd);
  
  OZ_RETURN(gfdd->getSync()); 
}
OZ_BI_end
