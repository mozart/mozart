/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *
 *  Copyright:
 *    Ralf Scheidhauer 1999
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

#if defined(INTERFACE)
#pragma implementation "tagged.hh"
#endif

#include "tagged.hh"

#ifdef DEBUG_CHECK

void initTagged(void) {
  void * p1 = (void *) malloc(100);
  void * p2 = tagValueOf(makeTaggedMiscp(p1));

  if (p1 != p2) {
    fprintf(stderr,"\n*******\nError, wrong configuration\n");
    fprintf(stderr,"Try defining\n\n");
    fprintf(stderr,"\t const long int mallocBase = 0x%lx;\n\n",
            (long int)p1 - (long int)p2);
    fprintf(stderr,"in \"tagged.hh\"\n\n");
    exit(1);
  }

  free(p1);

}

#endif
