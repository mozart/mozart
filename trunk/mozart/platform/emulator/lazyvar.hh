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
  void kickLazy();
  void kickLazy(TaggedRef*);
  void addSuspLazy(Thread*);
};

#endif /* __LAZYVAR__H__ */
