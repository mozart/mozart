/*
 *  Author:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Leif Kornstaedt, 2001
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation of Oz 3:
 *    http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *    http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 */

#include "builtins.hh"

OZ_BI_define(BIaliceRPC,3,0) {
  OZ_Term rpc = registry_get(AtomAliceRPC);
  if (rpc == 0)
    return oz_raise(E_ERROR,E_ALICE,"undefinedProperty",1,AtomAliceRPC);
  if (!oz_isProcedure(rpc) || tagged2Const(rpc)->getArity() != 3)
    return oz_raise(E_ERROR,E_ALICE,"illegalArity",2,AtomAliceRPC,rpc);
  am.prepareCall(rpc,RefsArray::make(OZ_in(0),OZ_in(1),OZ_in(2)));
  return BI_REPLACEBICALL;
} OZ_BI_end
