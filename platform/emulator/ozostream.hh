#ifndef OSTREAMH
#define OSTREAMH

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
  virtual ozostream &operator << (unsigned char c) { return (*this) << (char) c; }
  virtual ozostream &operator << (int i);
  virtual ozostream &operator << (double f);
  virtual ozostream &operator << (unsigned int i) { return (*this) << (int) i; }
  virtual ozostream &flush() { fflush(fd); return (*this); }
  virtual ozostream &ends() { return (*this) << '\0'; }
  virtual ozostream &endl() { return (*this) << "\n"; }
  virtual ozostream &operator<<(ozostream& (*func)(ozostream&)) {
    return (*func)(*this); }
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

  void reset() { cur = 0; }
  int pcount() { return cur; }
  char *str()  { set('\0'); return string; }

  void resize();

  virtual ozstrstream &operator << (char *s);
  virtual ozstrstream &operator << (void *p);
  virtual ozstrstream &operator << (char c)   { set(c); return (*this); }
  virtual ozstrstream &operator << (unsigned char c) { return (*this) << (char) c; }
  virtual ozstrstream &operator << (int i);
  virtual ozstrstream &operator << (unsigned int i) { return (*this)<< (int) i; }
  virtual ozstrstream &operator << (double f);
  virtual ozstrstream &flush() { return (*this); }
  virtual ozstrstream &ends() { return (*this) << '\0'; }
  virtual ozstrstream &endl() { return (*this) << "\n"; }
  virtual ozstrstream& operator<<(ozstrstream& (*func)(ozstrstream&)) {
    return (*func)(*this); }
};

extern ozostream ozcout, ozcerr;

extern ozostream& ends(ozostream& outs);
extern ozostream& endl(ozostream& outs);
extern ozostream& flush(ozostream& outs);

extern ozstrstream& ends(ozstrstream& outs);
extern ozstrstream& endl(ozstrstream& outs);
extern ozstrstream& flush(ozstrstream& outs);

#endif
