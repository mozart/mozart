/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#ifndef __INDEXINGH
#define __INDEXINGH

#ifdef INTERFACE
#pragma interface
#endif

#include "value.hh"

class HTEntry {
 protected:
  int label;
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

  HTEntry(Literal *name, int lbl) : label(lbl), next(0) { u.literal = name; };

  HTEntry(TaggedRef num, int lbl) : label(lbl), next(0) 
  {
    u.number = num;
    oz_staticProtect(&u.number);
  };

  HTEntry(Literal *name, SRecordArity arity, int lbl) : label(lbl), next(0) {
    u.functor.fname = name;
    u.functor.arity = arity;
  };

  void setNext(HTEntry *nxt) { next = nxt; }
  HTEntry* getNext(void) {return next;}

  int getLabel()              { return label; }
  int *getLabelRef()          { return &label; }
  TaggedRef getNumber()       { return u.number; }
  Literal *getLiteral()       { return u.literal; }
  void setLiteral(Literal *l) { u.literal = l; }
  Literal *getFunctor(SRecordArity &a)  
  {
    a = u.functor.arity;
    return u.functor.fname;
  }

  void setFunctor(Literal *l, SRecordArity a) {u.functor.fname=l; u.functor.arity=a; }


  int lookupLiteral(Literal * l, int elseLabel) {
    for (HTEntry * help = this; help != NULL; help = help->next)
      if (help->u.literal == l)
	return help->label;
    return elseLabel;
  }

  int lookupSRecord(Literal * l, SRecordArity sra, int elseLabel) {
    for (HTEntry *help = this; help != NULL; help = help->next)
      if ((help->u.functor.fname == l) &&
	  sameSRecordArity(help->u.functor.arity,sra))
	return help->label;
    return elseLabel;
  }

  int lookupSmallInt(TaggedRef term, int elseLabel) {
    Assert(oz_isSmallInt(term));
    for (HTEntry * help = this; help != NULL; help = help->next)
      if (oz_eq(help->u.number,term)) {
	Assert(oz_isSmallInt(help->u.number));
	return help->label;
      }
    return elseLabel;
  }

  int lookupBigInt(BigInt * i, int elseLabel) {
    for (HTEntry * help = this; help != NULL; help = help->next)
      if (oz_isConst(help->u.number) &&
	  (tagged2BigInt(help->u.number)->equal(i))) {
	Assert(oz_isBigInt(help->u.number));
	return help->label;
      }
    return elseLabel;
  }

  int lookupFloat(double f, int elseLabel) {
    for (HTEntry * help = this; help != NULL; help = help->next)
      if (oz_isFloat(help->u.number) && 
	  (tagged2Float(help->u.number)->getValue() == f))
	return help->label;
    return elseLabel;
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
  
  int elseLabel;
  int listLabel;

  IHashTable(int sz, int elseLbl) {
    numentries = 0;
    size = nextPowerOf2(sz);
    hashMask = size-1;
    literalTable = functorTable = numberTable = NULL;
    elseLabel = elseLbl;
    listLabel = elseLbl;
  };

  int *addToTable(EntryTable &table, HTEntry *entry, int pos);
  int *add(TaggedRef number, int label);
  int *add(Literal *constant, int label);
  int *add(Literal *functor, SRecordArity arity,
		      int label);
  void addList(int label) { listLabel = label; }

  int hash(int n) { return (n & hashMask); }  // return a value n with 0 <= n < size
  int getElse() { 
    return elseLabel; 
  }

  int lookupLTuple(void) {
    /* If there is no alternative list, listLabel equals elseLabel */
    return listLabel;
  }
  
  int lookupLiteral(TaggedRef term) {
    if (literalTable) {
      Literal * l = tagged2Literal(term);
      return literalTable[hash(l->hash())]->lookupLiteral(l,elseLabel);
    } else {
      return elseLabel;
    }
  }
  
  int lookupSRecord(TaggedRef term) {
    if (functorTable) {
      SRecord * r = tagged2SRecord(term);
      Literal * l = r->getLabelLiteral();
      Assert(l!=NULL);
      return functorTable[hash(l->hash())]->
	lookupSRecord(l,r->getSRecordArity(),elseLabel);
    } else {
      return elseLabel;
    }
  }
  
  int lookupSmallInt(TaggedRef term) {
    if (numberTable) {
      return numberTable[hash(smallIntHash(term))]->lookupSmallInt(term,elseLabel);
    } else {
      return elseLabel;
    }
  }
  
  int lookupConst(TaggedRef term) {
    if (numberTable && tagged2Const(term)->getType() == Co_BigInt) {
      BigInt * b = tagged2BigInt(term);
      return numberTable[hash(b->hash())]->lookupBigInt(b,elseLabel);
    } else {
      return elseLabel;
    }
  }
  
  int lookupFloat(TaggedRef term) {
    if (numberTable) {
      Float * f = tagged2Float(term);
      return numberTable[hash(f->hash())]->lookupFloat(f->getValue(),elseLabel);
    } else {
      return elseLabel;
    }
  }

  Bool disentailed(OzVariable * var);
  
};


#endif
