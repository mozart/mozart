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
#include "IntVarMacros.hh"


/** 
 * \brief Declares a Gecode::IntSet(Const int[]) from an Oz domain description
 * 
 * @param _t Mozart domain specification 
 * @param ds The domain description in terms of list
 * 
 */
#define DECLARE_INT_SET21(_t,ds)					\
	OZ_Term _l = _t; \
  int _length = OZ_width(_l); \
  int _lst[_length];													\
	for (int i = 0; i < _length; i++) {				 \
		_lst[i] = OZ_intToC(OZ_getArg(_l, i));  \
	}	\
  Gecode::IntSet ds(_lst, _length);
  
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
	
	/// Initial position of the domain of the next variable to distribute
	int ini_pos_val;

	/// Final position of the domain of the next variable to distribute
	int fin_pos_val;

public:

  GFDDistributor(Board *bb, TaggedRef *vs, int n) {
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
    sel_var = -1;
    ini_pos_val = 0;
		fin_pos_val = 0;
  }

	//Returns the next variable to distribute
  //virtual int select_var() = 0;
	//Set the position of domain of the next variable to distibute
	//virtual void set_pos_val() = 0;
  
	//The distributor work finished
	//Bound the variable sync. 
  virtual void finish() {
    (void) oz_unify(sync,AtomNil);
    dispose();
  }

	//This method makes the branches according the subdomains defined to 'pos_val'
	//first branch ->  [ vars[sel_var][ini_pos_val], vars[sel_var][fin_pos_val] ]
	//second branch -> [ the values that do not belong to first branch ]
  virtual void make_branch(Board *bb){
		//Definition of branches
		int _size = (fin_pos_val - ini_pos_val)+1;
		OZ_Term b1 = OZ_tuple(OZ_atom("compl"), _size);
		int sb1 = 0;
		//printf("1\n");fflush(stdout);
		OZ_Term b2 = OZ_tuple(OZ_atom("compl"), get_IntVarInfo(vars[sel_var]).size() - _size);
		int sb2 = 0;
		//printf("2\n");fflush(stdout);
		IntVar Domain = get_IntVarInfo(vars[sel_var]);
		IntVarRanges DomainRange(Domain);
		
		//This loop is neccesary to obtain the entire domain including 
		//domains defined into parts
		//i.e [0#3 5#7]  -> [0 1 2 3 5 6 7]
		int i = 0;
			for(;DomainRange();++DomainRange){
				for(int j=DomainRange.min();j<=DomainRange.max();j++,i++){
				if ((i >= ini_pos_val) && (i <= fin_pos_val)){
						OZ_putArg(b1,sb1,OZ_int(j));
						sb1++;
					}
					else{
						OZ_putArg(b2,sb2,OZ_int(j));
						sb2++;
					}
			}
		}

		// first possible branching
    TaggedRef fb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),b1);
		// second possible branching
		TaggedRef sb = OZ_mkTuple(OZ_atom("#"),2,OZ_int(sel_var),b2);
		
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
	
	
	int selectNaiveVar(void);
	int selectSizeVar(void);
	int selectMinVar(void);
	int selectMaxVar(void);
	int selectWidthVar(void);
	int selectNbPropVar(void);
	
	void selectPosValMin(void);
	void selectPosValMax(void);
	void selectPosValMid(void);
	void selectPosValSplitMin(void);
	void selectPosValSplitMax(void);

  virtual Distributor * gCollect(void) {
    GFDDistributor * t = (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_gCollectTerm(t->sync);
    t->vars = OZ_gCollectAllocBlock(size, t->vars);
    return t;
  }
  
  virtual Distributor * sClone(void) {
    GFDDistributor * t = (GFDDistributor *) oz_hrealloc(this, sizeof(GFDDistributor));
    OZ_sCloneTerm(t->sync);
    t->vars = OZ_sCloneAllocBlock(size, t->vars);
    return t;
  }
};



inline
int GFDDistributor::selectNaiveVar(void){
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

inline
int GFDDistributor::selectSizeVar(void){
	int index = -1;
	int domain;
	int i = 0;
	for (i=0; i<size;i++) {
		if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
		} else if(OZ_isGeIntVar(vars[i])) {
			//Actual domain
			//printf("3\n");fflush(stdout);
			int d = get_IntVarInfo(vars[i]).size();
			if (index == -1){
				//that is the first variable
				index = i;
				domain = d;
			}
			else{//The actual domain is compared to before domain (chooses the domain size is minimal)
				if (d < domain){
					index = i;
					domain = d;
				}
			}
		} else {
			Assert(false);
		}
	}
	return index;
}

