/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1998
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

#include "distribute.hh"
#include "builtins.hh"
#include "var_base.hh"
#include "var_fd.hh"
#include "var_bool.hh"
#include "distributor.hh"
#include "thr_int.hh"
#include "fdomn.hh"

// ---------------------------------------------------------------------
//                  Finite Domains Distribution Built-ins
// ---------------------------------------------------------------------

inline
int getSize(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getSize() : 2;
}

inline
int getFdWidth(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getWidth() : 1;
}

inline 
int getMin(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMinElem() : 0;
}

inline
int getMax(TaggedRef var) {
  return isGenFDVar(var) ? tagged2GenFDVar(var)->getDom().getMaxElem() : 1;
}

inline
int getMid(TaggedRef var) {
  if (isGenFDVar(var)) {
    OZ_FiniteDomain &dom = tagged2GenFDVar(var)->getDom();
    return dom.getMidElem();
  } else {
    return 0;
  }
}

inline
int getConstraints(TaggedRef var) {
  return oz_var_getSuspListLength(tagged2Var(var));
}


OZ_BI_proto(BIfdTellConstraint);

TaggedRef BI_DistributeTell;


void fd_dist_init(void) {
  BI_DistributeTell = makeTaggedConst(new 
				      Builtin("FD", "distribute (tell)",2,0, 
					      BIfdTellConstraint, OK));
}

inline
void tell_dom(Board * bb, const TaggedRef a, const TaggedRef b) {
  oz_newThreadInject(bb)->pushCall(BI_DistributeTell,RefsArray::make(b,a));
}

inline
void tell_eq(Board * bb, const TaggedRef a, const TaggedRef b) {
  oz_newThreadInject(bb)->pushCall(BI_Unify,RefsArray::make(a,b));
}


class FdDistributor : public Distributor {
protected:
  TaggedRef sync;
  int sel_var;
  TaggedRef sel_val;
  TaggedRef * vars;
  int size;
public:

  FdDistributor(Board *bb, TaggedRef * vs, int n) {
    vars = vs;
    size = n;
    sync = oz_newVariable(bb);
    DebugCode(sel_var = -1);
    sel_val = (TaggedRef) 0;
  }

  void dispose(void) {
    //    oz_freeListDispose(this, sizeof(FdDistributor));
  }

  TaggedRef getSync(void) {
    return sync;
  }

  void selectVarNaive(void);
  void selectVarSize(void);
  void selectVarWidth(void);
  void selectVarMin(void);
  void selectVarMax(void);
  void selectVarNbSusps(void);

  void selectValMin(void);
  void selectValMid(void);
  void selectValMax(void);
  void selectValSplitMin(void);
  void selectValSplitMax(void);

  virtual int commit(Board * bb, int n) {
    if (n > 2)
      return -2;

    if (size > 0) {
      TaggedRef dom;
      if (n == 1) {
	dom = sel_val;
      } else {
	SRecord * st = SRecord::newSRecord(AtomCompl, 1);
	st->setArg(0, sel_val);
	dom = makeTaggedSRecord(st);
      }
      Assert(sel_var != -1);
      tell_dom(bb,vars[sel_var],dom);
      return 1;
    } else {
      tell_dom(bb,sync,makeTaggedSmallInt(0));
      dispose();
      return 0;
    }
  }

  virtual Distributor * gCollect(void) {
    FdDistributor * t = (FdDistributor *) 
      oz_hrealloc(this, sizeof(FdDistributor));
    OZ_gCollectTerm(t->sync);
    OZ_gCollectTerm(t->sel_val);
    t->vars = OZ_gCollectAllocBlock(size, t->vars);
    return t;
  }

  virtual Distributor * sClone(void) {
    FdDistributor * t = (FdDistributor *) 
      oz_hrealloc(this, sizeof(FdDistributor));
    OZ_sCloneTerm(t->sync);
    OZ_sCloneTerm(t->sel_val);
    t->vars = OZ_sCloneAllocBlock(size, t->vars);
    return t;
  }

};


class FdAssigner : public FdDistributor {
public:
  FdAssigner(Board * b, TaggedRef * v, int s) :
    FdDistributor(b,v,s) {}

  virtual int commit(Board * bb, int n) {
    if (size > 0) {
      Assert(sel_var != -1);
      tell_eq(bb,vars[sel_var],sel_val);
      return 1;
    } else {
      tell_eq(bb,sync,makeTaggedSmallInt(0));
      dispose();
      return 0;
    }
  }

};



/*
 * Variable selection strategies
 *
 */

