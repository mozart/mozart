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

#include "builtins.hh"
#include "var_base.hh"
#include "var_fd.hh"
#include "var_bool.hh"
#include "distributor.hh"
#include "thr_int.hh"

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


inline
static int discard_determined(SRecord * vec) {
  // Discard all elements which are small ints already
  register int j = 0;
  register int w = vec->getWidth();

  for (int i = 0; i<w; i++) {
    TaggedRef var   = vec->getArg(i);
    TaggedRef d_var = oz_deref(var);

    if (!oz_isSmallInt(d_var)) {
      Assert(isGenFDVar(d_var) || isGenBoolVar(d_var));
      vec->setArg(j++, var);
    }
    
  }

  if (j)
    vec->downSize(j);

  return j;
}


OZ_BI_define(BIfdd_selVarNaive, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  OZ_RETURN(vec->getArg(0));
} OZ_BI_end


OZ_BI_define(BIfdd_selVarSize, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  
  TaggedRef var = vec->getArg(0);

  int minsize = getSize(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int cursize = getSize(d_arg);

    if (cursize < minsize) {
      minsize = cursize;
      var     = arg; 
    }
      
  } 

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarMax, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var = vec->getArg(0);
  int maxmax    = getMax(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curmax = getMax(d_arg);

    if (curmax > maxmax) {
      maxmax = curmax;
      var    = arg; 
    }
      
  }

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarMin, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var = vec->getArg(0);
  int minmin    = getMin(oz_deref(var));

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curmin = getMin(d_arg);

    if (curmin < minmin) {
      minmin = curmin;
      var    = arg; 
    }
      
  }

  OZ_RETURN(var);
} OZ_BI_end


OZ_BI_define(BIfdd_selVarNbSusps, 1, 1) {
  oz_declareSTupleIN(0, vec);

  int w = discard_determined(vec);

  // No elements left
  if (!w)
    return OZ_raise(makeTaggedSmallInt(-1));

  TaggedRef var   = vec->getArg(0);
  TaggedRef d_var = oz_deref(var);

  int minsize = getSize(d_var);
  int maxcon  = getConstraints(d_var);

  for (int i=1; i<w; i++) {
    TaggedRef arg   = vec->getArg(i);
    TaggedRef d_arg = oz_deref(arg);

    int curcon  = getConstraints(d_arg);

    if (curcon < maxcon)
      continue;

    if (curcon==maxcon) {
      int cursize = getSize(d_arg);

      if (cursize < minsize) {
	minsize = cursize;
      } else {
	continue;
      }
    }

    maxcon = curcon;
    var    = arg; 
    
  }

  OZ_RETURN(var);
} OZ_BI_end


#define NEW_DISTRIBUTORS

#ifdef NEW_DISTRIBUTORS

/*
 * New distributors
 *
 *
 */

OZ_C_proc_proto(BIfdTellConstraint);

TaggedRef BI_tell = makeTaggedConst(new  Builtin("FD.distributeTELL", 2, 0, 
						 BIfdTellConstraint, OK));

inline
void tell_dom(Board * bb, const TaggedRef a, const TaggedRef b) {
  RefsArray args = allocateRefsArray(2, NO);
  args[0] = b;
  args[1] = a;

  Thread * t = oz_newThreadInject(bb);
  t->pushCall(BI_tell,args,2);
}

class FdDistributor : public Distributor {
protected:
  TaggedRef sync;
  int sel_var;
  TaggedRef values[2];
  TaggedRef * vars;
  int size;
public:

  FdDistributor::FdDistributor(Board *bb, TaggedRef * vs, int n) {
    vars = vs;
    size = n;
    sync = oz_newVar(bb);
  }

  TaggedRef getSync() {
    return sync;
  }

  void normalize(void);
  void selectVarBySize(void);
  void selectValMin(void);

  virtual int getAlternatives(void);

  virtual int commit(Board * bb, int l, int r) {
    if (size > 0) {
      if (l==r) {
	tell_dom(bb,vars[sel_var],values[l-1]);
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

  virtual Distributor * gc(void) {
    FdDistributor * t = (FdDistributor *) 
      oz_hrealloc(this, sizeof(FdDistributor));
    OZ_collectHeapTerm(t->sync,t->sync);
    OZ_collectHeapBlock(t->values,t->values,2);
    t->vars = OZ_copyOzTerms(size, t->vars);
    return t;
  }

};


inline
void FdDistributor::normalize(void) {
  // Discard all elements which are already bound
  int j = size;

  for (int i = size; i--;) {
    if (!oz_isSmallInt(oz_deref(vars[i])))
      vars[--j]=vars[i];
  }
  
  vars += j;
  size -= j; 

}

inline
void FdDistributor::selectVarBySize(void) {
  Assert(size > 0);
  int minsize = getSize(oz_deref(vars[size-1]));

  sel_var = size-1;

  for (int i = size-1; i--; ) {
    int cursize = getSize(oz_deref(vars[i]));
    
    if (cursize < minsize) {
      minsize = cursize;
      sel_var = i; 
    }
  }
  
}

inline
void FdDistributor::selectValMin(void) {
  Assert(size > 0);

  int n = getMin(oz_deref(vars[sel_var]));

  values[0] = makeTaggedSmallInt(n);
  SRecord * st = SRecord::newSRecord(AtomCompl, 1);
  st->setArg(0, makeTaggedSmallInt(n));
  values[1] = makeTaggedSRecord(st);
    
}

int FdDistributor::getAlternatives(void) {
  normalize();
  if (size > 0) {
    selectVarBySize();
    selectValMin();
    return 2;
  } else {
    return 1;
  }
}

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
	

OZ_BI_define(fdd_test_ff, 1, 1) {
  oz_declareNonvarIN(0,vv);

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
  
    FdDistributor * fdd = new FdDistributor(bb, vars, n);
    
    bb->addToDistBag(fdd);
    
    OZ_RETURN(fdd->getSync());
  }
  
 bomb:
  oz_typeError(0,"vector of finite domains");
}
OZ_BI_end



#endif


