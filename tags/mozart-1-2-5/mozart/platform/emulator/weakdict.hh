#ifndef __WEAKDICT__HH__
#define __WEAKDICT__HH__

#include "base.hh"
#include "dictionary.hh"
#include "extension.hh"

extern void gDropWeakDictionaries(void);
extern void gCollectWeakDictionariesInit(void);
extern void gCollectWeakDictionariesPreserve(void);
extern void gCollectWeakDictionariesContent(void);

class WeakDictionary : public OZ_Extension {
private:
  DynamicTable *table;
  OZ_Term stream;
  friend void gCollectWeakDictionariesPreserve(void);
  friend void gCollectWeakDictionariesContent(void);
public:
  WeakDictionary() : OZ_Extension() {}
  WeakDictionary(DynamicTable*t,OZ_Term s)
    : OZ_Extension(),table(t),stream(s) {}
  WeakDictionary(OZ_Term srm)
    : OZ_Extension(),stream(srm)
    {
      table = DynamicTable::newDynamicTable(DictDefaultSize);
    }
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