#define ITERATOR(INIT,UPDATE) \
  int i = size, j = size;                            \
  TaggedRef vd;                                      \
  while (i--) {                                      \
    vd = oz_deref(vars[i]);                          \
    if (!oz_isSmallInt(vd)) break;                   \
  }                                                  \
  if (i < 0) {                                       \
    size = 0; return;                                \
  }                                                  \
  vars[--j] = vars[i];                               \
  INIT;                                              \
  sel_var = j;                                       \
  while (i--) {                                      \
    vd = oz_deref(vars[i]);                          \
    if (oz_isSmallInt(vd)) continue;                 \
    vars[--j] = vars[i];                             \
    UPDATE;                                          \
  }                                                  \
  if (j > 0) {                                       \
    oz_freeListDisposeUnsafe(vars, j * sizeof(TaggedRef)); \
    vars    += j;                                    \
    size    -= j;                                    \
    sel_var -= j;                                    \
  }
    

inline
void FdDistributor::selectVarNaive(void) {
  ITERATOR({},{});
}

inline
void FdDistributor::selectVarSize(void) {
  ITERATOR(int minsize = getSize(vd), 
	   int cursize = getSize(vd);
	   if (cursize < minsize) {
	     minsize = cursize; sel_var = j;
	   });
}

inline
void FdDistributor::selectVarWidth(void) {
  ITERATOR(int minwidth = getFdWidth(vd), 
	   int curwidth = getFdWidth(vd);
	   if (curwidth < minwidth) {
	     minwidth = curwidth; sel_var = j;
	   });
}

inline
void FdDistributor::selectVarMin(void) {
  ITERATOR(int minmin = getMin(vd),
	   int curmin = getMin(oz_deref(vars[i]));
	   if (curmin < minmin) {
	     minmin = curmin; sel_var = j; 
	   });
}

inline
void FdDistributor::selectVarMax(void) {
  ITERATOR(int maxmax = getMax(vd),
	   int curmax = getMax(oz_deref(vars[i]));
	   if (curmax > maxmax) {
	     maxmax = curmax; sel_var = j; 
	   });
}

inline
void FdDistributor::selectVarNbSusps(void) {
  ITERATOR(int minsize = getSize(vd);
	   int maxnb   = getConstraints(vd),
	   int curnb = getConstraints(vd);
	   if (curnb < maxnb)
	      continue;
	   int cursize = getSize(vd);
	   if (curnb > maxnb || cursize < minsize) {
	     maxnb   = curnb;
	     minsize = cursize; 
	     sel_var = j; 
	   });
}

#undef ITERATOR

/*
 * Value selection strategies
 *
 */

