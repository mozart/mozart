/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

*/


#ifdef __GNUC__
#pragma implementation "indexing.hh"
#endif


#include "indexing.hh"


EntryTable newEntryTable(int size)
{
  EntryTable help = new HTEntry*[size];
  for (int i = 0; i <size; i++)
    help[i] = (HTEntry*)NULL;

  return help;
}


void IHashTable::add(Atom *constant, ProgramCounter label) 
{
  int hsh = constant->hash() % size;
  
  if (atomTable == NULL)
    atomTable = newEntryTable(size); 
  
  /* we do not check, whether it is already in there */
  atomTable[hsh] = new HTEntry(constant,label,atomTable[hsh]);
}

void IHashTable::add(Atom *name, int arity, ProgramCounter label) 
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

  case FLOAT:
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


