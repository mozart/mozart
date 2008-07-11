#ifndef __WEAKDICT__HH__
#define __WEAKDICT__HH__

#include "base.hh"
#include "var_of.hh"
#include "extension.hh"

extern void gDropWeakDictionaries(void);
extern void gCollectWeakDictionariesInit(void);
extern void gCollectWeakDictionariesPreserve(void);
extern bool gCollectWeakDictionariesHasMore(void);
extern void gCollectWeakDictionariesContent(void);

class WeakDictionary : public OZ_Extension {
private:
  DynamicTable *table;
  OZ_Term stream;
  friend void gCollectWeakDictionariesPreserve(void);
  friend void gCollectWeakDictionariesContent(void);
  WeakDictionary() : OZ_Extension() {}
public:
  WeakDictionary(DynamicTable*t,OZ_Term s);
  WeakDictionary(OZ_Term srm);
  virtual int getIdV() { return OZ_E_WEAKDICTIONARY; }
  virtual OZ_Term typeV() { return OZ_atom("weakDictionary"); }
  virtual OZ_Extension* gCollectV();
  virtual void gCollectRecurseV(void);
  virtual OZ_Extension* sCloneV();
  virtual void sCloneRecurseV(void);
  virtual OZ_Term printV(int = 10);
  void close();
  void put(OZ_Term,OZ_Term);
  OZ_Boolean get(OZ_Term,OZ_Term&);
  void weakGC();
  //
  OZ_Term getKeys();
  OZ_Term getPairs();
  OZ_Term getItems();
  bool isEmpty();
  OZ_Term toRecord(OZ_Term);
  void remove(OZ_Term);
  void remove_all();
  bool member(OZ_Term);
  //
  // support for `.' and `:='
  //
  virtual OZ_Return getFeatureV(OZ_Term,OZ_Term&);
  virtual OZ_Return putFeatureV(OZ_Term,OZ_Term );
};

#endif
