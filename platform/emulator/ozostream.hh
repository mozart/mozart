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
#ifndef OSTREAMH
#define OSTREAMH

#ifdef INTERFACE
#pragma interface
#endif

#include <stdlib.h>
#include <stdio.h>

#define ostream ozostream
#define cout ozcout
#define cerr ozcerr

class ozostream {
  int fd;
public:
  ozostream(int f) { fd=f;}
  virtual ozostream &operator << (const char *s);
  virtual ozostream &operator << (char c); 

  ozostream &operator << (const void *p);
  ozostream &operator << (long i);
  ozostream &operator << (double f);

  virtual ozostream &operator<<(ozostream& (*func)(ozostream&)) { 
    return (*func)(*this); }

  ozostream &flush() { return (*this); }
  ozostream &ends()  { return (*this) << '\0'; }
  ozostream &endl()  { return (*this) << "\n"; }

  ozostream &operator << (unsigned char c) { return (*this) << (char) c; }
  ozostream &operator << (unsigned int i)  { return (*this) << (long) i; }
  ozostream &operator << (int i)           { return (*this) << (long) i; }
  ozostream &operator << (unsigned long i) { return (*this) << (long) i; }
};


#define ostrstream ozstrstream

class ozstrstream: public ozostream {
  char *string;
  int size;
  int cur;

  void set(char c) 
  {
    if (cur>=size) {
      resize();
    }
    string[cur++] = c;
  }

public:
  virtual ~ozstrstream() { free(string); }

  ozstrstream() : ozostream(-1), size(100), cur(0) {
    string = (char*) malloc(size*sizeof(char));
  }

  void resize();
  void reset() { cur = 0; }
  int pcount() { return cur; }
  char *str()  { set('\0'); return string; }

  ozostream &operator << (const char *s);
  ozostream &operator << (char c)   { set(c); return (*this); }
  ozostream &operator<<(ozostream& (*func)(ozostream&)) { 
    return (*func)(*this); }

  ozostream &operator << (const void *p) { return (*this).ozostream::operator <<(p); }
  ozostream &operator << (long i)        { return (*this).ozostream::operator <<(i); }
  ozostream &operator << (double f)      { return (*this).ozostream::operator <<(f); }

};

extern ozostream ozcout, ozcerr;

extern ozostream& ends(ozostream& outs);
extern ozostream& endl(ozostream& outs);
extern ozostream& flush(ozostream& outs);

#endif
