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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __MAPLE_HH__
#define __MAPLE_HH__

#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#include "mozart_cpi.hh"

//the temporary maple in file
#define MAPLE_IN_TEMPLATE "/tmp/oz-maple-in-XXXXXX"
#define MAPLE_OUT_TEMPLATE "/tmp/oz-maple-out-XXXXXX"
#define PARSE_OUT_TEMPLATE "/tmp/oz-parse-out-XXXXXX"

#define MAPLE_CALL_TEMPLATE "%s -q -w0 < %s > %s" 
#define PARSE_CALL_TEMPLATE "%s %s %s"
#define DELETE_TEMPLATE "rm -f %s; rm -f %s; rm -f %s"

#define OPEN_FILE OZ_atom("Could not open file")

#define EXCEPTION "maple"

#define PARSER_OUTPUT_LENGTH OZ_atom("The parser ouput is too large.")

//-----------------------------------------------------------------------------

#define INIT_FUNC(F_NAME) OZ_C_proc_interface * F_NAME(void)

extern "C" INIT_FUNC(oz_init_module);

OZ_BI_proto(maple_call);


//-----------------------------------------------------------------------------
// auxiliary functions and other stuff


#endif /* __MAPLE_HH__ */
