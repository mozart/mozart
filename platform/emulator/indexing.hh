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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
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
  ProgramCounter *getLabelRef()  {return &label;}
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

  ProgramCounter *add(TaggedRef number, ProgramCounter label);
  ProgramCounter *add(Literal *constant, ProgramCounter label);
  ProgramCounter *add(Literal *functor, SRecordArity arity,
		      ProgramCounter label);
  void addVar(ProgramCounter label)  { varLabel  = label; }
  void addList(ProgramCounter label) { listLabel = label; }

  int hash(int n) { return (n & hashMask); }  // return a value n with 0 <= n < size
  ProgramCounter getElse() { return elseLabel; }

  Bool disentailed(GenCVariable *var, TaggedRef *varPtr);
};

ProgramCounter switchOnTermOutline(TaggedRef term, TaggedRef *termPtr,
				   IHashTable *table, TaggedRef *&sP);

#endif

