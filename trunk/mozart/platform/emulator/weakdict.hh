#ifndef __WEAKDICT__HH__
#define __WEAKDICT__HH__

#include "base.hh"
#include "dictionary.hh"
#include "extension.hh"

extern void gCollectWeakDictionaries(void);

class WeakDictionary : public OZ_SituatedExtension {
private:
  DynamicTable *table;
  OZ_Term stream;
  WeakDictionary* next;
  friend void gCollectWeakDictionaries(void);
public:
  WeakDictionary();
  WeakDictionary(DynamicTable*t,OZ_Term s)
    : OZ_SituatedExtension(),table(t),stream(s) {}
  WeakDictionary(OZ_Term srm)
    : OZ_SituatedExtension(),stream(srm)
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
};

#endif
