#ifndef OSTREAMH
#define OSTREAMH

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "types.hh"

#define ostream ozostream
#define cout ozcout
#define cerr ozcerr

class ozostream {
  FILE *fd;
public:
  ozostream(FILE *f) { fd=f;}
  virtual ozostream &operator << (char *s);
  virtual ozostream &operator << (void *p);
  virtual ozostream &operator << (char c);
  virtual ozostream &operator << (long i);
  virtual ozostream &operator << (double f);
  ozostream &operator << (unsigned char c) { return (*this) << (char) c; }
  virtual ozostream &flush() { fflush(fd); return (*this); }
  virtual ozostream &ends() { return (*this) << '\0'; }
  virtual ozostream &endl() { return (*this) << "\n"; }
  virtual ozostream &operator<<(ozostream& (*func)(ozostream&)) {
    return (*func)(*this); }
  ozostream &operator << (unsigned int i)  { return (*this) << (long) i; }
  ozostream &operator << (int i)           { return (*this) << (long) i; }
  ozostream &operator << (unsigned long i) { return (*this) << (long) i; }
};


#define ostrstream ozstrstream

class ozstrstream: public ozostream {
  char *string;
  int size;
  int cur;
public:
  ~ozstrstream() { free(string); }

  ozstrstream() : ozostream(0), cur(0), size(100) {
    string = (char*) malloc(size*sizeof(char));
  }

  void set(char c)
  {
    if (cur>=size) {
      resize();
    }
    string[cur++] = c;
  }

  void resize();
  void reset() { cur = 0; }
  int pcount() { return cur; }
  char *str()  { set('\0'); return string; }

  virtual ozostream &operator << (char *s);
  virtual ozostream &operator << (void *p);
  virtual ozostream &operator << (char c)   { set(c); return (*this); }
  virtual ozostream &operator << (long i);
  virtual ozostream &operator << (double f);
  virtual ozostream &flush() { return (*this); }
  virtual ozostream &operator<<(ozostream& (*func)(ozostream&)) {
    return (*func)(*this); }
};

extern ozostream ozcout, ozcerr;

extern ozostream& ends(ozostream& outs);
extern ozostream& endl(ozostream& outs);
extern ozostream& flush(ozostream& outs);

#endif
