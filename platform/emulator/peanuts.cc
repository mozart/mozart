/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: many
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  For faster compilation: includes must be parsed only once
  ------------------------------------------------------------------------
*/

#define PEANUTS
#ifdef INTERFACE

#pragma implementation "am.hh"
#pragma implementation "assemble.hh"
#pragma implementation "avar.hh"
#pragma implementation "debug.hh"
#pragma implementation "perdiovar.hh"
#pragma implementation "cpbag.hh"
#pragma implementation "error.hh"
#pragma implementation "fdgenvar.hh"
#pragma implementation "fdhook.hh"
#pragma implementation "fdprofil.hh"
#pragma implementation "genvar.hh"
#pragma implementation "indexing.hh"
#pragma implementation "mem.hh"
#pragma implementation "solve.hh"
#pragma implementation "statisti.hh"
#pragma implementation "lps.hh"
#pragma implementation "susplist.hh"
#pragma implementation "taskstk.hh"
#pragma implementation "thread.hh"
#pragma implementation "variable.hh"
#pragma implementation "thrqueue.hh"
#pragma implementation "thrspool.hh"
#pragma implementation "lazyvar.hh"

#endif

#include "wsock.hh"

#include "am.cc"
#include "assemble.cc"
#include "avar.cc"
#include "codearea.cc"
#include "debug.cc"
#include "perdiovar.cc"
#include "cpbag.cc"
#include "error.cc"
#include "fdgenvar.cc"
#include "fdhook.cc"
#include "fdprofil.cc"
#include "genvar.cc"
#include "indexing.cc"
#include "mem.cc"
#include "solve.cc"
#include "statisti.cc"
#include "lps.cc"
#include "susplist.cc"
#include "taskstk.cc"
#include "thread.cc"
#include "variable.cc"
#include "thrqueue.cc"
#include "thrspool.cc"
#include "lazyvar.cc"
#include "print.cc"
