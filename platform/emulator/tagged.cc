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

#ifdef DEBUG_CHECK
char *TypeOfTermString[1<<tagSize];
#endif


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

#ifdef DEBUG_CHECK
  char **tts = TypeOfTermString;
  
  tts[0]        = "REF";       //  0
  tts[UVAR]     = "UVAR";      //  1
  tts[LTUPLE]   = "LTUPLE";    //  2
  tts[SMALLINT] = "SMALLINT";  //  3
  tts[4]        = "REF";       //  4
  tts[CVAR]     = "CVAR";      //  5
  tts[6]        = "UNUSED";    //  6
  tts[BIGINT]   = "BIGINT";    //  7
  tts[8]        = "REF";       //  8
  tts[SVAR]     = "SVAR";      //  9
  tts[OZCONST]  = "OZCONST";   // 10
  tts[OZFLOAT]  = "OZFLOAT";   // 11
  tts[12]       = "REF";       // 12
  tts[GCTAG]    = "GCTAG";     // 13
  tts[SRECORD]  = "SRECORD";   // 14
  tts[LITERAL]  = "ATOM";      // 15
#endif
}



