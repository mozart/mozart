/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Michael Mehl (1997,1998)
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

//  internal interface to AMOZ

#include "am.hh"
#include "refsarray.hh"

/*
 * Control Vars
 */

OZ_Return suspendOnControlVar()
{
  Assert(oz_currentThread() != NULL);
  am.prepareCall(BI_controlVarHandler,
		 RefsArray::make(am.emptySuspendVarList()));
  return BI_REPLACEBICALL;
}

void suspendOnControlVar2()
{
  (void) suspendOnControlVar();
}

