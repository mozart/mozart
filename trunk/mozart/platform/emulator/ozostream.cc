/*
 *  Authors:
 *    Author's name (Author's email address)
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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */
#include "ozostream.hh"


ozostream& ends(ozostream& outs)  { return outs.ends();}
ozostream& endl(ozostream& outs)  { return outs.endl(); }
ozostream& flush(ozostream& outs) { return outs.flush(); }


ozostream ozcout(stdout), ozcerr(stderr);


ozostream &ozostream::operator << (const char *s)  
{
  Assert(fd); 
  fprintf(fd,s);
  return *this;
}

ozostream &ozostream::operator << (const void *p)
{
  Assert(fd);
  fprintf(fd,"0x%p",p);
  return *this;
}


ozostream &ozostream::operator << (char c)  
{
  Assert(fd);
  fprintf(fd,"%c",c);
  return *this;
}

ozostream &ozostream::operator << (long i)    
{
  Assert(fd); 
  fprintf(fd,"%ld",i);
  return *this;
}

ozostream &ozostream::operator << (double f) 
{
  Assert(fd); 
  fprintf(fd,"%f",f); 
  return *this;
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

ozostream &ozstrstream::operator << (const void *p)  
{ 
  char buf[100];
  sprintf(buf,"0x%p",p);
  (*this) << buf;
  return *this;
}

ozostream &ozstrstream::operator << (long i)
{
  char buf[100];
  sprintf(buf,"%ld",i);
  (*this) << buf;
  return *this;
}

ozostream &ozstrstream::operator << (double f)
{
  char buf[100];
  sprintf(buf,"%g",f);
  (*this) << buf;
  return *this;
}