inline
int GFDDistributor::selectMinVar(void){
		int index = -1;
		int domain;
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
      } else if(OZ_isGeIntVar(vars[i])) {
				//Actual domain
				//printf("3\n");fflush(stdout);
				int d = get_IntVarInfo(vars[i]).min();
				if (index == -1){
					//that is the first variable
					index = i;
					domain = d;
				}
				else{//The actual domain is compared to before domain (chooses the domain size is minimal)
					if (d < domain){
						index = i;
						domain = d;
					}
				}
      } else {
				Assert(false);
      }
    }
		return index;
}

inline
int GFDDistributor::selectMaxVar(void){
		int index = -1;
		int domain;
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
      } else if(OZ_isGeIntVar(vars[i])) {
				//Actual domain
				//printf("4\n");fflush(stdout);
				int d = get_IntVarInfo(vars[i]).max();
				if (index == -1){
					//that is the first variable
					index = i;
					domain = d;
				}
				else{//The actual domain is compared to before domain (chooses the domain size is maximal)
					if (d > domain){
						index = i;
						domain = d;
					}
				}
      } else {
				Assert(false);
      }
    }
		return index;
}


inline
int GFDDistributor::selectWidthVar(void){
	int index = -1;
	int domain;
	int i = 0;
	for (i=0; i<size;i++) {
		if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
		} else if(OZ_isGeIntVar(vars[i])) {
			//Actual domain
			//printf("3\n");fflush(stdout);
			int d = get_IntVarInfo(vars[i]).width();
			if (index == -1){
				//that is the first variable
				index = i;
				domain = d;
			}
			else{//The actual domain is compared to before domain (chooses the domain size is minimal)
				if (d < domain){
					index = i;
					domain = d;
				}
			}
		} else {
			Assert(false);
		}
	}
	return index;
}



inline
int GFDDistributor::selectNbPropVar(void){
	//skip
}



inline
void GFDDistributor::selectPosValMin(void){
	ini_pos_val = 0;
	fin_pos_val = 0;
}

inline
void GFDDistributor::selectPosValMax(void){
	ini_pos_val = get_IntVarInfo(vars[sel_var]).size()-2;
	fin_pos_val = ini_pos_val;
}

inline
void GFDDistributor::selectPosValMid(void){
	int s = get_IntVarInfo(vars[sel_var]).size();
	ini_pos_val = s/2+s%2-1;
	fin_pos_val = ini_pos_val;
}


inline
void GFDDistributor::selectPosValSplitMin(void){
	int s = get_IntVarInfo(vars[sel_var]).size();
	ini_pos_val = 0;
	fin_pos_val = s/2+s%2-1;
}

inline
void GFDDistributor::selectPosValSplitMax(void){
	int s = get_IntVarInfo(vars[sel_var]).size();
	ini_pos_val = s/2+s%2;
	fin_pos_val = s-1;
}




#define DefGFDDistClass(CLASS,VARSEL,VALSEL)				\
class CLASS : public GFDDistributor {               \
 public:																						\
	  CLASS(Board * b, TaggedRef * v, int s) :        \
	    GFDDistributor(b,v,s) {}                      \
																										\
			virtual int notifyStable(Board *bb){					\
				sel_var = VARSEL();													\
				if (sel_var == -1)	{												\
					finish();																	\
					return 0;																	\
				}																						\
																										\
				Assert(!OZ_isInt(vars[sel_var]));						\
																										\
				VALSEL();																		\
																										\
				make_branch(bb);														\
																										\
				return 1;																		\
			}																							\
}


DefGFDDistClass(GFDDistNaiveMin,selectNaiveVar,selectPosValMin);
DefGFDDistClass(GFDDistNaiveMax,selectNaiveVar,selectPosValMax);
DefGFDDistClass(GFDDistNaiveMid,selectNaiveVar,selectPosValMid);
DefGFDDistClass(GFDDistNaiveSplitMin,selectNaiveVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistNaiveSplitMax,selectNaiveVar,selectPosValSplitMax);

DefGFDDistClass(GFDDistSizeMax,selectSizeVar,selectPosValMax);
DefGFDDistClass(GFDDistSizeMin,selectSizeVar,selectPosValMin);
DefGFDDistClass(GFDDistSizeMid,selectSizeVar,selectPosValMid);
DefGFDDistClass(GFDDistSizeSplitMin,selectSizeVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistSizeSplitMax,selectSizeVar,selectPosValSplitMax);

