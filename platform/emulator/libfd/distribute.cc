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
  return oz_var_getSuspListLength(tagged2CVar(var));
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
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = b;
  args[1] = a;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_DistributeTell,args,2);
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
    sync = oz_newVar(bb);
  }

  TaggedRef getSync() {
    return sync;
  }

  void selectVarNaive(void);
  void selectVarSize(void);
  void selectVarMin(void);
  void selectVarMax(void);
  void selectVarNbSusps(void);

  void selectValMin(void);
  void selectValMid(void);
  void selectValMax(void);
  void selectValSplitMin(void);
  void selectValSplitMax(void);

  virtual int commit(Board * bb, int l, int r) {
    if (size > 0) {
      if (l==r) {
	TaggedRef dom;
	if (l == 1) {
	  dom = sel_val;
	} else {
	  SRecord * st = SRecord::newSRecord(AtomCompl, 1);
	  st->setArg(0, sel_val);
	  dom = makeTaggedSRecord(st);
	}
	tell_dom(bb,vars[sel_var],dom);
	return 1;
      } else {
	return 2;
      }
    } else {
      tell_dom(bb,sync,makeTaggedSmallInt(0));
      return 0;
    }
  }

  virtual void dispose(void) {
    freeListDispose(this, sizeof(FdDistributor));
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



/*
 * Variable selection strategies
 *
 */

#define ITERATOR(INIT,UPDATE) \
  int i = size, j = size;                         \
  TaggedRef vd;                                   \
  while (i--) {                                   \
    vd = oz_deref(vars[i]);                       \
    if (!oz_isSmallInt(vd)) break;                \
  }                                               \
  if (i < 0) {                                    \
    size = 0; return;                             \
  }                                               \
  vars[--j] = vars[i];                            \
  INIT;                                           \
  sel_var = j;                                    \
  while (i--) {                                   \
    vd = oz_deref(vars[i]);                       \
    if (oz_isSmallInt(vd)) continue;              \
    vars[--j] = vars[i];                          \
    UPDATE;                                       \
  }                                               \
  if (j > 0) {                                    \
    freeListDispose(vars, j * sizeof(TaggedRef)); \
    vars    += j;                                 \
    size    -= j;                                 \
    sel_var -= j;                                 \
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
  sel_val = makeTaggedSmallInt(getMin(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValMid(void) {
  sel_val = makeTaggedSmallInt(getMid(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValMax(void) {
  sel_val = makeTaggedSmallInt(getMax(oz_deref(vars[sel_var])));
}

inline
void FdDistributor::selectValSplitMin(void) {
  SRecord * st = SRecord::newSRecord(AtomPair, 2);
  st->setArg(0, makeTaggedSmallInt(0));
  st->setArg(1, makeTaggedSmallInt(getMid(oz_deref(vars[sel_var]))));
  sel_val = makeTaggedSRecord(st);
}

inline
void FdDistributor::selectValSplitMax(void) {
  SRecord * st = SRecord::newSRecord(AtomPair, 2);
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
  };                                              \
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



/*
 * Vector processing
 */

#define TestElement(v) \
  {                                         \
    DEREF(v, v_ptr, v_tag);                 \
    if (isGenFDVar(v) || isGenBoolVar(v)) { \
      n++;                                  \
    } else if (oz_isSmallInt(v)) {          \
      ;                                     \
    } else if (oz_isVariable(v)) {          \
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

#define iValMin      0
#define iValMid      1
#define iValMax      2
#define iValSplitMin 3
#define iValSplitMax 4

#define PP(I,J) I*5+J

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
  
  if (oz_isLiteral(vv)) {
    ;
  } else if (oz_isCons(vv)) {
    
    TaggedRef vs = vv;

    while (oz_isCons(vs)) {
      TaggedRef v = oz_head(vs);
      TestElement(v);
      vs = oz_tail(vs);
      DEREF(vs, vs_ptr, vs_tag);
      if (isVariableTag(vs_tag))
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
  vars = (TaggedRef *) freeListMalloc(sizeof(TaggedRef) * n);

  if (oz_isCons(vv)) {
    TaggedRef vs = vv;
    int i = n;
    while (oz_isCons(vs)) {
      TaggedRef v = oz_head(vs);
      if (!oz_isSmallInt(oz_deref(v)))
	vars[--i] = v;
      vs = oz_deref(oz_tail(vs));
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
    OZ_RETURN(oz_newVar(oz_rootBoard()));

  {
    Board * bb = oz_currentBoard();
  
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
    default:
      Assert(0);
    }
    bb->addToDistBag(fdd);
    
    OZ_RETURN(fdd->getSync());
  }
  
 bomb:
  oz_typeError(0,"vector of finite domains");
}
OZ_BI_end

