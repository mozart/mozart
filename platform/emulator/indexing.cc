/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

*/


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "indexing.hh"
#endif

#include "tagged.hh"
#include "term.hh"

#include "constter.hh"
#include "board.hh"

#include "suspensi.hh"
#include "variable.hh"

#include "bignum.hh"

// #include "codearea.hh"
#include "indexing.hh"
#include "genvar.hh"


EntryTable newEntryTable(int sz)
{
  EntryTable help = new HTEntry*[sz];
  for (int i = 0; i <sz; i++)
    help[i] = (HTEntry*)NULL;

  return help;
}


void IHashTable::add(Literal *constant, ProgramCounter label) 
{
  int hsh = constant->hash() % size;
  
  if (literalTable == NULL)
    literalTable = newEntryTable(size); 
  
  /* we do not check, whether it is already in there */
  literalTable[hsh] = new HTEntry(constant,label,literalTable[hsh]);
}

void IHashTable::add(Literal *name, int arity, ProgramCounter label) 
{
  int hsh = name->hash() % size;
  
  if (functorTable == NULL)
    functorTable = newEntryTable(size); 
  
  /* we do not check, whether it is already in there */
  functorTable[hsh] = new HTEntry(name, arity, label, functorTable[hsh]);
}


void IHashTable::add(TaggedRef number, ProgramCounter label)
{
  int hsh;

  switch (tagTypeOf(number)) {

  case OZFLOAT:
    hsh = tagged2Float(number)->hash() % size;
    break;

  case BIGINT:
    hsh = tagged2BigInt(number)->hash() % size;
    break;

  case SMALLINT:
    hsh = smallIntHash(number) % size;
    break;
    
  default:
    error("Assertion failed in IHashTable::add: argument not a number");
    return;
  }

  if (numberTable == NULL)
    numberTable = newEntryTable(size); 
  
  /* we do not check, whether it is already in there */
  numberTable[hsh] = new HTEntry(number, label, numberTable[hsh]);
}


// - 'table' holds the code to branch to when indexing
// - 'elseLabel' is the PC of the ELSE-branch, ie. in case there is no
//   clause to switch to
// How it works:
// If none of the numbers is member of the domain, no guard can ever be
// entailed therefore goto to the else-branch. Otherwise goto varLabel, which
// wait for determination of the variable. Usually if unconstrained variables
// get bound to each other, det-nodes are not reentered, but since
// unifying two fd variables may result in a singleton (ie. determined term),
// det-nodes are reentered and we achieve completeness.

ProgramCounter IHashTable::index(GenCVariable *cvar, ProgramCounter elseLbl)
{
  if (cvar->getType()==AVAR) return varLabel;

  // if there are no integer guards goto else-branch
  if (numberTable) {
    HTEntry** aux_table = numberTable;
    int tsize = size;

    // if there is at least one integer member of the domain then goto varLabel
    for (int i = 0; i < tsize; i++) {
      HTEntry* aux_entry = aux_table[i];
      while (aux_entry) {
	if (cvar->valid(aux_entry->getNumber()))
	    return varLabel;
	aux_entry = aux_entry->getNext();
      }
    }
  }
  
  return elseLbl;
}