DefGFDDistClass(GFDDistMinMin,selectMinVar,selectPosValMin);
DefGFDDistClass(GFDDistMinMax,selectMinVar,selectPosValMax);
DefGFDDistClass(GFDDistMinMid,selectMinVar,selectPosValMid);
DefGFDDistClass(GFDDistMinSplitMin,selectMinVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistMinSplitMax,selectMinVar,selectPosValSplitMax);

DefGFDDistClass(GFDDistWidthMin,selectWidthVar,selectPosValMin);
DefGFDDistClass(GFDDistWidthMax,selectWidthVar,selectPosValMax);
DefGFDDistClass(GFDDistWidthMid,selectWidthVar,selectPosValMid);
DefGFDDistClass(GFDDistWidthSplitMin,selectWidthVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistWidthSplitMax,selectWidthVar,selectPosValSplitMax);

DefGFDDistClass(GFDDistMaxMin,selectMaxVar,selectPosValMin);
DefGFDDistClass(GFDDistMaxMax,selectMaxVar,selectPosValMax);
DefGFDDistClass(GFDDistMaxMid,selectMaxVar,selectPosValMid);
DefGFDDistClass(GFDDistMaxSplitMin,selectMaxVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistMaxSplitMax,selectMaxVar,selectPosValSplitMax);

DefGFDDistClass(GFDDistNbPropMin,selectNbPropVar,selectPosValMin);
DefGFDDistClass(GFDDistNbPropMax,selectNbPropVar,selectPosValMax);
DefGFDDistClass(GFDDistNbPropMid,selectNbPropVar,selectPosValMid);
DefGFDDistClass(GFDDistNbPropSplitMin,selectNbPropVar,selectPosValSplitMin);
DefGFDDistClass(GFDDistNbPropSplitMax,selectNbPropVar,selectPosValSplitMax);


#define iVarNaive   0
#define iVarSize    1
#define iVarMin     2
#define iVarMax     3
#define iVarNbProp  4
#define iVarWidth   5

#define iValMin      0
#define iValMid      1
#define iValMax      2
#define iValSplitMin 3
#define iValSplitMax 4
	
#define PP(I,J) I*(iVarWidth+1)+J
	
#define PPCL(I,J)                                  \
	  case PP(iVar ## I,iVal ## J):                    \
	    gfdd = new GFDDist ## I ## J(bb, vars, n); \
	    break;


class GFDNaiveDistributor: public GFDDistributor{
public:
	
  /**
     \brief Creates a distributor object for a variable vector \a vs
     of size n
  */
  GFDNaiveDistributor(Board *bb, TaggedRef *vs, int n)
    : GFDDistributor(bb,vs,n) { printf("Naive\n");fflush(stdout); }
  
	//Chooses the leftmost non determined variable on vars
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
	
	//Chooses the position of the lower bound of the domain of sel_var
  virtual void set_pos_val(){
		ini_pos_val = 0;
		fin_pos_val = 0;
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

    set_pos_val();

    /*
      After variable and value selections a branching must be created
      for the space.  This will allow Space.ask to be notified about
      the possibles branching descritpions that can be commited to
      this distributor.
			For this purpose get_IntVarInfo is used to not generate gecode space unstability.
    */
    make_branch(bb);
    // This distributor should be preserved.
    return 1;
  }
};

class GFDFFDistributor: public GFDDistributor{
public:
	
  /**
     \brief Creates a distributor object for a variable vector \a vs
     of size n
  */
  GFDFFDistributor(Board *bb, TaggedRef *vs, int n)
    : GFDDistributor(bb,vs,n) { printf("First Fail\n");fflush(stdout); }
  
	//Chooses the leftmost variable whose domain size is minimal.
  virtual int select_var(){
		int index = -1;
		int domain;
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
      } else if(OZ_isGeIntVar(vars[i])) {
				//Actual domain
				int d = get_IntVarInfo(vars[i]).size();
				if (index == -1){
					//that is the first variable
					index = i;
					domain = d;
				}
				else{//The actual domain is compared to before domain (chooses the domain size is minimal)
					if (d < domain){
						index = i;
						domain = d;
					}
				}
      } else {
				Assert(false);
      }
    }

    return index;
  }
	
	//Chooses the position of the lower bound of the domain of sel_var
	virtual void set_pos_val(){
		ini_pos_val = 0;
		fin_pos_val = 0;
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

    set_pos_val();

    /*
      After variable and value selections a branching must be created
      for the space.  This will allow Space.ask to be notified about
      the possibles branching descritpions that can be commited to
      this distributor.
			For this purpose get_IntVarInfo is used to not generate gecode space unstability.
    */
    make_branch(bb);
    // This distributor should be preserved.
    return 1;
  }
};

