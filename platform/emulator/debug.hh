/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, W-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr & mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

/* The Oz source and machine level debugger */

#ifndef __DEBUGH
#define __DEBUGH

enum OzDebugDoit {DBG_NOOP, DBG_STEP, DBG_NEXT};

class OzDebug {
public:
  USEFREELISTMEMORY;

  OzDebugDoit dothis;
  TaggedRef info;
  OzDebug(OzDebugDoit x, TaggedRef i) {
    dothis = x;
    info = i;
  }
  OzDebug *gcOzDebug();
  OzDebug *copy()
  {
    return new OzDebug(dothis,info);
  }

  void printCall();

  void dispose() 
  {
    freeListDispose(this,sizeof(OzDebug));
  }
};

void execBreakpoint(Thread*, Bool message=OK);
void debugStreamSuspend(ProgramCounter, Thread*, TaggedRef, TaggedRef, Bool);
void debugStreamCont(Thread*);
void debugStreamThread(Thread*,Thread* parent=NULL);
void debugStreamTerm(Thread*);
void debugStreamCall(ProgramCounter, char*, int, TaggedRef*, Bool, int);
void debugStreamExit(TaggedRef);
void debugStreamRaise(Thread*, TaggedRef);

OZ_C_proc_proto(BIdebugmode)
OZ_C_proc_proto(BItaskStack)
OZ_C_proc_proto(BIsuspendDebug)
OZ_C_proc_proto(BIrunChildren)
OZ_C_proc_proto(BIframeVariables)
OZ_C_proc_proto(BIbreakpointAt)
OZ_C_proc_proto(BIbreakpoint)
OZ_C_proc_proto(BIsetContFlag)
OZ_C_proc_proto(BIsetStepMode)
OZ_C_proc_proto(BItraceThread)
OZ_C_proc_proto(BIdisplayCode)
OZ_C_proc_proto(BIlocation)

Bool trace(char *s,Board *board=NULL,Actor *actor=NULL,
	   ProgramCounter PC=NOCODE,RefsArray Y=NULL,RefsArray G=NULL);
void tracerOn();
void tracerOff();

OZ_C_proc_proto(BIhalt)


#endif
