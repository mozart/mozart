/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Contributors:
 *    Michael Mehl <mehl@ps.uni-sb.de>
 *    Ralf Scheidhauer <scheidhr@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Michael Mehl, 1999
 *    Ralf Scheidhauer, 1999
 *    Christian Schulte, 2000
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

IHashTable * IHashTable::allocate(int n, int el) {
  int sz = nextPowerOf2(n+max(1,n>>1)); 
  Assert(sz > n);
  IHashTable * ht = (IHashTable *)
    malloc(sizeof(IHashTable) + (sz-1) * sizeof(IHashTableEntry));
  ht->elseLbl = el;
  ht->listLbl = el;
  ht->hashMsk = sz-1;
  for (int i = sz; i--; ) {
    ht->entries[i].val = makeTaggedNULL();
    ht->entries[i].lbl = el;
  }
  return ht;
}

IHashTable * IHashTable::clone(void) {
  size_t sz = sizeof(IHashTable) + (getSize()-1) * sizeof(IHashTableEntry);
  IHashTable * t = (IHashTable *) malloc(sz);
  t->elseLbl = elseLbl;
  t->listLbl = listLbl;
  t->hashMsk = hashMsk;
  for (int i = t->getSize(); i--; ) {
    t->entries[i].val = makeTaggedNULL();
    t->entries[i].lbl = elseLbl;
  }
  return t;
}

int IHashTable::getEntries(void) {
  int e = 0;
  for (int i = getSize(); i--; )
    if (entries[i].val) e++;
  return e;
}

void IHashTable::addRecord(TaggedRef l, SRecordArity a, int lbl) {
  int i = tagged2Literal(l)->hash();
  while (OK) {
    i &= hashMsk;
    if (!entries[i].val) {
      entries[i].val = l;
      entries[i].sra = a;
      entries[i].lbl = lbl;
      break;
    }
    i++;
  }
}
  
void IHashTable::addScalar(TaggedRef t, int lbl) {
  int i;
  if (oz_isSmallInt(t)) {
    i = smallIntHash(t);
  } else if (oz_isLiteral(t)) {
    i = tagged2Literal(t)->hash();
  } else if (oz_isFloat(t)) {
    i = tagged2Float(t)->hash();
  } else {
    Assert(oz_isBigInt(t));
    i = tagged2BigInt(t)->hash();
  }
  while (OK) {
    i &= hashMsk;
    if (!entries[i].val) {
      entries[i].val = t;
      entries[i].sra = mkTupleWidth(0);
      entries[i].lbl = lbl;
      break;
    }
    i++;
  }
}    


Bool IHashTable::disentailed(OzVariable *var) {
  switch (var->getType()) {
  case OZ_VAR_FD: 
  case OZ_VAR_BOOL: 
    {
      for (int i = getSize(); i--; )
	if (entries[i].val && oz_isSmallInt(entries[i].val) &&
	    oz_var_valid(var,entries[i].val))
	  return NO;
      break;
    }
  case OZ_VAR_OF:
    {
      OzOFVariable * ofsvar = (OzOFVariable*) var;
      if (!ofsvar->disentailed(tagged2Literal(AtomCons),2))
	return NO;
      for (int i = getSize(); i--; )
	if (entries[i].val && oz_isLiteral(entries[i].val)) {
	  Literal * l      = tagged2Literal(entries[i].val);
	  SRecordArity sra = entries[i].sra;
	  if (sraIsTuple(sra)) {
	    if (!ofsvar->disentailed(l,getTupleWidth(sra)))
	      return NO;
	  } else {
	    if (!ofsvar->disentailed(l,getRecordArity(sra)))
	      return NO;
	  }
	}
      break;
    }

  case OZ_VAR_EXT:
    // hack: an arbitrary number is check for validity
    return !oz_var_valid(var,makeTaggedSmallInt(4711));
  default:    
    return NO;
  }
  return OK;
}

