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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __PROPAGATORS_HH__
#define __PROPAGATORS_HH__

#include "lp.hh"

#ifdef INCLUDE_CPLEX
#include "cplex.h"
#endif

//-----------------------------------------------------------------------------

class RILPSolve : public OZ_Propagator {

  friend INIT_FUNC_LP;

private:

  static OZ_PropagatorProfile profile;

  OZ_Term _vars, _objfn, _constr, _sol, _res;

  static enum solvers_e {
#ifdef INCLUDE_CPLEX
    cplex_primopt = 0,
    cplex_dualopt,
    lpsolve
#else
    lpsolve = 0
#endif
  } solver;

  static enum mode_e {verbose = 0, quiet} mode;

public:
  RILPSolve(OZ_Term vars, OZ_Term objfn, 
		      OZ_Term constr, OZ_Term sol, OZ_Term res) 
    : _vars(vars), _objfn(objfn), _constr(constr), _sol(sol), _res(res) {}

  virtual OZ_PropagatorProfile * getProfile(void) const { return &profile; }

  OZ_Return propagate_lpsolve(void);

#ifdef INCLUDE_CPLEX
  OZ_Return propagate_cplex(void);
  virtual OZ_Return propagate(void) {
    return (solver == lpsolve) ? propagate_lpsolve() : propagate_cplex();
  }
#else
  virtual OZ_Return propagate(void) {
    return propagate_lpsolve();
  }
#endif

  virtual size_t sizeOf(void) { return sizeof(RILPSolve); }

  virtual void sClone(void) { 
    fprintf(stderr, "RILPSolve::sClone Must never be called!\n");
    fflush(stderr);
  }

  virtual void gCollect(void) { 
    fprintf(stderr, "RILPSolve::gCollect must never be called!\n");
    fflush(stderr);
  }

  virtual OZ_Term getParameters(void) const { return OZ_nil(); }

  static int cplex_avail(void) {
#ifdef INCLUDE_CPLEX
    int status;
    CPXENVptr cplex_env = CPXopenCPLEXdevelop(&status);    
    int ret = (cplex_env != NULL);
    CPXcloseCPLEX (&cplex_env);
#else
    int ret = 0;
#endif
    return ret;
  }

  static OZ_Term getSolverAtom(void) {
    static OZ_Term solver_atoms[] = { 
#ifdef INCLUDE_CPLEX
      atom_cplex_primopt, 
      atom_cplex_dualopt,
#endif
      atom_lpsolve 
    };
    return solver_atoms[solver];
  }
  static OZ_Term getModeAtom(void) {
    const static OZ_Term mode_atoms[] = {
      atom_verbose, atom_quiet
    };
    return mode_atoms[mode];
  }

  static void putSolverAtom(OZ_Term solver_term) {

    solver_term = OZ_deref(solver_term);
    
#ifdef INCLUDE_CPLEX
    if (cplex_avail()) {
      if (atom_cplex_primopt == solver_term) { 
	solver = cplex_primopt;
	return;
      }
      if (atom_cplex_dualopt == solver_term) { 
	solver = cplex_dualopt;
	return;
      }
    }
#endif
    
    solver = lpsolve;
  }
  
  static void putModeAtom(OZ_Term mode_term) {
    mode = (OZ_deref(mode_term) == atom_verbose) ? verbose : quiet;
  }
};

#define EM_LP_CONSTR "objfn(row: <FLOAT\\, ...\\, FLOAT> opt: `min'|`max')" 
#define EM_LP_OBJFN  "<constr(row: <FLOAT\\, ...\\, FLOAT>type: '=<'|'=='|'>=' rhs: FLOAT) ...>"

#define _PRINTSOLVER(S, ARGS) if (mode == verbose) {printf(S); printf ARGS;}
#define PRINT_SOLVE1(ARGS) if (mode == verbose) {printf ARGS;}
#define PRINT_CPLEX(ARGS)   _PRINTSOLVER("(cplex): ", ARGS)
#define PRINT_LPSOLVE(ARGS) _PRINTSOLVER("(lpsolve): ", ARGS)

#endif /* __PROPAGATORS_HH__ */
