#ifndef GENERIC
#define GENERIC

typedef void* GenPtr;

//#define MY_DEBUG

#ifdef MY_DEBUG

#define DEBUG(C) printf C; fflush(stdout);

#else
#define DEBUG(C)
#endif


class generic {
public:
  virtual void write() const {
    DEBUG(("generic class (should be overloaded)\r\n"));
  }
  virtual void read() const {    
  }
};

#endif