class GFDSplitDistributor: public GFDDistributor{
public:
	
  /**
     \brief Creates a distributor object for a variable vector \a vs
     of size n
  */
  GFDSplitDistributor(Board *bb, TaggedRef *vs, int n)
    : GFDDistributor(bb,vs,n) { printf("Split\n");fflush(stdout); }
		
		
 	//Chooses the leftmost variable whose domain size is minimal.
  virtual int select_var(){
		int index = -1;
		int domain;
    int i = 0;
    for (i=0; i<size;i++) {
      if (OZ_isInt(vars[i]) || !OZ_isGeIntVar(vars[i]) ) {
      } else if(OZ_isGeIntVar(vars[i])) {
				//Actual domain
				int d = get_IntVarInfo(vars[i]).size();
				if (index == -1){
					//that is the first variable
					index = i;
					domain = d;
				}
				else{//The actual domain is compared to before domain (chooses the domain size is minimal)
					if (d < domain){
						index = i;
						domain = d;
					}
				}
      } else {
				Assert(false);
      }
    }

    return index;
  }

	//Chooses the middle position of the domain of sel_var
	virtual void set_pos_val(){
		int s = get_IntVarInfo(vars[sel_var]).size();
		ini_pos_val = s/2+s%2-1;
		fin_pos_val = ini_pos_val;
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

    set_pos_val();
    
		/*
      After variable and value selections a branching must be created
      for the space.  This will allow Space.ask to be notified about
      the possibles branching descritpions that can be commited to
      this distributor.
			For this purpose get_IntVarInfo is used to not generate gecode space unstability.
    */
    make_branch(bb);
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
	Board *bb = oz_currentBoard();
  Assert(bb->getGenericSpace(true));

  // Get the variable
  TaggedRef var = OZ_in(0);
  Assert(OZ_isGeIntVar(var));

  // Get the value
  TaggedRef val = OZ_in(1);
	assert(OZ_isTuple(val));
	
	//This macro is used to define a IntSet variable
	//to makes the branch
	DECLARE_INT_SET21(val,ds);
	//printf("Tell Constraint\n");fflush(stdout);
	Gecode::dom(bb->getGenericSpace(true),
			get_IntVar(var), ds);
	
	// TODO: can i fail in advance?
	return PROCEED;
}
OZ_BI_end
  
  
OZ_BI_define(gfd_distribute, 3, 1) {
	oz_declareIntIN(0,var_sel);
	oz_declareIntIN(1,val_sel);
  oz_declareNonvarIN(2,vv);

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
      vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  }

  Board * bb = oz_currentBoard();
  
  if (bb->getDistributor())
    return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);
  
  //make the decision
  GFDDistributor * gfdd;
	
	  switch (PP(var_sel,val_sel)) {
	      PPCL(Naive,Min);
	      PPCL(Naive,Max);
				PPCL(Naive,Mid);
				PPCL(Naive,SplitMin);
				PPCL(Naive,SplitMax);
	
	      PPCL(Size,Min);
	      PPCL(Size,Max);
				PPCL(Size,Mid);
				PPCL(Size,SplitMin);
				PPCL(Size,SplitMax);
				
	      PPCL(Min,Min);
	      PPCL(Min,Max);
				PPCL(Min,Mid);
				PPCL(Min,SplitMin);
				PPCL(Min,SplitMax);
				
				PPCL(Max,Min);
	      PPCL(Max,Max);
				PPCL(Max,Mid);
				PPCL(Max,SplitMin);
				PPCL(Max,SplitMax);
				
				PPCL(NbProp,Min);
	      PPCL(NbProp,Max);
				PPCL(NbProp,Mid);
				PPCL(NbProp,SplitMin);
				PPCL(NbProp,SplitMax);
				
				default:
					Assert(false);
			}
	
  //printf("associating it to the board.\n");fflush(stdout);
  bb->setDistributor(gfdd);
  
  OZ_RETURN(gfdd->getSync()); 
}
OZ_BI_end
