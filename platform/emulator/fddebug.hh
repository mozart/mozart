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

#ifndef __FDDEBUG_HH__
#define __FDDEBUG_HH__


#if defined(DEBUG_CHECK) && defined(DEBUG_FD)

extern "C" {
void error( const char *format ...);
}

extern ostream * cpi_cout;

#  define DEBUG_FD_IR(COND, CODE) if (COND) { *cpi_cout << CODE << flush;}

#  define AssertFD(C) \
if (!(C)) error("FD assertion '%s' failed at %s:%d.", #C, __FILE__, __LINE__);

#  define DebugCodeFD(C) C

#else

#  define DEBUG_FD_IR(COND, CODE)
#  define AssertFD(C)
#  define DebugCodeFD(C)

#endif


#  define FORCE_ALL 0

#endif
