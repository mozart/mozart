/*
 *  Authors:
 *    Ralf Scheidhauer (scheidhr@dfki.de)
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


/*
 * wrapper for syslets. Does something like
 *     "exec ozengine $0 $@"
 */

#include <windows.h>
#include <process.h>

void main(int argc, char **argv)
{
  char buffer[5000];
  buffer[0] = '"';
  GetModuleFileName(NULL, &buffer[1], sizeof(buffer) - 2);
  strcat(buffer, "\"");

  char **newargs = new char*[argc+1];

  newargs[0] = "ozengine";
  newargs[1] = buffer;
  int i=2;
  for(; i<=argc; i++) {
    newargs[i] = argv[i-1];
  }
  newargs[i] = 0;

  return spawnvp(P_WAIT,newargs[0],newargs);
}
