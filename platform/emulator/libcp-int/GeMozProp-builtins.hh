/*
 *  Main authors:
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alejandro Arbelaez, 2006-2007
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

#ifndef __GEOZ_MOZ_PROP_BUILTINS_HH__
#define __GEOZ_MOZ_PROP_BUILTINS_HH__

#include "GeIntVar.hh"
#include "GeSpace-builtins.hh"
//#include "../libfd/fdaux.cc"

OZ_Term * vectorToOzTerms2(OZ_Term t, int &sz)
{
  OZ_Term * v;

  if (OZ_isLiteral(t)) {

    sz = 0; 
    v = NULL;

  } else if (OZ_isCons(t)) {

    sz = OZ_length(t);
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(t); t = OZ_tail(t)) 
      v[i++] = OZ_head(t);

  } else if (OZ_isTuple(t)) {

    sz = OZ_width(t);
    v = OZ_hallocOzTerms(sz);
    for (int i = 0; i < sz; i += 1) 
      v[i] = OZ_getArg(t, i);

  } else {

    //OZ_ASSERT(OZ_isRecord(t));

    OZ_Term al = OZ_arityList(t);
    sz = OZ_width(t);

    v = OZ_hallocOzTerms(sz);
    for (int i = 0; OZ_isCons(al); al = OZ_tail(al)) 
      v[i++] = OZ_subtree(t, OZ_head(al));

  } 

  return v;
}

//#include "MozProp/MozProp.hh"

/*
//Generic propagators
OZ_BI_proto(int_sumCN);

OZ_BI_proto(int_domlist);
OZ_BI_proto(int_nextLarger);
OZ_BI_proto(int_nextSmaller);
OZ_BI_proto(int_dom);

//Wathing domains
OZ_BI_proto(int_watch_min);
OZ_BI_proto(int_watch_max);
OZ_BI_proto(int_watch_size);

//Limits descriptions
OZ_BI_proto(int_sup);
OZ_BI_proto(int_min);

//Miscellaneous propagators
OZ_BI_proto(int_disjoint);

//Reified propagators
OZ_BI_proto(int_sumR);
OZ_BI_proto(int_sumCR);
OZ_BI_proto(int_sumCNR);
OZ_BI_proto(int_reified_int);

// 0/1 Propagators
OZ_BI_proto(bool_Gand);
OZ_BI_proto(bool_Gor);
OZ_BI_proto(bool_Gxor);
OZ_BI_proto(bool_Gnot);
OZ_BI_proto(bool_Gimp);
OZ_BI_proto(bool_Geqv);
*/



#endif
