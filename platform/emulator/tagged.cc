/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  */

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "tagged.hh"
#endif

#include "tagged.hh"

char *TypeOfTermString[1<<tagSize];


void initTagged()
{
  void *p = (void *) malloc(100);
  void *p1 = tagValueOf(makeTaggedMisc(p));
  if (p != p1) {
    fprintf(stderr,"\n*******\nError, wrong configuration\n");
    fprintf(stderr,"Try defining\n\n");
    fprintf(stderr,"\t const long int mallocBase = 0x%lx;\n\n",
	    (long int)p - (long int)p1);
    fprintf(stderr,"in \"tagged.hh\"\n\n");
    exit(1);
  }

  free(p);

  char **tts = TypeOfTermString;
  
  tts[0]        = "REF";       //  0
  tts[UVAR]     = "UVAR";      //  1
  tts[LTUPLE]   = "LTUPLE";    //  2
  tts[SMALLINT] = "SMALLINT";  //  3
  tts[4]        = "REF";       //  4
  tts[CVAR]     = "CVAR";      //  5
  tts[STUPLE]   = "STUPLE";    //  6
  tts[BIGINT]   = "BIGINT";    //  7
  tts[8]        = "REF";       //  8
  tts[SVAR]     = "SVAR";      //  9
  tts[OZCONST]  = "OZCONST";     // 10
  tts[OZFLOAT]  = "OZFLOAT";     // 11
  tts[12]       = "REF";       // 12
  tts[GCTAG]    = "GCTAG";     // 13
  tts[SRECORD]  = "SRECORD";   // 14
  tts[LITERAL]  = "ATOM";      // 15
}


//-----------------------------------------------------------------------------
//                          class Continuation

void Continuation::setX(RefsArray x, int i)
{
  if (i <= 0 || x == NULL) {
    xRegs = NULL;
  } else {
    xRegs = allocateRefsArray(i,NO);
    while ((--i) >= 0) {
      Assert(MemChunks::isInHeap(x[i]));
      xRegs[i] = x[i];
    }
  }
}

void Continuation::setY(RefsArray Y) {
  yRegs = Y;
#ifdef DEBUG_CHECK
  if (Y != (RefsArray) 0) {
    for (int i = 0; i < getRefsArraySize(Y); i++) {
      TaggedRef aux = Y[i];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}

void Continuation::init(ProgramCounter p, RefsArray y, RefsArray g,
			RefsArray x, int i)
{
  pc = p;
  yRegs = y;
  gRegs = g;
  setX (x, i);


  DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));
#ifdef DEBUG_CHECK
  if (y != (RefsArray) 0) {
    for (int iii = 0; iii < getRefsArraySize(y); iii++) {
      TaggedRef aux = y[iii];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}

Continuation::Continuation(ProgramCounter p, RefsArray y, RefsArray g,
			   RefsArray x, int i)
  : pc(p), yRegs(y), gRegs(g)
{
  setX (x, i);

  DebugCheckT(for (int ii = 0; ii < i; ii++) CHECK_NONVAR(x[ii]));
#ifdef DEBUG_CHECK
  if (y != (RefsArray) 0) {
    for (int iii = 0; iii < getRefsArraySize(y); iii++) {
      TaggedRef aux = y[iii];
      if (aux != (TaggedRef) 0) { DEREF(aux,_ptr,_tag); }
    }
  }
#endif
}

