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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */
#include <errno.h>

#include "ozostream.hh"
#include "base.hh"

ozostream& ends(ozostream& outs)  { return outs.ends();}
ozostream& endl(ozostream& outs)  { return outs.endl(); }
ozostream& flush(ozostream& outs) { return outs.flush(); }


ozostream ozcout(stdout), ozcerr(stderr);


ozostream &ozostream::operator << (const char *s)  
{
  Assert(fd);
 loop:
  if (fprintf(fd,s) < 0) {
    if (errno == EINTR) goto loop;
    perror("fprintf");
  }
  return *this;
}

ozostream &ozostream::operator << (const void *p)
{
  char buf[100];
  sprintf(buf,"%p",p);
  return *this << buf;
}


ozostream &ozostream::operator << (char c)  
{
  char buf[100];
  sprintf(buf,"%c",c);
  return *this << buf;
}

ozostream &ozostream::operator << (long i)    
{
  char buf[100];
  sprintf(buf,"%ld",i);
  return *this << buf;
}


ozostream &ozostream::operator << (double d)    
{
  char buf[100];
  sprintf(buf,"%f",d);
  return *this << buf;
}


void ozstrstream::resize()
{
  size = (size*3)/2;
  string = (char *)realloc(string, size * sizeof(char));
}


ozostream &ozstrstream::operator << (const char *s) 
{
  while(*s) {
    set(*s++);
  }
  return *this;
}


