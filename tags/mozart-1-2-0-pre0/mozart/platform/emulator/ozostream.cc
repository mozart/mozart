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


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "ozostream.hh"
#endif

#include <errno.h>

#include "base.hh"
#include "ozostream.hh"
#include "os.hh"

ozostream& ends(ozostream& outs)  { return outs.ends();}
ozostream& endl(ozostream& outs)  { return outs.endl(); }
ozostream& flush(ozostream& outs) { return outs.flush(); }


ozostream ozcout(STDOUT_FILENO), ozcerr(STDERR_FILENO);


ozostream &ozostream::operator << (const char *s)  
{
  Assert(fd!=-1);
  union { char *s1; const char *s2; } u;
  u.s2 = s;
  if (ossafewrite(fd,u.s1,strlen(s))<0) {
    perror("ozostream write");
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
  if (ossafewrite(fd,&c,1)<0) {
    perror("ozostream write");
  }
  return *this;
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
  sprintf(buf,"%g",d);
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


