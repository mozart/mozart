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



#include "gecode/int/branch.hh"

// Define a builtin operation that tells a constraint in the gecode space.
OZ_BI_proto(BIGfdTellConstraint);

TaggedRef BI_DistributeTell = 0;

void gfd_dist_init(void) {
  BI_DistributeTell = 
    makeTaggedConst(new Builtin("GFD", "distribute (tell)",
				2, 0, BIGfdTellConstraint, OK));
}

class ValMin {
public:
  int val(const GenericSpace*, IntView x) {
    return x.min();
  }

  Gecode::ModEvent tell(GenericSpace *home, unsigned int a, IntView x, int v) {
    printf("Called tell ValMin\n");fflush(stdout);
    return (a == 0) ?  x.eq(home,v) : x.gr(home,v);
    /*
    if (a == 1) {
      // case compl(v)
      Gecode::rel(home,x,Gecode::IRT_NQ, v);
    } else {
      Assert(a == 0);
      Gecode::rel(home,x,Gecode::IRT_EQ, v);
    }
    // TODO: can i fail in advance?
    printf("Called tell ValMin - finished\n");fflush(stdout);
   
    return PROCEED;
    */
  }
};
class ByNone {
public:
  Gecode::ViewSelStatus init(const GenericSpace*, IntView x) {
    return VSS_COMMIT;
  }
  Gecode::ViewSelStatus select(const GenericSpace*, IntView x) {
    GECODE_NEVER;
    return VSS_NONE;
  }
};

/*
  \brief This class encapsulates the operations that concern to
  specific constraint system variables. For the distributor we need to
  know when consider an OZ_Term to be a bound representation of the
  specific constraint variable, for instance an integer is considered
  a bound FD variable. Also we need a way to create a view from an
  OZ_Term to be able to use gecode provided view selection strategies.
 */
class IntVarBasics {
public:
  static IntView getView(OZ_Term t) {
    Assert(!assigned(t));
    return IntView(get_IntVarInfo(t).var());
  }
  static bool assigned(OZ_Term t) {
    return OZ_isInt(t);
  }
};

template <class ValSel, class ViewSel>
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

#ifdef DEBUG_CHECK
  Board *home;
  GenericSpace *gs_home;
#endif
  //ViewArray<IntView> xv;

