/*
 *  Authors:
 *    Joerg Wuertz (wuertz@ps.uni-sb.de)
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

#include "diffn.hh"
#include "rel.hh"
#include "auxcomp.hh"
#include <stdlib.h>

class Min_max {
public:
  int min, max;
};

//////////
// BUILTIN
//////////
OZ_C_proc_begin(fdp_distinct2, 4)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT "," OZ_EM_VECT OZ_EM_FD "," OZ_EM_VECT OZ_EM_INT);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectVectorInt);
  OZ_EXPECT(pe, 2, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 3, expectVectorInt);
  SAMELENGTH_VECTORS(0, 1);
  SAMELENGTH_VECTORS(0, 2);
  SAMELENGTH_VECTORS(0, 3);

  return pe.impose(new DiffnPropagator(OZ_args[0], OZ_args[1], OZ_args[2],
                                       OZ_args[3]));
}
OZ_C_proc_end

//////////
// CONSTRUCTOR
//////////

DiffnPropagator::DiffnPropagator(OZ_Term xtasks, OZ_Term xdurs,
                                 OZ_Term ytasks, OZ_Term ydurs)
{
  int i=0;

  reg_x = vectorToOzTerms(xtasks, reg_size);
  reg_y = vectorToOzTerms(ytasks, reg_size);
  reg_xdurs   = vectorToInts(xdurs, reg_size);
  reg_ydurs   = vectorToInts(ydurs, reg_size);
  int limit   = reg_size * reg_size;
  reg_ordered = OZ_hallocCInts(limit);

  VectorIterator vix(xtasks);
  VectorIterator viy(ytasks);
  VectorIterator vidx(xdurs);
  VectorIterator vidy(ydurs);


  for (i=0; i<limit; i++) {
    reg_ordered[i] = 0;
  }

}

//////////
// DESTRUCTOR
//////////

DiffnPropagator::~DiffnPropagator()
{
}

//////////
// COPYING
//////////

void DiffnPropagator::updateHeapRefs(OZ_Boolean duplicate)
{
  OZ_Term * new_reg_x      = OZ_hallocOzTerms(reg_size);
  int * new_reg_xdurs      = OZ_hallocCInts(reg_size);
  OZ_Term * new_reg_y      = OZ_hallocOzTerms(reg_size);
  int * new_reg_ydurs      = OZ_hallocCInts(reg_size);
  int * new_reg_ordered    = OZ_hallocCInts(reg_size*reg_size);

  int i;
  for (i = reg_size; i--; ) {
    new_reg_x[i]       = reg_x[i];
    new_reg_y[i]       = reg_y[i];
    new_reg_xdurs[i]   = reg_xdurs[i];
    new_reg_ydurs[i]   = reg_ydurs[i];
    new_reg_ordered[i] = reg_ordered[i];
    OZ_updateHeapTerm(new_reg_x[i]);
    OZ_updateHeapTerm(new_reg_y[i]);
  }
  for (i = reg_size; i<reg_size*reg_size; i++)
    new_reg_ordered[i] = reg_ordered[i];


  reg_x               = new_reg_x;
  reg_y               = new_reg_y;
  reg_xdurs           = new_reg_xdurs;
  reg_ydurs           = new_reg_ydurs;
  reg_ordered         = new_reg_ordered;
}

OZ_Term DiffnPropagator::getParameters(void) const
{
  TERMVECTOR2LIST(reg_x, reg_size, x);
  INTVECTOR2LIST(reg_xdurs, reg_size, xdurs);
  TERMVECTOR2LIST(reg_y, reg_size, y);
  INTVECTOR2LIST(reg_ydurs, reg_size, ydurs);

  RETURN_LIST4(x, xdurs, y, ydurs);
}

//////////
// SPAWNER
//////////

OZ_CFunHeader DiffnPropagator::spawner = fdp_distinct2;


//////////
// RUN METHOD
//////////

OZ_Return DiffnPropagator::propagate(void)
{
  int &ts  = reg_size;
  int * xdur = reg_xdurs;
  int * ydur = reg_ydurs;

  DECL_DYN_ARRAY(OZ_FDIntVar, x, ts);
  DECL_DYN_ARRAY(OZ_FDIntVar, y, ts);

  int i, j;
  for (i = ts; i--; ) {
    x[i].read(reg_x[i]);
    y[i].read(reg_y[i]);
  }

  DECL_DYN_ARRAY(Min_max, xMinMax, ts);
  DECL_DYN_ARRAY(Min_max, yMinMax, ts);
  for (i=ts; i--;){
    xMinMax[i].min = x[i]->getMinElem();
    xMinMax[i].max = x[i]->getMaxElem();
    yMinMax[i].min = y[i]->getMinElem();
    yMinMax[i].max = y[i]->getMaxElem();
  }

  int disjFlag = 0;
  int ground = 1;

reifiedloop:

  for (i=0; i<ts; i++) {
    int count = i * reg_size;
    for (j=i+1; j<ts; j++) {
      if (reg_ordered[count+j] == 0) {
        // if one is entailed, no action takes place
        int xui = xMinMax[i].max, xdi = xdur[i], xlj = xMinMax[j].min;
        if (xui + xdi <= xlj) {
          reg_ordered[count+j] = 1;
          continue;
        }
        int xuj = xMinMax[j].max, xdj = xdur[j], xli = xMinMax[i].min;
        if (xuj + xdj <= xli) {
          reg_ordered[count+j] = 1;
          continue;
        }
        int yui = yMinMax[i].max, ydi = ydur[i], ylj = yMinMax[j].min;
        if (yui + ydi <= ylj) {
          reg_ordered[count+j] = 1;
          continue;
        }
        int yuj = yMinMax[j].max, ydj = ydur[j], yli = yMinMax[i].min;
        if (yuj + ydj <= yli) {
          reg_ordered[count+j] = 1;
          continue;
        }
        if ( (yli + ydi > yuj) && (ylj + ydj > yui) ) {
          if (xli + xdi > xuj) {
            if (xuj > xui - xdj) {
              disjFlag = 1;
              FailOnEmpty(*x[j] <= xui - xdj);
              xMinMax[j].max = x[j]->getMaxElem();
            }
            if (xli < xlj + xdj) {
              disjFlag = 1;
              FailOnEmpty(*x[i] >= xlj + xdj);
              xMinMax[i].min = x[i]->getMinElem();
            }
          }
          if (xlj + xdj > xui) {
            if (xui > xuj - xdi) {
              disjFlag = 1;
              FailOnEmpty(*x[i] <= xuj - xdi);
              xMinMax[i].max = x[i]->getMaxElem();
            }
            if (xlj < xli + xdi) {
              disjFlag = 1;
              FailOnEmpty(*x[j] >= xli + xdi);
              xMinMax[j].min = x[j]->getMinElem();
            }
          }
        }
        else {
          if ( (xli + xdi > xuj) && (xlj + xdj > xui) ) {
            if (yli + ydi > yuj) {
              if (yuj > yui - ydj) {
                disjFlag = 1;
                FailOnEmpty(*y[j] <= yui - ydj);
                yMinMax[j].max = y[j]->getMaxElem();
              }
              if (yli < ylj + ydj) {
                disjFlag = 1;
                FailOnEmpty(*y[i] >= ylj + ydj);
                yMinMax[i].min = y[i]->getMinElem();
              }
            }
            if (ylj + ydj > yui) {
              if (yui > yuj - ydi) {
                disjFlag = 1;
                FailOnEmpty(*y[i] <= yuj - ydi);
                yMinMax[i].max = y[i]->getMaxElem();
              }
              if (ylj < yli + ydi) {
                disjFlag = 1;
                FailOnEmpty(*y[j] >= yli + ydi);
                yMinMax[j].min = y[j]->getMinElem();
              }
            }
          }
        }
      }
    }
  }

  if (disjFlag == 1) {
    disjFlag = 0;
    goto reifiedloop;
  }

  for (i=0; i<ts; i++) {
    if (x[i].leave()) ground = 0;
    if (y[i].leave()) ground = 0;
  }

  if (ground == 1) return PROCEED;
  else return SLEEP;

failure:

  for (i=0; i<ts; i++) {
    x[i].fail();
    y[i].fail();
  }
  return FAILED;

}
