/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: wuertz
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#include "taskintervals.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

//-----------------------------------------------------------------------------


#define INITIALSIZE 10000

static int *constraints;
static int initConstraints[INITIALSIZE];


static inline int intMin(int a, int b) { return a < b ? a : b; }
static inline int intMax(int a, int b) { return a > b ? a : b; }

OZ_C_proc_begin(sched_taskIntervals, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);

  return pe.impose(new TaskIntervalsPropagator(OZ_args[0], OZ_args[1]),
                   OZ_getLowPrio());
}
OZ_C_proc_end


class Min_max {
public:
  int min, max;
};

OZ_Return TaskIntervalsPropagator::propagate(void)
{
  int &ts  = reg_sz;
  int * dur = reg_offset;

  struct Set {
    int low, up, dur, extSize;
    int * ext;
    // extension for tasks inside task intervals
    int max, min;
  };


  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);
  PropagatorController_VV P(ts, x);


  int left, right, i, j, k, l;
  int fd_sup = OZ_getFDSup();
  int loopFlag = 0;
  int disjFlag = 0;

  // task interval data structures

  struct Set ** taskints;
  taskints = ((Set **)::new Set*[ts]);
  for (i=0; i<ts; i++) {
    taskints[i] = ::new Set[ts];
  }

  for (i=0; i<ts; i++)
    for (j=0; j<ts; j++)
      taskints[i][j].ext = ::new int[ts];


  // read fd variables
  for (i = ts; i--; )
    x[i].read(reg_l[i]);

  // initialize min/max values
  DECL_DYN_ARRAY(Min_max, MinMax, ts);
  for (i=ts; i--;){
    MinMax[i].min = x[i]->getMinElem();
    MinMax[i].max = x[i]->getMaxElem();
  }

  constraints = initConstraints;
  int * constraintsExtension = NULL;

  int constraintLimit = INITIALSIZE;



