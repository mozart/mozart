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

#include "term.hh"
#include "bignum.hh"

class HTEntry {
 protected:
  ProgramCounter label;
  HTEntry *next;
  union {
    Atom *atom;
    TaggedRef number;
    struct {
      Atom *fname;
      int arity;
    } functor;
  };

 public:

  HTEntry(Atom *name, ProgramCounter lbl, HTEntry *nxt)
    : atom(name), label(lbl), next(nxt) {};

  HTEntry(TaggedRef num, ProgramCounter lbl, HTEntry *nxt)
    : number(num), label(lbl), next(nxt)
  {
    OZ_protect(&number);
  };

  HTEntry(Atom *name, int arity, ProgramCounter lbl, HTEntry *nxt)
    : label(lbl), next(nxt) {
    functor.fname = name;
    functor.arity = arity;
  };

  HTEntry* getNext(void) {return next;}

  /* look up an atom */
  ProgramCounter lookup(Atom *name, ProgramCounter elseLabel)
    {
      ProgramCounter  ret = elseLabel;

      for (HTEntry *help = this; help != NULL; help = help->next) {
        if (help->atom == name) {
          ret = help->label;
          break;
        }
      }
      return ret;
    }

  /* look up functor/arity */
  ProgramCounter lookup(Atom *name, int arity, ProgramCounter elseLabel)
    {
      ProgramCounter ret = elseLabel;

      for (HTEntry *help = this; help != NULL; help = help->next) {
        if ( (help->functor.fname == name) &&
             (help->functor.arity == arity) ) {
          ret = help->label;
          break;
        }
      }
      return ret;
    }

  /* look a number */
  ProgramCounter lookup(TaggedRef term /* actually a number*/ , ProgramCounter elseLabel)
    {
      ProgramCounter ret = elseLabel;

      for (HTEntry *help = this; help != NULL; help = help->next) {
        if (numberEq(help->number,term)) {
          ret = help->label;
          break;
        }
      }
      return ret;
    }



};

inline int nextPowerOf2(int n)
{
  for(int i=2;; i *=2) {
    if (i>=n) return i;
  }
}


typedef HTEntry** EntryTable;

class IHashTable {
 public:
  int size;      // is always a power of 2
  int hashMask;  // always size-1

  EntryTable atomTable;
  EntryTable functorTable;
  EntryTable numberTable;

  ProgramCounter elseLabel;
  ProgramCounter listLabel;
  ProgramCounter varLabel;

  IHashTable(int sz, ProgramCounter elseLbl) {
    size = nextPowerOf2(sz);
    hashMask = size-1;
    atomTable = functorTable = numberTable = NULL;
    elseLabel = elseLbl;
    listLabel = elseLabel;
    varLabel  = elseLabel;
  };

  void add(TaggedRef number, ProgramCounter label);
  void add(Atom *constant, ProgramCounter label);
  void add(Atom *functor, int arity, ProgramCounter label);
  void addVar(/* no arg means list structure*/ ProgramCounter label) {
    varLabel = label;
  }
  void addList(ProgramCounter label) {
    listLabel = label;
  }

  int hash(int n) { return (n & hashMask); }  // return a value n with 0 <= n < size
  ProgramCounter getElse() { return elseLabel; }

};

#endif
