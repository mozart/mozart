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
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */


/* The Oz source and machine level debugger */

#ifndef __DEBUGH
#define __DEBUGH

/* The Oz source level debugger */

class OzDebug {
public:
  USEFREELISTMEMORY;

  ProgramCounter PC;
  RefsArray Y, G;
  TaggedRef data;
  RefsArray arguments;

  OzDebug(ProgramCounter pc, RefsArray y, RefsArray g) {
    PC        = pc;
    Y         = y;
    G         = g;
    data      = makeTaggedNULL();
    arguments = (RefsArray) NULL;
  }

  OzDebug(const OzDebug &d) {
    PC        = d.PC;
    Y         = d.Y;
    G         = d.G;
    data      = d.data;
    arguments = d.arguments == (RefsArray) NULL ?
                      (RefsArray) NULL : copyRefsArray(d.arguments);
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
void debugStreamThread(Thread*, Thread* parent=NULL);
void debugStreamBlocked(Thread*);
void debugStreamReady(Thread*);
void debugStreamTerm(Thread*);
void debugStreamException(Thread*, TaggedRef);
void debugStreamEntry(OzDebug*, int);
void debugStreamExit(OzDebug*, int);
void debugStreamUpdate(Thread*);

OZ_C_proc_proto(BIdebugmode)
OZ_C_proc_proto(BIaddEmacsThreads)
OZ_C_proc_proto(BIaddSubThreads)
OZ_C_proc_proto(BIgetDebugStream)
OZ_C_proc_proto(BIthreadUnleash)
OZ_C_proc_proto(BIsetContFlag)
OZ_C_proc_proto(BIsetStepFlag)
OZ_C_proc_proto(BIsetTraceFlag)
OZ_C_proc_proto(BIcheckStopped)

OZ_C_proc_proto(BIbreakpointAt)
OZ_C_proc_proto(BIbreakpoint)
OZ_C_proc_proto(BIdisplayCode)
OZ_C_proc_proto(BIprocedureCode)
OZ_C_proc_proto(BIlivenessX)


/* The Oz machine level debugger */

#ifdef DEBUG_TRACE
Bool trace(char *s,Board *board=NULL,Actor *actor=NULL,
           ProgramCounter PC=NOCODE,RefsArray Y=NULL,RefsArray G=NULL);
void tracerOn();
void tracerOff();

OZ_C_proc_proto(BIhalt)
#endif

#endif