tiloop:


  int constraintsSize = 0;

  /////////
  // Initialize task intervals, cubic complexity
  ////////
  for (left = 0; left < ts; left++)
    for (right = 0; right < ts; right++) {
      struct Set *cset = &taskints[left][right];
      cset->low        = MinMax[left].min;
      cset->up         = MinMax[right].max + dur[right];

      cset->max        = MinMax[left].max + dur[left];
      cset->min        = MinMax[right].min;

      int cdur = 0;
      int csize = 0;
      if ( (cset->low <= MinMax[right].min)
           && (MinMax[left].max + dur[left] <= cset->up)
           )
        {
          // otherwise the task interval is trivially empty
          for (l=0; l < ts; l++) {
            int durL     = dur[l];
            int dueL     = MinMax[l].max + durL;
            int releaseL = MinMax[l].min;
            if ( (cset->low <= releaseL)
                 && (dueL <= cset->up) ) {
              cdur += durL;
              cset->ext[csize++] = l;

              if ( (l!=left) && (l!=right) ) {
                cset->max = intMax(cset->max, dueL);
                cset->min = intMin(cset->min, releaseL);
              }

            }
          }
        }
      cset->dur     = cdur;
      cset->extSize = csize;

      if ( (csize > 0) && (cset->up - cset->low < cdur) ) {
        goto failure;
      }
    }


  //////////
  // resize constraints if necessary
  //////////
  while (constraintsSize + ts * ts * ts * 3
         > constraintLimit) {
    cout << "increase constraintsSize" << endl;
      if (constraintLimit > INITIALSIZE)
        ::delete [] constraintsExtension;
      constraintsExtension = ::new int[constraintLimit*2];
      for (i=0; i<constraintsSize; i++)
        constraintsExtension[i] = constraints[i];
      constraints = constraintsExtension;
      constraintLimit = constraintLimit * 2;
  }

  //////////
  // Do the edge-finding
  //////////

  for (left = 0; left < ts; left++)
    for (right = 0; right < ts; right++) {
      struct Set *cset = &taskints[left][right];
      int setSize = cset->extSize;
      if (setSize > 1) {
        int releaseTI = cset->low;
        int dueTI     = cset->up;
        int durTI     = cset->dur;
        for (i = 0; i < ts; i++) {
          int maxI     = MinMax[i].max;
          int durI     = dur[i];
          int releaseI = MinMax[i].min;
          int dueI     = maxI + durI;
          int durAll, tdurTI, treleaseTI, tdueTI;
          if ( (releaseI >= releaseTI) && (dueI <= dueTI) ) {
            // I is in TI
            durAll = durTI;
            tdurTI = durTI - durI;
            if (i==left) {
              treleaseTI = cset->min;
              tdueTI = dueTI;
            }
            else {
              if (i==right) {
                tdueTI = cset->max;
                treleaseTI = releaseTI;
              }
              else {
                treleaseTI = releaseTI;
                tdueTI = dueTI;
              }
            }
          }
          else {
            // I is not in TI
            treleaseTI = releaseTI;
            tdueTI     = dueTI;
            tdurTI     = durTI;
            durAll     = durI + durTI;
          }

          int tmax = 0;
          int tmin = fd_sup;
          for (k=0; k<setSize; k++) {
            int element = cset->ext[k];
            if (element!=i) {
              tmax = intMax(tmax, MinMax[element].max);
              tmin = intMin(tmin, MinMax[element].min + dur[element]);
            }
          }
          if ( (tdueTI - treleaseTI >= durAll) &&
               (MinMax[i].min + durI <= tmax) &&   // I before the last
               (tmin <= maxI) )                    // I after the first
            {
            // I may be inside
            if (tdueTI - releaseI < durAll) {
              // I cannot be first but may be inside
              // ask the oracle
              if (releaseI < treleaseTI + dur[left]) {
                // otherwise it would be useless
                // after_first
                int mini = fd_sup;
                for (j=0; j<setSize; j++) {
                  int element = cset->ext[j];
                  if (element != i)
                    mini = intMin(mini, MinMax[element].min+dur[element]);
                }
                if (releaseI < mini) {
                  loopFlag = 1;
                  constraints[constraintsSize] = i;
                  constraints[constraintsSize+1] = 1;
                  constraints[constraintsSize+2] = mini;
                  constraintsSize += 3;
                }
              }
            }
            if (dueI - treleaseTI < durAll) {
              // I cannot be last but may be inside
              // ask the oracle
              if (dueI > tdueTI - dur[right]) {
                // otherwise it would be useless
                // before_last
                int maxi = 0;
                for (j=0; j<setSize; j++) {
                  int element = cset->ext[j];
                  if (element != i)
                    maxi = intMax(maxi, MinMax[element].max);
                }
                if (dueI > maxi) {
                  loopFlag = 1;
                  constraints[constraintsSize] = i;
                  constraints[constraintsSize+1] = 0;
                  constraints[constraintsSize+2] = maxi-durI;
                  constraintsSize += 3;
                }
              }
            }
          }
          else {
            if (tdueTI - releaseI < durAll) {
              // I cannot be first and not inside --> I must be last
              if (releaseI < treleaseTI + tdurTI) {
                loopFlag = 1;
                constraints[constraintsSize] = i;
                constraints[constraintsSize+1] = 1;
                constraints[constraintsSize+2] = treleaseTI + tdurTI;
                constraintsSize += 3;
              }
              // all others in TI must be finished before I starts
              for (j = 0; j < setSize; j++) {
                int element = cset->ext[j];
                int right = maxI - dur[element];
                if ( (i!=element) && (MinMax[element].max > right) ) {
                  loopFlag = 1;
                  constraints[constraintsSize] = element;
                  constraints[constraintsSize+1] = 0;
                  constraints[constraintsSize+2] = right;
                  constraintsSize += 3;
                  /*
                    // is not useful
                  OZ_Term left_side_task = reg_l[element];
                  OZ_Term right_side_task = reg_l[i];
                  addImpose(fd_prop_bounds, left_side_task);
                  addImpose(fd_prop_bounds, right_side_task);

                  impose(new LessEqOffPropagator(left_side_task, right_side_task,
                  -dur[element]));
                  */
                }
              }
            }
            if (dueI - treleaseTI < durAll) {
              // I cannot be last and not inside --> I must be first
              if (maxI > tdueTI - tdurTI - durI) {
                loopFlag = 1;
                constraints[constraintsSize] = i;
                constraints[constraintsSize+1] = 0;
                constraints[constraintsSize+2] = tdueTI - tdurTI - durI;
                constraintsSize += 3;
              }
              // all others in TI must start after I has finished
              for (j = 0; j < setSize; j++) {
                int element = cset->ext[j];
                int right = releaseI + durI;
                if ( (i!=element) && (MinMax[element].min < right) ) {
                  loopFlag = 1;
                  constraints[constraintsSize] = element;
                  constraints[constraintsSize+1] = 1;
                  constraints[constraintsSize+2] = right;
                  constraintsSize += 3;
                  /*
                    // is not useful
                  OZ_Term left_side_task = reg_l[i];
                  OZ_Term right_side_task = reg_l[element];
                  addImpose(fd_prop_bounds, left_side_task);
                  addImpose(fd_prop_bounds, right_side_task);

                  impose(new LessEqOffPropagator(left_side_task, right_side_task,
                  -dur[i]));
                  */
                }
              }
            }
          }
        }
      }
    }


  //////////
  // constrain the variables as memorized
  //////////
  for (i=0; i<constraintsSize; i+=3) {
    if (constraints[i+1]==0) {
      FailOnEmpty( *x[constraints[i]] <= constraints[i+2]);
      MinMax[constraints[i]].max = x[constraints[i]]->getMaxElem();
    }
    else {
      FailOnEmpty( *x[constraints[i]] >= constraints[i+2]);
      MinMax[constraints[i]].min = x[constraints[i]]->getMinElem();
    }
  }


  if (loopFlag == 1) {
    loopFlag = 0;
    goto tiloop;
  }

