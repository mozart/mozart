/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: mehl
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$
  */

#ifndef __BUILTINSH
#define __BUILTINSH

#ifdef INTERFACE
#pragma interface
#endif

// specification for builtins
struct BIspec {
  char *name;
  int arity;
  OZ_CFun fun;
  IFOR ifun;
};


// add specification to builtin table
void BIaddSpec(BIspec *spec);


BuiltinTabEntry *BIinit();
BuiltinTabEntry *BIadd(char *name,int arity,OZ_CFun fun,IFOR infun=(IFOR) NULL);
BuiltinTabEntry *BIaddSpecial(char *name,int arity,BIType t);


// -----------------------------------------------------------------------
// tables

class BuiltinTab : public HashTable {
public:
  BuiltinTab(int sz) : HashTable(HT_CHARKEY,sz) {};
  ~BuiltinTab() {};
  unsigned memRequired(void) {
    return HashTable::memRequired(sizeof(BuiltinTabEntry));
  }
  char * getName(void * fp) {
    HashNode * hn = getFirst();
    for (; hn != NULL; hn = getNext(hn)) {
      BuiltinTabEntry * abit = (BuiltinTabEntry *) hn->value;
      if (abit->getInlineFun() == (IFOR) fp ||
          abit->getFun() == (OZ_CFun) fp)
        return hn->key.fstr;
    }
    return "???";
  }
};

extern BuiltinTab builtinTab;
extern State dotInline(TaggedRef term, TaggedRef fea, TaggedRef &out);
extern State uparrowInline(TaggedRef term, TaggedRef fea, TaggedRef &out);


#endif
