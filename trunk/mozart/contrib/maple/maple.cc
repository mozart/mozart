/*
 *  Authors:
 *    Juergen Zimmer (jzimmer@ps.uni-sb.de)
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

#include "maple.hh"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


extern "C" 
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table [] = {
      {"call",          3, 1, maple_call},
      {0,0,0,0}
    };
    return i_table;
  }
} /* extern "C" */




// End of File
//-----------------------------------------------------------------------------