public:

  GFDDistributor(Board *bb, TaggedRef *vs, int n) {
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
   
#ifdef DEBUG_CHECK
    home = bb;
    gs_home = bb->getGenericSpace(true);
    Assert(gs_home);
#endif
    
    /*
    ViewArray<IntView> tv(bb->getGenericSpace(true), size); 

    for (int i=0; i<size; i++) {
      IntView v(get_IntVarInfo(vars[i]).var()); 
      tv[i]=v;
    }
    xv=tv;
    */
  }

  virtual void finish() {
    (void) oz_unify(sync,AtomNil);
    dispose();
  }

  virtual void make_branch(Board *bb, int pos, int val){
    // first possible branching: sel_var#val
    TaggedRef fb = 
      OZ_mkTuple(OZ_atom("#"),2,OZ_int(pos),OZ_int(val));
    
    // second possible branching: sel_var#compl(val)
    TaggedRef sb = 
      OZ_mkTuple(OZ_atom("#"),2,OZ_int(pos),
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
     
    bb->prepareTell(BI_DistributeTell,
		    RefsArray::make(OZ_int(pos),value));
    
    /* 
       Assumes the only method able to tell the sapce to remove the
       distributor is notifyStable. This can be optimized further.
    */
    return 1;
  }

 virtual int notifyStable(Board *bb) {
   printf("Called tell notifyStable\n");fflush(stdout);
    /*
      The space is stable and now it is safe to select a new variable
      with a new value for the next distribution step.
    */
   GenericSpace *gs = bb->getGenericSpace(true);
   Assert(gs);
   /*
     TODO: must be this board the same one in which this distributor
     was created?. If it must then it would be better to put an
     assertion about gs == gs_home. Answer: No, that is not true for
     all the cases, because bb can be a clone of the space in which
     the distributor was created and distributor cloning reuses the
     same object with diferent variables
   */

   ViewSel vs; // For view selection
   ValSel  vl; // Fr value selection
   
   int i = 0;
   int j = 0;
   for (j = 0; j < size && IntVarBasics::assigned(vars[j]); j++);
   i = j;
   int b = i++;
   
   if (j == size) goto finished;

   Assert(j==size || !IntVarBasics::assigned(vars[b]));
   

   //printf("size before selction %d\n",xv.size());fflush(stdout);

   if (vs.init(gs, IntVarBasics::getView(vars[b])) != VSS_COMMIT)
     for (; i < size; i++){
       if (!IntVarBasics::assigned(vars[i]))
	 switch (vs.select(gs,IntVarBasics::getView(vars[i]))) {
	 case VSS_SELECT: 
	   b=i; 
	   printf("VSS_SELECT at iteration %d\n",i);
	   fflush(stdout);
	   break;
	 case VSS_COMMIT: b=i; 
	   printf("VSS_COMMIT at iteration %d -> going to create\n",i);
	   fflush(stdout);
	   goto create;
	 case VSS_NONE:
	   printf("VSS_NONE at iteration %d -> going to finished\n",i);
	   fflush(stdout);
	   break;
	 default:         GECODE_NEVER;
	 }
     }
   
 create:
   {
     /*
       After variable and value selections a branching must be created
       for the space.  This will allow Space.ask to be notified about
       the possibles branching descritpions that can be commited to
       this distributor.
     */
     IntView vb = IntVarBasics::getView(vars[b]);
     Assert(!vb.assigned());
     make_branch(bb, b, vl.val(gs, vb));
   }

 finished:
   if (status()) {
     // This distributor should be preserved.
     return 1;
   } else {
     printf("Called notifyStable almost finish\n");fflush(stdout);
     finish();
     /*
       There are no more variables to distribute in the array.  This
       distributor must be removed from the board.
     */
     return 0;
   }
 }
  
  bool status(void) {
    for (int i=0; i < size; i++)
      if (oz_isGeVar(vars[i])) {
        //start = i;
        return true;
      }
    return false;
  }

  /*
    To access gecode space's variable use get_IntVar instead of
    get_IntVarInfo to make the space unstable. Space unstability is
    fine because after a tell the space will be unstable.
  */
  virtual OZ_Return tell(RefsArray *args) {
    printf("Called tell\n");fflush(stdout);
    int p = OZ_intToC(args->getArg(0));
    TaggedRef val = args->getArg(1);

    Assert(OZ_isGeIntVar(vars[p]));
    GenericSpace *gs = oz_currentBoard()->getGenericSpace(true);

    /*
      TODO: should be this board the same one in which this
      distributor was created?. If it should then it would be better
      to put an assertion about gs == gs_home
     */

    ValSel vs;

    unsigned int a = OZ_isTuple(val) ? 1 : 0;    
    int v = (a == 1) ? 
      OZ_intToC(OZ_getArg(val,0)) : OZ_intToC(val);

#ifdef DEBUG_CHEK
    if (OZ_isTuple(val)) 
      // case compl(v)
      Assert(OZ_width(val) == 1);
    else 
      Assert(OZ_isInt(val));
#endif

  
    // Post the real constraint in the gecode space
    printf("Tell constraint, variable at pos: %d with val %s\n",p,OZ_toC(vars[p],100,100));fflush(stdout);
    Assert(!IntVarBasics::assigned(vars[p]));
    vs.tell(gs,a,get_IntVar(vars[p]),v);

    /* (ggutierrez) Unstability should be an effect of a tell
       operation on the gecode space. I'm not sure about
       Assert(!gs->isStable());
     */

    printf("Called tell - finished\n");fflush(stdout);

    /* 
       TODO: Gecode ViewSel classes return a ModEvent with possibly
       useful information. We can convert this to something
       meaningfull to mozart and maybe fail the board in advance.
     */
    return PROCEED;
  }

  virtual Distributor * gCollect(void) {
    GFDDistributor * t = 
      (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_gCollectTerm(t->sync);
    t->vars = OZ_gCollectAllocBlock(size, t->vars);
    return t;
  }
  
  virtual Distributor * sClone(void) {
    GFDDistributor * t = 
      (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_sCloneTerm(t->sync);
    t->vars = OZ_sCloneAllocBlock(size, t->vars);
    return t;
  }
};

/*
  This builtin performs the tell operation. We need a builtin because
  the tell is generated by the search engine and at that time the
  board installed may not correspond to the board of the tell
  operation. When the built in is executed both, the board installed
  and the target board are the same.
*/
OZ_BI_define(BIGfdTellConstraint, 2, 0)
{

  printf(">>>Running tell thread\n");fflush(stdout);

  Board *bb = oz_currentBoard();
  Assert(bb->getGenericSpace(true));
  
  return bb->getDistributor()->tell(RefsArray::make(OZ_in(0),OZ_in(1)));  
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
    for (int i =0; i < n; i++) {
      TaggedRef v = oz_head(vs);
      vars[i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  /*
    // An example
  GFDDistributor<Gecode::Int::Branch::ValMed<IntView>, 
    Gecode::Int::Branch::ByMinMax<IntView> > * gfdd =
    
    new GFDDistributor<Gecode::Int::Branch::ValMed<IntView>, 
    Gecode::Int::Branch::ByMinMax<IntView> >(bb,vars,n);
  */
  
   GFDDistributor<Gecode::Int::Branch::ValMin<IntView>, 
    Gecode::Int::Branch::ByDegreeMin<IntView> > * gfdd =
    
    new GFDDistributor<Gecode::Int::Branch::ValMin<IntView>, 
    Gecode::Int::Branch::ByDegreeMin<IntView> >(bb,vars,n);

  printf("Created distributor associating it to the board.\n");fflush(stdout);
  bb->setDistributor(gfdd);
  
  OZ_RETURN(gfdd->getSync()); 
}
OZ_BI_end


template <class ValSel, class ViewSel>
class GFDAssignment : public Distributor {
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

#ifdef DEBUG_CHECK
  Board *home;
  GenericSpace *gs_home;
#endif
  //ViewArray<IntView> xv;

public:

  GFDAssignment(Board *bb, TaggedRef *vs, int n) {
    printf("Called assign constructor\n");fflush(stdout);
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
   
#ifdef DEBUG_CHECK
    home = bb;
    gs_home = bb->getGenericSpace(true);
    Assert(gs_home);
#endif
    
  }

  virtual void finish() {
    (void) oz_unify(sync,AtomNil);
    dispose();
  }
  
  virtual void make_branch(Board *bb, int pos, int val){
    // first possible branching: sel_var#val
    TaggedRef fb = 
      OZ_mkTuple(OZ_atom("#"),2,OZ_int(pos),OZ_int(val));
    
    // second possible branching: sel_var#compl(val)
    TaggedRef sb = 
      OZ_mkTuple(OZ_atom("#"),2,OZ_int(pos),
		 OZ_mkTuple(OZ_atom("compl"),1,OZ_int(val)));
    
    bb->setBranching(OZ_cons(fb,OZ_cons(sb,OZ_nil())));
  }

  /*
    \brief As the distributor injects tell operations in the board
    that not take place inmediately, termination is notified by
    binding \a sync to an atom.
  */
  TaggedRef getSync() { return sync; }
  
  
  void dispose(void) { oz_freeListDispose(this, sizeof(GFDAssignment)); }

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
     
    bb->prepareTell(BI_DistributeTell,
		    RefsArray::make(OZ_int(pos),value));
    
    /* 
       Assumes the only method able to tell the sapce to remove the
       distributor is notifyStable. This can be optimized further.
    */
    return 1;
  }

 virtual int notifyStable(Board *bb) {
   printf("Called tell notifyStable\n");fflush(stdout);
    /*
      The space is stable and now it is safe to select a new variable
      with a new value for the next distribution step.
    */
   GenericSpace *gs = bb->getGenericSpace(true);
   Assert(gs);
   /*
     TODO: must be this board the same one in which this distributor
     was created?. If it must then it would be better to put an
     assertion about gs == gs_home. Answer: No, that is not true for
     all the cases, because bb can be a clone of the space in which
     the distributor was created and distributor cloning reuses the
     same object with diferent variables
   */

   //   ViewSel vs; // For view selection
   ValSel  vl; // Fr value selection
   
   int i = 0;
   int j = 0;
   for (j = 0; j < size && IntVarBasics::assigned(vars[j]); j++);
   i = j;
   int b = i++;
   
   if (j == size) goto finished;

   Assert(j==size || !IntVarBasics::assigned(vars[b]));
   
   /*
     After variable and value selections a branching must be created
     for the space.  This will allow Space.ask to be notified about
     the possibles branching descritpions that can be commited to
     this distributor.
   */
   {
     IntView vb = IntVarBasics::getView(vars[b]);
     Assert(!vb.assigned());
     make_branch(bb, b, vl.val(gs, vb));
   }
 finished:
   if (status()) {
     // This distributor should be preserved.
     return 1;
   } else {
     printf("Called notifyStable almost finish\n");fflush(stdout);
     finish();
     /*
       There are no more variables to distribute in the array.  This
       distributor must be removed from the board.
     */
     return 0;
   }
 }
  
  bool status(void) {
    for (int i=0; i < size; i++)
      if (oz_isGeVar(vars[i])) {
        //start = i;
        return true;
      }
    return false;
  }

  /*
    To access gecode space's variable use get_IntVar instead of
    get_IntVarInfo to make the space unstable. Space unstability is
    fine because after a tell the space will be unstable.
  */
  virtual OZ_Return tell(RefsArray *args) {
    printf("Called tell\n");fflush(stdout);
    int p = OZ_intToC(args->getArg(0));
    TaggedRef val = args->getArg(1);

    Assert(OZ_isGeIntVar(vars[p]));
    GenericSpace *gs = oz_currentBoard()->getGenericSpace(true);

    /*
      TODO: should be this board the same one in which this
      distributor was created?. If it should then it would be better
      to put an assertion about gs == gs_home
     */

    ValSel vs;

    unsigned int a = OZ_isTuple(val) ? 1 : 0;    
    int v = (a == 1) ? 
      OZ_intToC(OZ_getArg(val,0)) : OZ_intToC(val);

#ifdef DEBUG_CHEK
    if (OZ_isTuple(val)) 
      // case compl(v)
      Assert(OZ_width(val) == 1);
    else 
      Assert(OZ_isInt(val));
#endif
    
    // Post the real constraint in the gecode space
    printf("Tell constraint, variable at pos: %d with val %s\n",p,OZ_toC(vars[p],100,100));fflush(stdout);
    Assert(!IntVarBasics::assigned(vars[p]));
    vs.tell(gs,a,get_IntVar(vars[p]),v);

    /* (ggutierrez) Unstability should be an effect of a tell
       operation on the gecode space. I'm not sure about
       Assert(!gs->isStable());
     */

    printf("Called tell - finished\n");fflush(stdout);

    /* 
       TODO: Gecode ViewSel classes return a ModEvent with possibly
       useful information. We can convert this to something
       meaningfull to mozart and maybe fail the board in advance.
     */
    return PROCEED;
  }

  virtual Distributor * gCollect(void) {
    GFDAssignment * t = 
      (GFDAssignment *) oz_hrealloc(this, sizeof(GFDAssignment));
    OZ_gCollectTerm(t->sync);
    t->vars = OZ_gCollectAllocBlock(size, t->vars);
    return t;
  }
  
  virtual Distributor * sClone(void) {
    GFDAssignment * t = 
      (GFDAssignment *) oz_hrealloc(this, sizeof(GFDAssignment));
    OZ_sCloneTerm(t->sync);
    t->vars = OZ_sCloneAllocBlock(size, t->vars);
    return t;
  }
};

  
OZ_BI_define(gfd_assign, 1, 1) {
  printf("called gfd_assign\n");fflush(stdout);
  
  oz_declareNonvarIN(0,vv);

  printf("called gfd_assign\n");fflush(stdout);
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
    for (int i =0; i < n; i++) {
      TaggedRef v = oz_head(vs);
      vars[i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  /*
    // An example
  GFDDistributor<Gecode::Int::Branch::ValMed<IntView>, 
    Gecode::Int::Branch::ByMinMax<IntView> > * gfdd =
    
    new GFDDistributor<Gecode::Int::Branch::ValMed<IntView>, 
    Gecode::Int::Branch::ByMinMax<IntView> >(bb,vars,n);
  */
  
   GFDAssignment<Gecode::Int::Branch::ValMin<IntView>, 
    Gecode::Int::Branch::ByNone<IntView> > * gfda =
    
     new GFDAssignment<Gecode::Int::Branch::ValMin<IntView>, 
    Gecode::Int::Branch::ByNone<IntView> >(bb,vars,n);

   printf("Created assignment associating it to the board.\n");fflush(stdout);
   bb->setDistributor(gfda);
  
  OZ_RETURN(gfda->getSync()); 
}
OZ_BI_end