inline
void FdDistributor::selectValMin(void) {
  Assert(sel_var != -1);
  sel_val = makeTaggedSmallInt(getMin(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValMid(void) {
  Assert(sel_var != -1);
  sel_val = makeTaggedSmallInt(getMid(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValMax(void) {
  Assert(sel_var != -1);
  sel_val = makeTaggedSmallInt(getMax(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValSplitMin(void) {
  SRecord * st = SRecord::newSRecord(AtomPair, 2);
  st->setArg(0, makeTaggedSmallInt(0));
  Assert(sel_var != -1);
  st->setArg(1, makeTaggedSmallInt(getMid(oz_deref(vars[sel_var]))));
  sel_val = makeTaggedSRecord(st);
}

inline
void FdDistributor::selectValSplitMax(void) {
  SRecord * st = SRecord::newSRecord(AtomPair, 2);
  Assert(sel_var != -1);
  st->setArg(0, makeTaggedSmallInt(getMid(oz_deref(vars[sel_var])) + 1));
  st->setArg(1, makeTaggedSmallInt(fd_sup));
  sel_val = makeTaggedSRecord(st);
}



/*
 * Create class for different combinations
 */

#define DefFdDistClass(CLASS,VARSEL,VALSEL) \
class CLASS : public FdDistributor {              \
  public:                                         \
  CLASS(Board * b, TaggedRef * v, int s) :        \
    FdDistributor(b,v,s) {}                       \
  virtual int getAlternatives(void) {             \
    VARSEL();                                     \
    if (size > 0) { VALSEL(); return 2;           \
    } else {                  return 1;           \
    }                                             \
  }                                               \
}

DefFdDistClass(FdDist_Naive_Min,selectVarNaive,selectValMin);
DefFdDistClass(FdDist_Naive_Mid,selectVarNaive,selectValMid);
DefFdDistClass(FdDist_Naive_Max,selectVarNaive,selectValMax);
DefFdDistClass(FdDist_Naive_SplitMin,selectVarNaive,selectValSplitMin);
DefFdDistClass(FdDist_Naive_SplitMax,selectVarNaive,selectValSplitMax);

DefFdDistClass(FdDist_Size_Min,selectVarSize,selectValMin);
DefFdDistClass(FdDist_Size_Mid,selectVarSize,selectValMid);
DefFdDistClass(FdDist_Size_Max,selectVarSize,selectValMax);
DefFdDistClass(FdDist_Size_SplitMin,selectVarSize,selectValSplitMin);
DefFdDistClass(FdDist_Size_SplitMax,selectVarSize,selectValSplitMax);

DefFdDistClass(FdDist_Width_Min,selectVarWidth,selectValMin);
DefFdDistClass(FdDist_Width_Mid,selectVarWidth,selectValMid);
DefFdDistClass(FdDist_Width_Max,selectVarWidth,selectValMax);
DefFdDistClass(FdDist_Width_SplitMin,selectVarWidth,selectValSplitMin);
DefFdDistClass(FdDist_Width_SplitMax,selectVarWidth,selectValSplitMax);

DefFdDistClass(FdDist_Min_Min,selectVarMin,selectValMin);
DefFdDistClass(FdDist_Min_Mid,selectVarMin,selectValMid);
DefFdDistClass(FdDist_Min_Max,selectVarMin,selectValMax);
DefFdDistClass(FdDist_Min_SplitMin,selectVarMin,selectValSplitMin);
DefFdDistClass(FdDist_Min_SplitMax,selectVarMin,selectValSplitMax);

DefFdDistClass(FdDist_Max_Min,selectVarMax,selectValMin);
DefFdDistClass(FdDist_Max_Mid,selectVarMax,selectValMid);
DefFdDistClass(FdDist_Max_Max,selectVarMax,selectValMax);
DefFdDistClass(FdDist_Max_SplitMin,selectVarMax,selectValSplitMin);
DefFdDistClass(FdDist_Max_SplitMax,selectVarMax,selectValSplitMax);

DefFdDistClass(FdDist_NbSusps_Min,selectVarNbSusps,selectValMin);
DefFdDistClass(FdDist_NbSusps_Mid,selectVarNbSusps,selectValMid);
DefFdDistClass(FdDist_NbSusps_Max,selectVarNbSusps,selectValMax);
DefFdDistClass(FdDist_NbSusps_SplitMin,selectVarNbSusps,selectValSplitMin);
DefFdDistClass(FdDist_NbSusps_SplitMax,selectVarNbSusps,selectValSplitMax);

#define DefFdAssignClass(CLASS,VALSEL) \
class CLASS : public FdAssigner {                 \
  public:                                         \
  CLASS(Board * b, TaggedRef * v, int s) :        \
    FdAssigner(b,v,s) {}                          \
  virtual int getAlternatives(void) {             \
    selectVarNaive();                             \
    if (size > 0) { VALSEL(); }                   \
    return 1;                                     \
  }                                               \
}

DefFdAssignClass(FdAssign_Min,selectValMin);
DefFdAssignClass(FdAssign_Mid,selectValMid);
DefFdAssignClass(FdAssign_Max,selectValMax);


/*
 * Vector processing
 */

#define TestElement(v) \
  {                                         \
    DEREF(v, v_ptr);                        \
    Assert(!oz_isRef(v));		    \
    if (isGenFDVar(v) || isGenBoolVar(v)) { \
      n++;                                  \
    } else if (oz_isSmallInt(v)) {          \
      ;                                     \
    } else if (oz_isVarOrRef(v)) {          \
      oz_suspendOnPtr(v_ptr);               \
    } else {                                \
      goto bomb;                            \
    }                                       \
  }
	
#define iVarNaive   0
#define iVarSize    1
#define iVarMin     2
#define iVarMax     3
#define iVarNbSusps 4
#define iVarWidth   5

#define iValMin      0
#define iValMid      1
#define iValMax      2
#define iValSplitMin 3
#define iValSplitMax 4

#define PP(I,J) I*(iVarWidth+1)+J

#define PPCL(I,J)                                  \
  case PP(iVar ## I,iVal ## J):                    \
    fdd = new FdDist_ ## I ## _ ## J(bb, vars, n); \
    break;

OZ_BI_define(fdd_distribute, 3, 1) {
  oz_declareIntIN(0,var_sel);
  oz_declareIntIN(1,val_sel);
  oz_declareNonvarIN(2,vv);

  int n = 0;
  TaggedRef * vars;
  
  Assert(!oz_isRef(vv));
  if (oz_isLiteral(vv)) {
    ;
  } else if (oz_isLTupleOrRef(vv)) {
    
    TaggedRef vs = vv;

    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      TestElement(v);
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr);
      Assert(!oz_isRef(vs));
      if (oz_isVarOrRef(vs))
	oz_suspendOnPtr(vs_ptr);
    }
    
    if (!oz_isNil(vs))
      goto bomb;
    
  } else if (oz_isSRecord(vv)) {
    
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      TestElement(v);
    }
    
  } else 
    goto bomb;
  
  if (n == 0)
    OZ_RETURN(NameUnit);

  // This is inverse order!
  vars = (TaggedRef *) oz_freeListMalloc(sizeof(TaggedRef) * n);

  Assert(!oz_isRef(vv));
  if (oz_isLTupleOrRef(vv)) {
    TaggedRef vs = vv;
    int i = n;
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      if (!oz_isSmallInt(oz_deref(v)))
	vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  } else {
    int j = 0;
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      if (!oz_isSmallInt(oz_deref(v)))
	vars[j++] = v;
    }
  }

  if (oz_onToplevel())
    OZ_RETURN(oz_newVariable(oz_rootBoard()));

  {
    Board * bb = oz_currentBoard();

    if (bb->getDistributor())
      return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);

    FdDistributor * fdd;
    
    switch (PP(var_sel,val_sel)) {
      PPCL(Naive,Min);
      PPCL(Naive,Mid);
      PPCL(Naive,Max);
      PPCL(Naive,SplitMin);
      PPCL(Naive,SplitMax);

      PPCL(Size,Min);
      PPCL(Size,Mid);
      PPCL(Size,Max);
      PPCL(Size,SplitMin);
      PPCL(Size,SplitMax);

      PPCL(Min,Min);
      PPCL(Min,Mid);
      PPCL(Min,Max);
      PPCL(Min,SplitMin);
      PPCL(Min,SplitMax);

      PPCL(Max,Min);
      PPCL(Max,Mid);
      PPCL(Max,Max);
      PPCL(Max,SplitMin);
      PPCL(Max,SplitMax);

      PPCL(NbSusps,Min);
      PPCL(NbSusps,Mid);
      PPCL(NbSusps,Max);
      PPCL(NbSusps,SplitMin);
      PPCL(NbSusps,SplitMax);

      PPCL(Width,Min);
      PPCL(Width,Mid);
      PPCL(Width,Max);
      PPCL(Width,SplitMin);
      PPCL(Width,SplitMax);

    default:
      Assert(0);
    }
    bb->setDistributor(fdd);
    
    OZ_RETURN(fdd->getSync());
  }
  
 bomb:
  oz_typeError(0,"vector of finite domains");
}
OZ_BI_end


OZ_BI_define(fdd_assign, 2, 1) {
  oz_declareNonvarIN(0,val_sel);
  oz_declareNonvarIN(1,vv);

  int n = 0;
  TaggedRef * vars;
  
  Assert(!oz_isRef(vv));
  if (oz_isLiteral(vv)) {
    ;
  } else if (oz_isLTupleOrRef(vv)) {
    
    TaggedRef vs = vv;

    Assert(!oz_isRef(vs));
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      TestElement(v);
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr);
      Assert(!oz_isRef(vs));
      if (oz_isVarOrRef(vs))
	oz_suspendOnPtr(vs_ptr);
    }
    
    if (!oz_isNil(vs))
      goto bomb;
    
  } else if (oz_isSRecord(vv)) {
    
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      TestElement(v);
    }
    
  } else 
    goto bomb;
  
  if (n == 0)
    OZ_RETURN(NameUnit);

  // This is inverse order!
  vars = (TaggedRef *) oz_freeListMalloc(sizeof(TaggedRef) * n);

  Assert(!oz_isRef(vv));
  if (oz_isLTupleOrRef(vv)) {
    TaggedRef vs = vv;
    int i = n;
    while (oz_isLTuple(vs)) {
      TaggedRef v = oz_head(vs);
      if (!oz_isSmallInt(oz_deref(v)))
	vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
      Assert(!oz_isRef(vs));
    }
  } else {
    int j = 0;
    for (int i = tagged2SRecord(vv)->getWidth(); i--; ) {
      TaggedRef v = tagged2SRecord(vv)->getArg(i);
      if (!oz_isSmallInt(oz_deref(v)))
	vars[j++] = v;
    }
  }

  if (oz_onToplevel())
    OZ_RETURN(oz_newVariable(oz_rootBoard()));

  {
    Board * bb = oz_currentBoard();

    if (bb->getDistributor())
      return oz_raise(E_ERROR,E_KERNEL,"spaceDistributor", 0);

    FdDistributor * fdd;
    
    if (oz_eq(val_sel,AtomMin)) {
      fdd = new FdAssign_Min(bb, vars, n);
    } else if (oz_eq(val_sel,AtomMid)) {
      fdd = new FdAssign_Mid(bb, vars, n);
    } else if (oz_eq(val_sel,AtomMax)) {
      fdd = new FdAssign_Max(bb, vars, n);
    } else {
      oz_typeError(0,"min/mid/max");
    }

    bb->setDistributor(fdd);
    
    OZ_RETURN(fdd->getSync());
  }
  
 bomb:
  oz_typeError(1,"vector of finite domains");
}
OZ_BI_end

