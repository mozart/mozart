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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#include "reflect_space.hh"

//-----------------------------------------------------------------------------

OZ_Term reflect_space(OZ_Term v)
{
  TableClass<OZ_Term>      var_table;
  TableClass<Propagator *> prop_table;
  ReflectStackClass rec_stack((OzVariable *) NULL); //!!! tmueller

  while (!rec_stack.isEmpty()) {
    OZ_Term se = (OZ_Term) rec_stack.pop();
    void * ptr = tagValueOf(se);
    TypeOfReflStackEntry what = (TypeOfReflStackEntry) tagTypeOf(se);
    
    switch (what) {
    case Entry_Propagator:
      break;
    case Entry_Variable:
      break;
    default:
      break;
    }
  } // while
  
}
