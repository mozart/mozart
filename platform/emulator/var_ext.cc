/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Copyright:
 *    Michael Mehl (1998)
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

#if defined(INTERFACE) && !defined(VAR_ALL)
#pragma implementation "var_ext.hh"
#endif

#include "var_ext.hh"

unsigned int oz_newVarId()
{
  static unsigned int counter=OZ_EVAR_LAST;
  if (counter==0) OZ_error("oz_newVarId: counter overflow");
  return counter++;
}

