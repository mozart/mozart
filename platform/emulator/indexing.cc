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

#include "am.hh"
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
  numentries++;
  unsigned int hsh = constant->hash() % size;

  if (literalTable == NULL)
    literalTable = newEntryTable(size);

  /* we do not check, whether it is already in there */
  literalTable[hsh] = new HTEntry(constant,label,literalTable[hsh]);
}

void IHashTable::add(Literal *name, SRecordArity arity, ProgramCounter label)
{
  numentries++;
  unsigned int hsh = name->hash() % size;

  if (functorTable == NULL)
    functorTable = newEntryTable(size);

  /* we do not check, whether it is already in there */
  functorTable[hsh] = new HTEntry(name, arity, label, functorTable[hsh]);
}


void IHashTable::add(TaggedRef number, ProgramCounter label)
{
  numentries++;

  unsigned int hsh;
  switch (tagTypeOf(number)) {

  case OZFLOAT:  hsh = tagged2Float(number)->hash() % size;  break;
  case BIGINT:   hsh = tagged2BigInt(number)->hash() % size; break;
  case SMALLINT: hsh = smallIntHash(number) % size;          break;
  default:       Assert(0);                                  return;
  }

  if (numberTable == NULL)
    numberTable = newEntryTable(size);

  /* we do not check, whether it is already in there */
  numberTable[hsh] = new HTEntry(number, label, numberTable[hsh]);
}


// - 'table' holds the code to branch to when indexing
// How it works:
// If none of the numbers is member of the domain, no guard can ever be
// entailed therefore goto to the else-branch. Otherwise goto varLabel, which
// wait for determination of the variable. Usually if unconstrained variables
// get bound to each other, det-nodes are not reentered, but since
// unifying two fd variables may result in a singleton (ie. determined term),
// det-nodes are reentered and we achieve completeness.

Bool IHashTable::disentailed(GenCVariable *cvar, TaggedRef *ptr)
{
  switch (cvar->getType()) {
  case FDVariable:
  case BoolVariable:
    {
      /* if there are no integer guards goto else-branch */
      if (!numberTable) {
        return OK;
      }

      // if there is at least one integer member of the domain then goto varLabel
      for (int i = 0; i < size; i++) {
        for (HTEntry* aux = numberTable[i]; aux!=NULL; aux=aux->getNext()) {
          if (cvar->valid(ptr,aux->getNumber()))
            return NO;
        }
      }

      return OK;
    }
  case OFSVariable:
    {
      GenOFSVariable *ofsvar = (GenOFSVariable*) cvar;
      if (literalTable) {
        for (int i = 0; i < size; i++) {
          for (HTEntry* aux = literalTable[i]; aux!=NULL; aux=aux->getNext()) {
            if (!ofsvar->disentailed(aux->getLiteral(),(int)0))
              return NO;
          }
        }
      }

      if (functorTable) {
        for (int i = 0; i < size; i++) {
          for (HTEntry* aux = functorTable[i]; aux!=NULL; aux=aux->getNext()) {
            SRecordArity arity;
            Literal *label = aux->getFunctor(arity);
            if (sraIsTuple(arity)) {
              if (!ofsvar->disentailed(label,getTupleWidth(arity)))
                return NO;
            } else {
              if (!ofsvar->disentailed(label,getRecordArity(arity)))
                return NO;
            }
          }
        }
      }
      return OK;
    }

  case AVAR:
    return ((GenCVariable *)this)->valid(ptr,OZ_int(4711));

  case PerdioVariable:
    return ((GenCVariable *)this)->valid(ptr,OZ_int(4711));

  default:
    return NO;
  }
}

ProgramCounter switchOnTermOutline(TaggedRef term, TaggedRef *termPtr,
                                   IHashTable *table, TaggedRef *&sP)
{
  ProgramCounter offset = table->getElse();
  if (isSRecord(term)) {
    if (table->functorTable) {
      SRecord *rec = tagged2SRecord(term);
      Literal *lname = rec->getLabelLiteral();
      Assert(lname!=NULL);
      unsigned int hsh = table->hash(lname->hash());
      offset = table->functorTable[hsh]->lookup(lname,rec->getSRecordArity(),offset);
      sP = rec->getRef();
    }
    return offset;
  }

  if (isLiteral(term)) {
    if (table->literalTable) {
      unsigned int hsh = table->hash(tagged2Literal(term)->hash());
      offset = table->literalTable[hsh]->lookup(tagged2Literal(term),offset);
    }
    return offset;
  }

  if (isNotCVar(term)) {
    return table->varLabel;
  }

  if (isSmallInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(smallIntHash(term));
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isFloat(term)) {
    if (table->numberTable) {
      unsigned int hsh = table->hash(tagged2Float(term)->hash());
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isBigInt(term)) {
    if (table->numberTable) {
      unsigned int hsh = table->hash(tagged2BigInt(term)->hash());
      offset =table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (isCVar(term) && !table->disentailed(tagged2CVar(term),termPtr)) {
    return table->varLabel;
  }

  return offset;
}
