/*
 *  Authors:
 *    Jürgen Zimmer (jzimmer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Tobias Müller (tmueller@ps.uni-sb.de)
 * 
 *  Copyright:
 *    1999
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

extern "C"
{
  OZ_C_proc_interface * oz_init_module(void)
  {
    static OZ_C_proc_interface i_table [] = {
      {"call",          2, 1, maple_call},
      {0,0,0,0}
    };
    return i_table;
  }
} /* extern "C" */

// End of File
//-----------------------------------------------------------------------------
