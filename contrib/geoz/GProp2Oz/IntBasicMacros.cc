/*
 *  Main authors:
 *     Javier A. Mena <javimena@univalle.edu.co>    
 *
 *  Contributing authors:
 *     
 *
 *  Copyright:
 *     Javier A. Mena, 2006    
 *
 *  Last modified:
 *     $Date: 2006-09-05 15:26:30 -0500 (Tue, 05 Sep 2006) $
 *     $Revision: 225 $
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */


#include "IntBasicMacros.hh"

GeSpace* get_GeIntVarArgsAnyGeSpace(OZ_Term vec) {
  vec = OZ_deref(vec);
  GeSpace* cs = NULL;
  if (OZ_isList(vec,NULL)) {
    OZ_Term v = vec;
    for (; !OZ_isNil(v); v = OZ_tail(v)) {
      OZ_Term head = OZ_head(v);
      CHECK_AND_SET_SPACE(cs,head);
    }
  }
  else {
    assert(OZ_isTuple(vec));
    int oz_width = OZ_width(vec);
    for (int i=0; i < oz_width; i++) {
      OZ_Term v = OZ_getArg(vec,i);
      CHECK_AND_SET_SPACE(cs,v);
    }
  }
  return cs;
}


bool vectorChecker(OZ_Term v, bool checkingIsIntVar) {
  v = OZ_deref(v);
  bool isOzList = OZ_isList(v,NULL);
  int res = 0;
  if (isOzList) {
    for(; !OZ_isNil(v); v = OZ_tail(v)) {
      OZ_Term h = OZ_head(v);

      /* checker code */
      if (checkingIsIntVar && OZ_isGeIntVar(h)) res = 2;
      else if (!OZ_isInt(h)) return false;
      else res = (res==2) ? 2 : 1; /* the value is an Int */
    }
  }
  else if (OZ_isTuple(v)) {
    int oz_width = OZ_width(v);
    for (int i=0; i < oz_width; i++) {
      OZ_Term h = OZ_getArg(v,i);

      /* checker code */
      if (checkingIsIntVar && OZ_isGeIntVar(h)) res = 2;
      else if (!OZ_isInt(h)) return false;
      else res = (res==2) ? 2 : 1; /* the value is an Int */
    }
  }

  if (checkingIsIntVar && (res==2)) return true;
  else if (!checkingIsIntVar && (res==1)) return true;
  else return false;
}

/* Returns true if there is at least one element of type GeIntVar,
 * and all other elements are IntVar or ints.
 */
bool isGeIntVarArgs(OZ_Term v) {
  return vectorChecker(v,true);
}


bool isIntArgsTerm(OZ_Term v) {
  return vectorChecker(v,false);
}

  
