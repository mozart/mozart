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

#include "lpkit.h"

OZ_Return RILPSolve::propagate_lpsolve(void)
{
  static char * result_str [] = {
    "optimal", 
    "milp_fail", 
    "infeasible", 
    "unbounded", 
    "failure", 
    "running"
  };

  int i;
  int cols = OZ_vectorSize(_vars);
  int rows = OZ_vectorSize(_constr);
  
  lprec * lp = make_lp(0, cols);
  
  int maximise;

  RIVar     sol(_sol);
  int       impose_lb = (sol->lowerBound() > RI_FLOAT_MIN) ? 1 : 0;
  int       impose_ub = (sol->upperBound() < RI_FLOAT_MAX) ? 1 : 0;

  RIVar x[cols];
  PropagatorController_VRI_RI P(cols, x, sol);
  
  // add objective function
  double coeff_row[cols+1];
  OZ_Term objfn_row = OZ_subtree(_objfn, atom_row);
  if (OZ_vectorSize(objfn_row) != cols) {
    RUNTIME_ERROR(("objective function %d: number of columns differs", i));
  }
  
  OZ_Term objfn_opt = OZ_deref(OZ_subtree(_objfn, atom_opt));
  maximise = (objfn_opt == atom_max);
  (objfn_opt == atom_max ? impose_ub : impose_lb) = 0;

  getDoubleVector(objfn_row, coeff_row+1);
  
  set_obj_fn(lp, coeff_row);
  
  // add constraints
  {
    double rhs, constr_row[cols+1];
    OZ_Term constr[rows];
    OZ_getOzTermVector(_constr, constr);
    
    for (i = 0; i < rows; i += 1) {
      OZ_Term row_i = constr[i];

      OZ_Term row = OZ_subtree(row_i, atom_row);
      if (OZ_vectorSize(row) != cols) {
	RUNTIME_ERROR(("constraint %d: number of columns differs", i));
      }
      getDoubleVector(row, constr_row+1);

      rhs = OZ_floatToC(OZ_subtree(row_i, atom_rhs));

      OZ_Term type = OZ_deref(OZ_subtree(row_i, atom_type));
      short int constr_type = (type == atom_le ? LE : 
			       (type == atom_ge ? GE :
				(type == atom_eq ? EQ : OF)));

      add_constraint(lp, constr_row, constr_type, rhs);
    }

    if (impose_lb) {
      PRINT_LPSOLVE(("Adding: sum c_i*x_i >= %f\n", sol->lowerBound()));

      add_constraint(lp, coeff_row, GE, sol->lowerBound());
    }
    if (impose_ub) {
      PRINT_LPSOLVE(("Adding: sum c_i*x_i <= %f\n", sol->upperBound()));

      add_constraint(lp, coeff_row, LE, sol->upperBound());
    }
    
  }

  // add bounds
  OZ_Term vars[cols];
  OZ_getOzTermVector(_vars, vars);
  
  for (i = 0; i < cols; i += 1) {
    x[i].read(vars[i]);
    
    double lb = x[i]->lowerBound();
    if (lb > RI_FLOAT_MIN) 
      set_lowbo(lp, i + 1, lb);
    
    double ub = x[i]->upperBound();
    if (RI_FLOAT_MAX > ub)
      set_upbo(lp, i + 1, ub); 
  }
  
  if (maximise) 
    set_maxim(lp);

#ifdef DEBUG
  print_lp(lp);
#endif

  PRINT_LPSOLVE(("Solving -> "));

  int result = solve(lp);
  
  PRINT_SOLVE1(("%s\n", result_str[result]));

#ifdef DEBUG
  print_solution(lp);
#endif

  OZ_Term res = atom_oops;
  
  
  switch (result) {
  case OPTIMAL: {

    for(i = 0; i < lp->columns; i += 1)
      FailOnInvalid(*x[i] = (double)lp->best_solution[lp->rows+i+1]);
    
#ifdef DEBUG_CHECK
    PRINT_LPSOLVE(("optimal solution=%f\n", (double)lp->best_solution[0]));
#endif

    FailOnInvalid(*sol = ((double) lp->best_solution[0]) );
    res = atom_optimal;
    break;
  }
  case INFEASIBLE: res = atom_infeasible; break;
  case UNBOUNDED:  res = atom_unbounded;  break;
  case FAILURE:    res = atom_failure;  break;
  }
  
  P.vanish();
  delete_lp(lp);
  return OZ_unify(_res, res);

failure:
  delete_lp(lp);
  return P.fail();
}
