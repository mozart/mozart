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


#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "indexing.hh"
#endif

#include "am.hh"
#include "indexing.hh"
#include "var_base.hh"
#include "var_of.hh"

EntryTable newEntryTable(int sz)
{
  EntryTable help = new HTEntry*[sz];
  for (int i = 0; i <sz; i++)
    help[i] = (HTEntry*)NULL;

  return help;
}


int *IHashTable::addToTable(EntryTable &table, HTEntry *entry, int pos)
{
  numentries++;

  if (table == NULL)
    table = newEntryTable(size);

  /* append new entries to the end, so (un)marshalling will preserve order */
  HTEntry *aux = table[pos];
  if (aux==NULL) {
    table[pos] = entry;
  } else {
    while(aux->getNext()) {
      aux = aux->getNext();
    }
    aux->setNext(entry);
  }

  return entry->getLabelRef();
}


int *IHashTable::add(Literal *constant, int label)
{
  unsigned int hsh = hash(constant->hash());
  return addToTable(literalTable,new HTEntry(constant,label),hsh);
}


int *IHashTable::add(Literal *name, SRecordArity arity,
                                int label)
{
  unsigned int hsh = hash(name->hash());
  return addToTable(functorTable,new HTEntry(name, arity, label),hsh);
}


int *IHashTable::add(TaggedRef number, int label)
{
  unsigned int hsh;
  switch (tagTypeOf(number)) {

  case TAG_FLOAT:    hsh = tagged2Float(number)->hash();  break;
  case TAG_CONST:    hsh = tagged2BigInt(number)->hash(); break;
  case TAG_SMALLINT: hsh = smallIntHash(number);          break;
  default:       Assert(0); return 0;
  }

  hsh = hash(hsh);

  return addToTable(numberTable,new HTEntry(number,label),hsh);
}


// - 'table' holds the code to branch to when indexing
// How it works:
// If none of the numbers is member of the domain, no guard can ever be
// entailed therefore goto to the else-branch. Otherwise goto varLabel, which
// wait for determination of the variable. Usually if unconstrained variables
// get bound to each other, det-nodes are not reentered, but since
// unifying two fd variables may result in a singleton (ie. determined term),
// det-nodes are reentered and we achieve completeness.

Bool IHashTable::disentailed(OzVariable *cvar, TaggedRef *ptr)
{
  switch (cvar->getType()) {
  case OZ_VAR_FD:
  case OZ_VAR_BOOL:
    {
      /* if there are no integer guards goto else-branch */
      if (!numberTable) {
        return OK;
      }

      // if there is at least one integer member of the domain then goto varLabel
      for (int i = 0; i < size; i++) {
        for (HTEntry* aux = numberTable[i]; aux!=NULL; aux=aux->getNext()) {
          if (oz_var_valid(cvar,ptr,aux->getNumber()))
            return NO;
        }
      }

      return OK;
    }
  case OZ_VAR_OF:
    {
      OzOFVariable *ofsvar = (OzOFVariable*) cvar;
      if (listLabel && !ofsvar->disentailed(tagged2Literal(AtomCons),2))
        return NO;

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

  // mm2: hack: an arbitrary number is check for validity
  case OZ_VAR_EXT:
    return !oz_var_valid(cvar,ptr,oz_int(4711));

  default:
    return NO;
  }
}

int switchOnTermOutline(TaggedRef term, TaggedRef *termPtr,
                        IHashTable *table, TaggedRef *&sP)
{
  int offset = table->getElse();
  if (oz_isSRecord(term)) {
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

  if (oz_isLiteral(term)) {
    if (table->literalTable) {
      unsigned int hsh = table->hash(tagged2Literal(term)->hash());
      offset = table->literalTable[hsh]->lookup(tagged2Literal(term),offset);
    }
    return offset;
  }

  if (oz_isUVar(term)) {
    return 0;
  }

  if (oz_isSmallInt(term)) {
    if (table->numberTable) {
      int hsh = table->hash(smallIntHash(term));
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (oz_isFloat(term)) {
    if (table->numberTable) {
      unsigned int hsh = table->hash(tagged2Float(term)->hash());
      offset = table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (oz_isBigInt(term)) {
    if (table->numberTable) {
      unsigned int hsh = table->hash(tagged2BigInt(term)->hash());
      offset =table->numberTable[hsh]->lookup(term,offset);
    }
    return offset;
  }

  if (oz_isVariable(term)) {
    if (!oz_isKinded(term)) {
      return 0;
    }

    if (oz_isCVar(term) && !table->disentailed(tagged2CVar(term),termPtr)) {
      return 0;
    }
  }

  // fail
  return offset;
}
