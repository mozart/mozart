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

class OzDictionary: public ConstTerm {
  friend void ConstTerm::gcConstRecurse(void);
private:
  DynamicTable *table;

public:
  OzDictionary() : ConstTerm(Co_Dictionary)
  {
    table = DynamicTable::newDynamicTable();
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
    if (table->fullTest()) {
      table = table->doubleDynamicTable();
    }
    (void) table->add(key,value);
  }

  void remove(TaggedRef key)
  {
    table = table->remove(key);
  }

  TaggedRef keys()
  {
    return table->getKeys();
  }

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
