/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, W-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr & mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

/* The Oz source and machine level debugger */

#include "term.hh"

class OzDebug {
public:
  static unsigned long goalCounter;
  SRecord *pred;   // really a Abstraction* or a Builtin*
  TaggedRef *args;
  unsigned long goalNum;
  OzDebug(SRecord *p, int arity, TaggedRef *a)
  {
    pred = p;
    args = arity==0 ? NULL : copyRefsArray(a,arity);
    goalNum = goalCounter++;
  }

  OzDebug *gcOzDebug();
  void printCall();
};


void enterCall(Board *b, SRecord *def,int arity, TaggedRef *args);
void exitCall(OZ_Bool,OzDebug *);
void exitBuiltin(OZ_Bool,Builtin *bi, int arity, TaggedRef *args);

extern "C" {
  OZ_Bool BIspy(int,TaggedRef*);
  OZ_Bool BInospy(int,TaggedRef*);
  OZ_Bool BItraceOn(int,TaggedRef*);
  OZ_Bool BItraceOff(int,TaggedRef*);
}


Bool trace(char *s,Board *board=NULL,Actor *actor=NULL,
           ProgramCounter PC=NOCODE,RefsArray Y=NULL,RefsArray G=NULL);
void tracerOn();
void tracerOff();
