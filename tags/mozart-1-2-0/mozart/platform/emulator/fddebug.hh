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
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __FDDEBUG_HH__
#define __FDDEBUG_HH__

#include "resources.hh"

//-----------------------------------------------------------------------------

//#define DEBUG_CONSTRAINT_IR

#ifdef DEBUG_CONSTRAINT_IR
//#define DEBUG_FD
//#define DEBUG_FSET

//#define DEBUG_FD_CONSTRREP
//#define DEBUG_FSET_CONSTRREP

#define DEBUG_FD_CONSTRREP_DETAILED_OUTPUT
#define DEBUG_FSET_CONSTRREP_DETAILED_OUTPUT

//#define TO_FSET_FILE
//#define TO_FD_FILE
#endif

#ifdef DEBUG_FD_CONSTRREP
#define DEBUG_FD_CONSTRREP_DETAILED_OUTPUT
#endif

#ifdef DEBUG_FSET_CONSTRREP
#define DEBUG_FSET_CONSTRREP_DETAILED_OUTPUT
#endif

extern FILE * _fdomn_file, * _fset_file;


//-----------------------------------------------------------------------------
#ifdef DEBUG_FD_CONSTRREP
#define AssertFD(C) \
if (!(C)) OZ_error("AssertFD '%s' failed at %s:%d.", #C, __FILE__, __LINE__);

#define _DEBUG_FD_IR(CODE) { print_to_fdfile CODE ; }
#define DEBUG_FD_IR(CODE) _DEBUG_FD_IR(CODE)
#define DebugCodeFD(C) C

#else

#define _DEBUG_FD_IR(CODE)
#define DEBUG_FD_IR(CODE)
#define AssertFD(C)
#define DebugCodeFD(C)

#endif

//-----------------------------------------------------------------------------

#ifdef DEBUG_FSET_CONSTRREP

#define AssertFS(C) \
if (!(C)) OZ_error("AssertFS '%s' failed at %s:%d.", #C, __FILE__, __LINE__);

#define _DEBUG_FSET_IR(CODE) { print_to_fsfile CODE ; }
#define DEBUG_FSET_IR(CODE) _DEBUG_FSET_IR(CODE)
#define FSDEBUG(X) { X; }

#else

#define AssertFS(C)
#define _DEBUG_FSET_IR(CODE)
#define DEBUG_FSET_IR(CODE)
#define FSDEBUG(X)

#endif

//-----------------------------------------------------------------------------

#endif
