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

//#define PRINT_CPLEX_PROBLEM

#ifdef PRINT_CPLEX_PROBLEM
#include "cplexprint.cc"
#endif

#ifdef ALLWAYS_CLOSE_CPLEX

#define DELETE_CPLEX_LP							\
{									\
  if (lp != NULL) {							\
    status = CPXfreeprob (CPLEX_env, &lp);				\
    if ( status ) {							\
      fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);	\
    }									\
  }									\
  if (CPLEX_env != NULL) {						\
    status = CPXcloseCPLEX (&CPLEX_env);				\
									\
    if ( status ) {							\
      char  errmsg[1024];						\
      fprintf (stderr, "Could not close CPLEX environment.\n");		\
      CPXgeterrorstring (CPLEX_env, status, errmsg);			\
      fprintf (stderr, "%s", errmsg);					\
    }									\
  }									\
}

#else

#define DELETE_CPLEX_LP							\
{									\
  if (lp != NULL) {							\
    status = CPXfreeprob (CPLEX_env, &lp);				\
    if ( status ) {							\
      fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);	\
    }									\
  }									\
}

CPXENVptr CPLEX_env;
#endif


OZ_Return RILPSolve::propagate_cplex(void)
{
  static char * result_str [] = {
    "fail",
    "optimal",
    "infeasible",
    "unbounded",
    "obj_lim",
    "it_lim_feas",
    "it_lim_infeas",
    "time_lim_feas",
    "time_lim_infeas",
    "num_best_feas",
    "num_best_infeas",
    "optimal_infeas",
    "abort_feas",
    "abort_infeas",
    "abort_dual_infeas",
    "abort_prim_infeas",
    "abort_prim_dual_infeas",
    "abort_prim_dual_feas",
    "abort_crossover"
  };

  int i;

  // if`sol' is constrained then additionally impose the following
  // constraints:
  //    sum c_i * x_i >= lb(sol)
  //    sum c_i * x_i <= ub(sol)

  RIVar     sol(_sol);
  int       impose_lb = (sol->lowerBound() > RI_FLOAT_MIN) ? 1 : 0;
  int       impose_ub = (sol->upperBound() < RI_FLOAT_MAX) ? 1 : 0;
  int       numcols = OZ_vectorSize(_vars); 
  int       objsen;

  OZ_Term objfn_row = OZ_subtree(_objfn, atom_row);
  if (OZ_vectorSize(objfn_row) != numcols) {
    RUNTIME_ERROR(("objective function %d: number of columns differs", i));
  }

  {
    OZ_Term objfn_opt = OZ_deref(OZ_subtree(_objfn, atom_opt));

    objsen = (objfn_opt == atom_max) ? CPX_MAX : CPX_MIN;

    (objfn_opt == atom_max ? impose_ub : impose_lb) = 0;
  }
  
  char *    probname = "cplex 5.0";
  int       numrows = OZ_vectorSize(_constr) + impose_lb + impose_ub; 
  int       numnz = numcols * numrows;
  double    obj[numcols];
  double    rhs[numrows]; 
  char      sense[numrows]; 
  int       matbeg[numcols]; 
  int       matcnt[numcols]; 
  int       matind[numnz];
  double    matval[numnz];
  double    lb[numcols];
  double    ub[numcols];

  CPXLPptr  lp  = NULL;
  int       status;

  RIVar     xs[numcols];

  OZ_Term   vars[numcols];
  PropagatorController_VRI_RI P(numcols, xs, sol);

  // add objective function
  getDoubleVector(objfn_row, obj);

  // add constraints
  {
    double constr_row[numcols];
    OZ_Term constr[numrows - impose_lb - impose_ub];
    OZ_getOzTermVector(_constr, constr);
    

    for (i = 0; i < numcols; i += 1) {
      matbeg[i] = i * numrows;
      matcnt[i] = numrows;
    }

    // count rows-wise
    for (i = 0; i < numrows - impose_lb - impose_ub; i += 1) {
      OZ_Term row_i = constr[i];
      
      OZ_Term row = OZ_subtree(row_i, atom_row);
      
      if (OZ_vectorSize(row) != numcols) {
	RUNTIME_ERROR(("constraint %d: number of columns differs", i));
      }
      
      // right hand-side
      rhs[i] = OZ_floatToC(OZ_subtree(row_i, atom_rhs));
      
      // sense of row
      OZ_Term type = OZ_deref(OZ_subtree(row_i, atom_type));
      
      sense[i] = (type == atom_le ? 'L' : 
		  (type == atom_ge ? 'G' :
		   (type == atom_eq ? 'E' : 'R')));
      
      // constraint of row
      getDoubleVector(row, constr_row);
      
      // count column-wise
      for (int cnt = i, j = 0; j < numcols; j += 1, cnt += numrows) {
	matind[cnt] = i;
	matval[cnt] = constr_row[j];
      }
    }

    // add upper and lower bound constraints
    i = numrows - impose_lb - impose_ub;
    if (impose_lb) {
      PRINT_CPLEX(("Adding: sum c_i*x_i >= %f\n", sol->lowerBound()));

      rhs[i]   = sol->lowerBound();
      sense[i] = 'G';

      for (int cnt = i, j = 0; j < numcols; j += 1, cnt += numrows) {
	matind[cnt] = i;
	matval[cnt] = obj[j];
      }

      i += 1;
    }
    if (impose_ub) {
      PRINT_CPLEX(("Adding: sum c_i*x_i <= %f\n", sol->upperBound()));

      rhs[i]   = sol->upperBound();
      sense[i] = 'L';

      for (int cnt = i, j = 0; j < numcols; j += 1, cnt += numrows) {
	matind[cnt] = i;
	matval[cnt] = obj[j];
      }
    }
  }

  // add bounds
  OZ_getOzTermVector(_vars, vars);
  
  for (i = 0; i < numcols; i += 1) {
    xs[i].read(vars[i]);
    
    // this is a trick to fix a problem of CPLEX
    lb[i] = max_ri(xs[i]->lowerBound(), -CPX_INFBOUND/2.0);
    ub[i] = min_ri(xs[i]->upperBound(), CPX_INFBOUND/2.0);
  }

#ifdef ALLWAYS_CLOSE_CPLEX
  // open cplex
  CPXENVptr CPLEX_env = CPXopenCPLEXdevelop(&status);

  if (CPLEX_env == NULL) {
    char  errmsg[1024];
    fprintf (stderr, "Could not open CPLEX environment.\n");
    CPXgeterrorstring (CPLEX_env, status, errmsg);
    fprintf (stderr, "%s", errmsg);
  }
#endif

  // create the problem
  lp = CPXcreateprob (CPLEX_env, &status, probname);
  
  if ( lp == NULL ) {
    fprintf (stderr, "Failed to create LP.\n");
    goto terminate;
  }

  status = CPXcopylp (CPLEX_env, lp, numcols, numrows, objsen, obj, rhs, 
		      sense, matbeg, matcnt, matind, matval,
		      lb, ub, NULL);

  if ( status ) {
    fprintf (stderr, "Failed to copy problem data.\n");
    goto terminate;
  }

#ifdef PRINT_CPLEX_PROBLEM
  printCPXproblem(CPLEX_env, lp);
#endif

  PRINT_CPLEX(("Solving -> "));;
  
  status = (solver == cplex_primopt) 
    ? CPXprimopt(CPLEX_env, lp) 
    : ((solver == cplex_dualopt) ? CPXdualopt(CPLEX_env, lp) : 0);

  PRINT_SOLVE1(("%s\n", result_str[status]));
  
  if (status) {
    goto terminate;
  }

  {
    int       solstat;
    double    objval;
    double    x[numcols];
    double    pi[numrows];
    double    slack[numrows];
    double    dj[numcols];

    status = CPXsolution (CPLEX_env, lp, &solstat, &objval, x, pi, slack, dj);
   
    if ( status ) {
      fprintf (stderr, "Failed to obtain solution.\n");
      goto terminate;
    }
    
#ifdef DEBUG
    {
      printf ("\nSolution status = %d\n", solstat);
      printf ("Solution value  = %f\n\n", objval);
      
      int cur_numrows = CPXgetnumrows (CPLEX_env, lp);
      int cur_numcols = CPXgetnumcols (CPLEX_env, lp);
      for (i = 0; i < cur_numrows; i++) {
	printf ("Row %d:  Slack = %10f  Pi = %10f\n", i, slack[i], pi[i]);
      }
      
      for (int j = 0; j < numcols; j++) {
	printf ("Column %d:  Value = %10f  Reduced cost = %10f\n",
		j, x[j], dj[j]);
      }
    }
#endif

    {
      OZ_Term res = atom_oops;
      
      switch (solstat) {
      case CPX_OPTIMAL:
      default: 
	{  

	  int cur_numrows = CPXgetnumrows (CPLEX_env, lp);
	  int cur_numcols = CPXgetnumcols (CPLEX_env, lp);

	  if (numcols != cur_numcols) {
	    fprintf(stderr, "numcols and cur_numcols differ!\n");
	  }

	  for(i = 0; i < numcols; i += 1) {
	    ri_float x_i = x[i];
	    // cplex does not pay sufficient attention to the bounds
	    x_i = min_ri(x_i, ub[i]);
	    x_i = max_ri(x_i, lb[i]);
	    FailOnInvalid(*xs[i] = (double)x_i);
	  }

	  PRINT_CPLEX(("optimal solution=%f\n", objval));
	  
	  FailOnInvalid(*sol = objval);

	  res = atom_optimal;
	 
	  break;
	}

      case CPX_INFEASIBLE: res = atom_infeasible; break;
      case CPX_UNBOUNDED:  res = atom_unbounded;  break;
      case 0:              res = atom_failure;  break;
      }

      P.vanish();
      DELETE_CPLEX_LP;            
      return OZ_unify(_res, res);
    }
  }

failure:
  DELETE_CPLEX_LP;
  return P.fail();
  
terminate:
  DELETE_CPLEX_LP;
  return P.fail();
}

