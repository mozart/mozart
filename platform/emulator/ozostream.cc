#include "ozostream.hh"


ozostream& ends(ozostream& outs)  { return outs.ends();}
ozostream& endl(ozostream& outs)  { return outs.endl(); }
ozostream& flush(ozostream& outs) { return outs.flush(); }


ozostream ozcout(stdout), ozcerr(stderr);


ozostream &ozostream::operator << (char *s)
{
  Assert(fd);
  fprintf(fd,s);
  return *this;
}

ozostream &ozostream::operator << (void *p)
{
  Assert(fd);
  fprintf(fd,"0x%x",p);
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
  fprintf(fd,"%d",i);
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


ozostream &ozstrstream::operator << (char *s)
{
  while(*s) {
    set(*s++);
  }
  return *this;
}

ozostream &ozstrstream::operator << (void *p)
{
  char buf[100];
  sprintf(buf,"0x%x",p);
  (*this) << buf;
  return *this;
}

ozostream &ozstrstream::operator << (long i)
{
  char buf[100];
  sprintf(buf,"%d",i);
  (*this) << buf;
  return *this;
}

ozostream &ozstrstream::operator << (double f)
{
  char buf[100];
  sprintf(buf,"%f",f);
  (*this) << buf;
  return *this;
}
