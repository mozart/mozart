#ifndef __LAZYVAR__H__
#define __LAZYVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "am.hh"
#include "genvar.hh"
#include "tagged.hh"
#include "value.hh"
#include "mem.hh"
#include "thread.hh"

class GenLazyVariable: public GenCVariable {
private:
  OZ_Term function;
  OZ_Term result;
public:
  GenLazyVariable(OZ_Term fun,OZ_Term res)
    :GenCVariable(LazyVariable),function(fun),result(res){}
  void gc();
  size_t getSize(void) { return sizeof(GenLazyVariable); }
  Bool unifyLazy(TaggedRef*,TaggedRef*,ByteCode*);
  // int hasFeature(TaggedRef fea,TaggedRef *out);
  Bool valid(TaggedRef /* val */) { return TRUE; }
  OZ_Term getFunction() { return function; }
  void kickLazy();
  void kickLazy(TaggedRef*);
  void addSuspLazy(Thread*,int);
};


inline
Bool isLazyVar(TaggedRef term)
{
  GCDEBUG(term);
  return isCVar(term) && (tagged2CVar(term)->getType() == LazyVariable);
}

inline
GenLazyVariable *tagged2LazyVar(TaggedRef t) {
  Assert(isLazyVar(t));
  return (GenLazyVariable *) tagged2CVar(t);
}



#endif /* __LAZYVAR__H__ */
