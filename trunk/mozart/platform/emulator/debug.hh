/*
 *  Authors:
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 *    Benjamin Lorenz (lorenz@ps.uni-sb.de)
 *    Michael Mehl (mehl@dfki.de)
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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */


/* The Oz source and level debugger */

#ifndef __DEBUGH
#define __DEBUGH

#include "base.hh"
#include "value.hh"

/* The Oz source level debugger */

class OzDebug {
public:
  USEFREELISTMEMORY;

  ProgramCounter PC;
  RefsArray Y;
  Abstraction *CAP;
  TaggedRef data;
  RefsArray arguments;

  OzDebug(ProgramCounter pc, RefsArray y, Abstraction *cap) {
    PC        = pc;
    Y         = y;
    CAP       = cap;
    data      = makeTaggedNULL();
    arguments = (RefsArray) NULL;
  }

  OzDebug(const OzDebug &d) {
    PC        = d.PC;
    Y         = d.Y;
    CAP       = d.CAP;
    data      = d.data;
    arguments = d.arguments == (RefsArray) NULL ?
		      (RefsArray) NULL : copyRefsArray(d.arguments);
  }

  void setSingleArgument(TaggedRef x) {
    arguments = allocateRefsArray(2,NO);
    arguments[0] = x;
    arguments[1] = makeTaggedNULL();
  }

  // providing a frameId means that the variable names of the
  // procedure may be found by looking into the thread stack;
  // not providin a frameId (-1) means that the variable names
  // are directly adjoined to the record
  TaggedRef toRecord(const char *label, Thread *thread, int frameId = -1);

  TaggedRef getFrameVariables();

  OzDebug *gcOzDebug();

  void dispose() 
  {
    if (arguments != (RefsArray) NULL)
      disposeRefsArray(arguments);
    freeListDispose(this,sizeof(OzDebug));
  }
};

void execBreakpoint(Thread*);
void debugStreamBreakpoint(Thread*);
void debugStreamBlocked(Thread*);
void debugStreamReady(Thread*);
void debugStreamTerm(Thread*);
void debugStreamException(Thread*, TaggedRef);
void debugStreamEntry(OzDebug*, int);
void debugStreamExit(OzDebug*, int);
void debugStreamUpdate(Thread*);

OZ_BI_proto(BIgetDebugStream);
OZ_BI_proto(BIthreadUnleash);
OZ_BI_proto(BIsetContFlag);
OZ_BI_proto(BIsetStepFlag);
OZ_BI_proto(BIsetTraceFlag);
OZ_BI_proto(BIcheckStopped);

OZ_BI_proto(BIbreakpointAt);
OZ_BI_proto(BIbreakpoint);
OZ_BI_proto(BIdisplayDef);
OZ_BI_proto(BIdisplayCode);
OZ_BI_proto(BIprocedureCode);
OZ_BI_proto(BIprocedureCoord);
OZ_BI_proto(BIlivenessX);


#endif /* __DEBUGH */
