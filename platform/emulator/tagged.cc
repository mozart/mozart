/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  */

#ifdef __GNUC__
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
    fprintf(stderr,"\t const int mallocBase = 0x%x;\n\n",
	    (int)p - (int)p1);
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
  tts[CONST]    = "CONST";     // 10
  tts[FLOAT]    = "FLOAT";     // 11
  tts[12]       = "REF";       // 12
  tts[GCTAG]    = "GCTAG";     // 13
  tts[SRECORD]  = "SRECORD";   // 14
  tts[LITERAL]  = "ATOM";      // 15
}
