/*
 *  Authors:
 *    Christian Schulte (schulte@dfki.de)
 * 
 *  Copyright:
 *    Christian Schulte, 1997, 1998
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

#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include "../builtins.hh"

/*
 * The builtin table
 */

#ifndef STATIC_LIBFD

#include "../libschedule.dcl"

OZ_C_proc_interface oz_interface[] = {
#include "../libschedule.tbl"
 {0,0,0,0}
};

#endif


