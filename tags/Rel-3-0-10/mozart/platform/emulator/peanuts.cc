/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
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

// For faster compilation: includes must be parsed only once

#define PEANUTS
#ifdef INTERFACE

#pragma implementation "trace.hh"
#pragma implementation "perdiovar.hh"
#pragma implementation "fdgenvar.hh"
#pragma implementation "fdhook.hh"
#pragma implementation "genvar.hh"
#pragma implementation "indexing.hh"
#pragma implementation "mem.hh"
#pragma implementation "solve.hh"
#pragma implementation "statisti.hh"
#pragma implementation "lps.hh"
#pragma implementation "taskstk.hh"
#pragma implementation "variable.hh"
#pragma implementation "thrqueue.hh"
#pragma implementation "thrspool.hh"

#endif

#include "wsock.hh"

#include "am.cc"
#include "codearea.cc"
#include "trace.cc"
#include "perdiovar.cc"
#include "fdgenvar.cc"
#include "fdhook.cc"
#include "genvar.cc"
#include "indexing.cc"
#include "mem.cc"
#include "solve.cc"
#include "statisti.cc"
#include "lps.cc"
#include "taskstk.cc"
#include "variable.cc"
#include "thrqueue.cc"
#include "thrspool.cc"
#include "print.cc"
