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
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE) && !defined(VAL_ALL)
#pragma implementation "tagged.hh"
#endif

#include "tagged.hh"

#ifdef DEBUG_CHECK
char *TypeOfTermString[1<<tagSize];
#endif

#ifdef DEBUG_REF
int debugRef=1;
#endif

#ifdef DEBUG_CHECK

void initTagged()
{
  void *p = (void *) malloc(100);
  void *p1 = tagValueOf(makeTaggedMiscp(p));
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
  
  tts[REF]      = "REF";       //  0
  tts[REFTAG2]  = "REF2";      //  4
  tts[REFTAG3]  = "REF3";      //  8
  tts[REFTAG4]  = "REF4";      //  12
  tts[UVAR]     = "UVAR";      //  1
  tts[LTUPLE]   = "LTUPLE";    //  2
  tts[SMALLINT] = "SMALLINT";  //  3
  tts[CVAR]     = "CVAR";      //  5
  tts[FSETVALUE]= "FSETVALUE"; //  6
  tts[EXT]      = "EXTENSION"; //  7
  tts[UNUSED_VAR] = "";        //  9 // FUT
  tts[OZCONST]  = "OZCONST";   // 10
  tts[OZFLOAT]  = "OZFLOAT";   // 11
  tts[GCTAG]    = "GCTAG";     // 13
  tts[SRECORD]  = "SRECORD";   // 14
  tts[LITERAL]  = "ATOM";      // 15
}

#endif


