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

#if fertig
enum OzDebugDoit {DBG_NEXT};

class OzDebug {
public:
  OzDebugDoit dothis;
  OzDebug(OzDebugDoit x)
  {
    ;
  }

  OzDebug *gcOzDebug();
  void printCall();
};
#endif

class OzDebug {
public:
  static unsigned long goalCounter;
  TaggedRef pred;   // really a Abstraction* or a Builtin*
  TaggedRef *args;
  unsigned long goalNum;
  OzDebug(TaggedRef p, int arity, TaggedRef *a)
  {
    pred = p;
    args = arity==0 ? (RefsArray) NULL : copyRefsArray(a,arity);
    goalNum = goalCounter++;
  }

  OzDebug *gcOzDebug();
  void printCall();
};


void enterCall(Board *b, TaggedRef def,int arity, TaggedRef *args);
void exitCall(OZ_Return,OzDebug *);
void exitBuiltin(OZ_Return, TaggedRef bi, int arity, TaggedRef *args);

void debugStreamThread(Thread*);
void debugStreamTerm(Thread*, TaggedRef);
void debugStreamCall(ProgramCounter, char*, int, TaggedRef*);

OZ_C_proc_proto(BItaskStack)
OZ_C_proc_proto(BIlocation)
OZ_C_proc_proto(BIgetThreadByID)
OZ_C_proc_proto(BIspy)
OZ_C_proc_proto(BInospy)
OZ_C_proc_proto(BItraceOn)
OZ_C_proc_proto(BItraceOff)
OZ_C_proc_proto(BIdisplayCode)
OZ_C_proc_proto(BIbreakpoint)

Bool trace(char *s,Board *board=NULL,Actor *actor=NULL,
	   ProgramCounter PC=NOCODE,RefsArray Y=NULL,RefsArray G=NULL);
void tracerOn();
void tracerOff();


#endif
