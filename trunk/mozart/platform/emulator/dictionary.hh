/*
 * FBPS Saarbr"ucken
 * Author: mehl
 * Last modified: $Date$ from $Author$
 * Version: $Revision$
 * State: $State$
 *
 * Values: literal, list, records
 */

#ifndef __DICTIONARYHH
#define __DICTIONARYHH

#ifdef INTERFACE
#pragma interface
#endif

/*===================================================================
 * Dictionaries
 *=================================================================== */

class OzDictionary: public ConstTermWithHome {
  friend void ConstTerm::gcConstRecurse(void);
private:
  DynamicTable *table;
  Bool isSafe; // Perdio: safe dictionaries (i.e. those used
               // within objects) can be marshalled

public:
  OzDictionary();
  ~OzDictionary();
  OzDictionary(OzDictionary&);
  OzDictionary(Board *b, int sz=4) : ConstTermWithHome(b,Co_Dictionary) 
  {
    table = DynamicTable::newDynamicTable(sz);
    isSafe = NO;
  }
  OzDictionary(Board *b, DynamicTable *t) : ConstTermWithHome(b,Co_Dictionary)
  {
    table = t->copyDynamicTable();
    isSafe = NO;
  }
  
  OZ_Return getArg(TaggedRef key, TaggedRef &out) 
  { 
    TaggedRef ret = table->lookup(key);
    if (ret == makeTaggedNULL())
      return FAILED;
    out = ret;
    return PROCEED;
  }

  TaggedRef member(TaggedRef key) 
  { 
    TaggedRef found = table->lookup(key);
    return (found == makeTaggedNULL()) ? NameFalse : NameTrue;
  }

  void setArg(TaggedRef key, TaggedRef value)
  { 
    if (table->fullTest()) resizeDynamicTable(table);
    Bool valid=table->add(key,value);
    if (!valid) {
      resizeDynamicTable(table);
      valid = table->add(key,value);
    }
    Assert(valid);
  }

  void remove(TaggedRef key) { table = table->remove(key); }

  TaggedRef keys()  { return table->getKeys(); }
  TaggedRef pairs() { return table->getPairs(); }
  TaggedRef items() { return table->getItems(); }

  Bool isSafeDict() { return isSafe; }
  void markSafe()   { isSafe=OK; }
  int getSize()     { return table->numelem; }

  TaggedRef clone() {
    OzDictionary *aux = new OzDictionary(getBoard(), table);
    if (isSafe) 
      aux->markSafe();
    return makeTaggedConst(aux);
  }

  TaggedRef toRecord(TaggedRef lbl) { return table->toRecord(lbl); }

  // iteration
  int getFirst()            { return table->getFirst(); }
  int getNext(int i)        { return table->getNext(i); }
  TaggedRef getKey(int i)   { return table->getKey(i); }
  TaggedRef getValue(int i) { return table->getValue(i); }

  OZPRINT;
  OZPRINTLONG;
};


inline
Bool isDictionary(TaggedRef term)
{
  return isConst(term) && tagged2Const(term)->getType() == Co_Dictionary;
}

inline
OzDictionary *tagged2Dictionary(TaggedRef term)
{
  Assert(isDictionary(term));
  return (OzDictionary *) tagged2Const(term);
}

#endif
