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
/*
#define DEBUG_FD_CONSTRREP
#define DEBUG_FD
#define DEBUG_FSET
#define DEBUG_FSET_CONSTRREP
*/
extern ostream * cpi_cout;


#if defined(DEBUG_CHECK) && defined(DEBUG_FD_CONSTRREP)

#  define DEBUG_FD_IR(COND, CODE) if (COND || 1) { *cpi_cout << CODE << flush;}


#  define AssertFD(C) \
if (!(C)) OZ_error("FD assertion '%s' failed at %s:%d.", #C, __FILE__, __LINE__);

#  define DebugCodeFD(C) C

#else


#  define DEBUG_FD_IR(COND, CODE)

#  define AssertFD(C)
#  define DebugCodeFD(C)

#endif


#ifdef DEBUG_FSET_CONSTRREP

#define _DEBUG_FSETIR(CODE) (*cpi_cout) << CODE << flush;
#define DEBUG_FSETIR(CODE) _DEBUG_FSETIR(CODE)
#define FSDEBUG(X) { X; }

#else

#define _DEBUG_FSETIR(CODE)
#define DEBUG_FSETIR(CODE)
#define FSDEBUG(X)

#endif /* DEBUG_FSET */


#endif