reifiedloop:

   //////////
   // do the reification in a loop
   //////////
   for (i=0; i<ts; i++)
     for (j=i+1; j<ts; j++) {
       int xui = MinMax[i].max, di = dur[i], xlj = MinMax[j].min;
       if (xui + di <= xlj) continue;
       int xuj = MinMax[j].max, dj = dur[j], xli = MinMax[i].min;
       if (xuj + dj <= xli) continue;
       if (xli + di > xuj) {
         if (xuj > xui - dj) {
           disjFlag = 1;
           FailOnEmpty(*x[j] <= xui - dj);
           MinMax[j].max = x[j]->getMaxElem();
         }
         if (xli < xlj + dj) {
           disjFlag = 1;
           FailOnEmpty(*x[i] >= xlj + dj);
           MinMax[i].min = x[i]->getMinElem();
         }
       }
       if (xlj + dj > xui) {
         if (xui > xuj - di) {
           disjFlag = 1;
           FailOnEmpty(*x[i] <= xuj - di);
           MinMax[i].max = x[i]->getMaxElem();
         }
         if (xlj < xli + di) {
           disjFlag = 1;
           FailOnEmpty(*x[j] >= xli + di);
           MinMax[j].min = x[j]->getMinElem();
         }
       }
     }

  if (disjFlag == 1) {
    disjFlag = 0;
    goto reifiedloop;
  }

  if (constraintLimit > INITIALSIZE)
    :: delete [] constraintsExtension;

  for (i=0; i<ts; i++)
    for (j=0; j<ts; j++)
      :: delete [] taskints[i][j].ext;

  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  return P.leave();

failure:
  if (constraintLimit > INITIALSIZE)
    :: delete [] constraintsExtension;

  for (i=0; i<ts; i++)
    for (j=0; j<ts; j++)
      :: delete [] taskints[i][j].ext;

  for (i=0; i<ts; i++) {
      :: delete taskints[i];
    }
  :: delete [] taskints;

  return P.fail();
}


OZ_CFun TaskIntervalsPropagator::spawner = sched_taskIntervals;
