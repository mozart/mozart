inline 
char sign_char(double d) { 
  return d < 0.0 ? '-' : '+'; 
}

inline 
char sense_char(char s) {
  return s == 'E' ? '=' : (s == 'L' ? '<' : (s == 'G' ? '>' : '?'));
}

inline 
void linebreak(int i) {
  if (i % 5 == 0)
    printf("\n");
}

void printCPXproblem(CPXENVptr env, CPXLPptr lp) 
{
  int num_cols = CPXgetnumcols(env, lp);
  int num_rows = CPXgetnumrows(env, lp);
  
  // print objective function
  if (CPXgetobjsen(env, lp) == CPX_MIN)
    printf("\nMinimize\n");
  else
    printf("\nMaximize\n");

  double obj[num_cols];
  CPXgetobj(env, lp, obj, 0, num_cols - 1);
   
  printf("%g C1 ", obj[0]);
  for (int i = 1; i < num_cols; i += 1) {
    printf("%c %g C%d ", sign_char(obj[i]),  fabs(obj[i]), i+1);
    linebreak(i);
  }

  printf("\n\nSubject To\n");

  char sense[num_rows];
  CPXgetsense(env, lp, sense, 0, num_rows - 1);
  double rhs[num_rows];
  CPXgetrhs(env, lp, rhs, 0, num_rows - 1);
  
  for (int j = 0; j < num_rows; j += 1) {
    
    double val;
    CPXgetcoef(env, lp, j , 0, &val);
    printf("%g C1 ", val);
    
    for (int i = 1; i < num_cols; i += 1) {
      double val;
      CPXgetcoef(env, lp, j , i, &val);
      printf("%c %g C%d ", sign_char(val),  fabs(val), i+1);
      linebreak(i);
    }
    printf(" %c %g\n", sense_char(sense[j]), rhs[j]);
  }
  printf("\n\nBounds\n");
  
  double lb[num_cols];
  CPXgetlb(env, lp, lb, 0, num_cols - 1);
  double ub[num_cols];
  CPXgetub(env, lp, ub, 0, num_cols - 1);

  for (int i = 0; i < num_cols; i += 1) {
    printf("%g <= C%d <= %g\n", lb[i], i+1, ub[i]);
  }
  printf("End\n");
}
