/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __GENFDVAR__H__
#define __GENFDVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "var_base.hh"
#include "fdomn.hh"

#ifdef OUTLINE 
#define inline
#endif

//-----------------------------------------------------------------------------
//                           class OzFDVariable
//-----------------------------------------------------------------------------

class OzFDVariable: public OzVariable {

friend class OzVariable;
friend class OzBoolVariable;
friend inline void addSuspFDVar(TaggedRef, Suspendable *, OZ_FDPropState);
friend OZ_Return oz_var_cast(TaggedRef *&, Board *, TypeOfVariable);
  
private:
  OZ_FiniteDomain finiteDomain;
  SuspList * fdSuspList[fd_prop_any];
  
  void relinkSuspListToItself(Bool reset_local = FALSE);

  OzBoolVariable * becomesBool(void);
public:  
  OzFDVariable(OZ_FiniteDomain &fd,Board *bb) : OzVariable(OZ_VAR_FD,bb) {
    ozstat.fdvarsCreated.incf();
    finiteDomain = fd;
    fdSuspList[fd_prop_singl] = fdSuspList[fd_prop_bounds] = NULL;
  }

  OzFDVariable(Board *bb) : OzVariable(OZ_VAR_FD,bb) {
    ozstat.fdvarsCreated.incf();
    finiteDomain.initFull();
    fdSuspList[fd_prop_singl] = fdSuspList[fd_prop_bounds] = NULL;
  }

  void gCollect(Board *); 
  void sClone(Board *); 

  inline void dispose(void);

  // methods for trailing
  OzVariable * copyForTrail(void);
  void restoreFromCopy(OzFDVariable *);
  
  void becomesSmallIntAndPropagate(TaggedRef * trPtr);
  void becomesBoolVarAndPropagate(TaggedRef * trPtr);

  int intersectWithBool(void);

  // is X=val still valid, i.e. is val a smallint and is it still in the domain
  Bool valid(TaggedRef val);

  void setDom(OZ_FiniteDomain &fd) {
    Assert(fd != fd_bool);
    finiteDomain = fd;
  }
  OZ_FiniteDomain &getDom(void) {return finiteDomain;}

  void relinkSuspListTo(OzFDVariable * lv, Bool reset_local = FALSE);
  void relinkSuspListTo(OzBoolVariable * lv, Bool reset_local = FALSE);

  void propagate(OZ_FDPropState state,
		 PropCaller prop_eq = pc_propagator);  

  void propagateUnify() {
    propagate(fd_prop_singl, pc_cv_unif);
  }

  int getSuspListLength(void) {
    int len = suspList->length();
    for (int i = fd_prop_any; i--; )
      len += fdSuspList[i]->length();
    return len;
  }

  SuspList * getSuspList(int i) { return fdSuspList[i]; }

  void installPropagators(OzFDVariable *);

  OZ_Return bind(TaggedRef *, TaggedRef);
  OZ_Return unify(TaggedRef *, TaggedRef *);

  void printStream(ostream &out,int depth = 10) {
    out << getDom().toString();
  }
  void printLongStream(ostream &out, int depth = 10, int offset = 0) {
    printStream(out,depth); out << endl;
  }
  //
  void dropPropagator(Propagator * prop) {
    for (int i = fd_prop_any; i--; ) {
      fdSuspList[i] = fdSuspList[i]->dropPropagator(prop);
    }
    suspList = suspList->dropPropagator(prop);
  }
  //
  // tagging and untagging constrained variables
  //
  OZ_FDIntVar * getTag(void) {
    return (OZ_FDIntVar *)  (u.var_type & ~u_mask);
  }
  //
  // end of tagging ...
  //
};

void addSuspFDVar(TaggedRef, Suspendable *, OZ_FDPropState = fd_prop_any);
OZ_Return tellBasicConstraint(OZ_Term, OZ_FiniteDomain *);

#ifndef OUTLINE 
#include "var_fd.icc"
#else
Bool isGenFDVar(TaggedRef term);
OzFDVariable * tagged2GenFDVar(TaggedRef term);
#undef inline
#endif

#endif

// eof
//-----------------------------------------------------------------------------
