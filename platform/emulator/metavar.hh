#ifndef __metavar__hh__
#define __metavar__hh__


#if defined(__GNUC__)
#pragma interface
#endif

#include "genvar.hh"
#include "oz_meta.h"

//-----------------------------------------------------------------------------
// Here goes the private part, not accessible by oz.h

char * printMetaDefault(OZ_Term);

class GenMetaVariable : public GenCVariable {
friend class GenCVariable;
private:
  TaggedRef data;
  unsigned tag;
  enum {maxmetavars = 10};
  static struct meta_vars_t {
    char * name;
    unifyMeta_t unify_data;
    printMeta_t print_data;
  } meta_vars[maxmetavars];
  static unsigned last_tag;

  Bool unifyMeta(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);
public:
  GenMetaVariable(unsigned ty, TaggedRef tr);

  static unsigned introduceMetaVar(char * name, unifyMeta_t unify_data,
                                   printMeta_t print_data = printMetaDefault);

  char * getName(void) { return meta_vars[tag].name; }
  unsigned getMetaType(void) { return tag; }
  TaggedRef getData(void) { return data; }
  void setData(TaggedRef d) { data = d; }
  size_t getSize(void) { return sizeof(GenMetaVariable); }
  void gc(void);
  char * PrintMeta(void) {
    return meta_vars[getMetaType()].print_data(data);
  }
  Bool valid(TaggedRef v);
  void constrainVar(TaggedRef v, TaggedRef d) {
    setData(d);
    propagate(v, suspList, d, pc_propagator);
  }
};


#endif
