/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

*/


#ifndef __INDEXINGH
#define __INDEXINGH

#ifdef INTERFACE
#pragma interface
#endif

class HTEntry {
 protected:
  ProgramCounter label;
  HTEntry *next;
  union {
    Literal *literal;
    TaggedRef number;
    struct {
      Literal *fname;
      SRecordArity arity;
    } functor;
  } u;

 public:

  HTEntry(Literal *name, ProgramCounter lbl, HTEntry *nxt)
    : label(lbl), next(nxt) { u.literal = name; };

  HTEntry(TaggedRef num, ProgramCounter lbl, HTEntry *nxt)
    : label(lbl), next(nxt)
  {
    u.number = num;
    gcStaticProtect(&u.number);
  };

  HTEntry(Literal *name, SRecordArity arity, ProgramCounter lbl, HTEntry *nxt)
    : label(lbl), next(nxt) {
    u.functor.fname = name;
    u.functor.arity = arity;
  };

  HTEntry* getNext(void) {return next;}

  ProgramCounter getLabel()  {return label;}
  TaggedRef getNumber()  {return u.number;}
  Literal *getLiteral()  {return u.literal;}
  Literal *getFunctor(SRecordArity &a)
  {
    a = u.functor.arity;
    return u.functor.fname;
  }

  /* look up a literal */
  ProgramCounter lookup(Literal *name, ProgramCounter elseLabel)
  {
    ProgramCounter  ret = elseLabel;

    for (HTEntry *help = this; help != NULL; help = help->next) {
      if (help->u.literal == name) {
        ret = help->label;
        break;
      }
    }
    return ret;
  }

  /* look up functor/arity */
  ProgramCounter lookup(Literal *name, SRecordArity arity, ProgramCounter elseLabel)
  {
    ProgramCounter ret = elseLabel;

    for (HTEntry *help = this; help != NULL; help = help->next) {
      if ( (help->u.functor.fname == name) &&
           sameSRecordArity(help->u.functor.arity,arity) ) {
        ret = help->label;
        break;
      }
    }
    return ret;
  }

  /* look a number */
  ProgramCounter lookup(TaggedRef term, ProgramCounter elseLabel)
  {
    Assert(isNumber(term));
    ProgramCounter ret = elseLabel;

    for (HTEntry *help = this; help != NULL; help = help->next) {
      if (numberEq(help->u.number,term)) {
        ret = help->label;
        break;
      }
    }
    return ret;
  }
};


typedef HTEntry** EntryTable;

class IHashTable {
 public:
  int size;      // is always a power of 2
  int hashMask;  // always size-1
  int numentries;

  EntryTable literalTable;
  EntryTable functorTable;
  EntryTable numberTable;

  ProgramCounter elseLabel;
  ProgramCounter listLabel;
  ProgramCounter varLabel;

  IHashTable(int sz, ProgramCounter elseLbl) {
    numentries = 0;
    size = nextPowerOf2(sz);
    hashMask = size-1;
    literalTable = functorTable = numberTable = NULL;
    elseLabel = elseLbl;
    listLabel = elseLabel;
    varLabel  = elseLabel;
  };

  void add(TaggedRef number, ProgramCounter label);
  void add(Literal *constant, ProgramCounter label);
  void add(Literal *functor, SRecordArity arity, ProgramCounter label);
  void addVar(ProgramCounter label)  { varLabel  = label; }
  void addList(ProgramCounter label) { listLabel = label; }

  int hash(int n) { return (n & hashMask); }  // return a value n with 0 <= n < size
  ProgramCounter getElse() { return elseLabel; }

  Bool disentailed(GenCVariable *var, TaggedRef *varPtr);
};

ProgramCounter switchOnTermOutline(TaggedRef term, TaggedRef *termPtr,
                                   IHashTable *table, TaggedRef *&sP);

#endif
