/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Konstantin Popov <kost@sics.se>
 * 
 *  Contributors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Konstantin Popov, 2002
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

#if defined(INTERFACE)
#pragma implementation "dictionary.hh"
#endif

#include "dictionary.hh"
#include "hashtbl.hh"
#include "am.hh"
#include "ozostream.hh"
#include "builtins.hh"
#include "sort.hh"
#include "board.hh"

//
static const double GDT_MAXENTRIES	= 0.9;
// static const double GDT_MAXENTRIES	= 1.0;
// static const double GDT_MAXSLOTS	= 1.0;
//
static const double GDT_IDEALENTRIES	= 0.7;
static const int GDT_MINFULL		= 4;   // 

//
static unsigned int minTabSize = HT_PRIME0;
static unsigned int maxTabSize = HT_PRIME45;
// primes stepping ca. 3/2 from each other;
unsigned int dictHTSizes[] = {
  HT_PRIME0,
  HT_PRIME1,
  HT_PRIME2,
  HT_PRIME3,
  HT_PRIME4,
  HT_PRIME5,
  HT_PRIME6,
  HT_PRIME7,
  HT_PRIME8,
  HT_PRIME9,
  HT_PRIME10,
  HT_PRIME11,
  HT_PRIME12,
  HT_PRIME13,
  HT_PRIME14,
  HT_PRIME15,
  HT_PRIME16,
  HT_PRIME17,
  HT_PRIME18,
  HT_PRIME19,
  HT_PRIME20,
  HT_PRIME21,
  HT_PRIME22,
  HT_PRIME23,
  HT_PRIME24,
  HT_PRIME25,
  HT_PRIME26,
  HT_PRIME27,
  HT_PRIME28,
  HT_PRIME29,
  HT_PRIME30,
  HT_PRIME31,
  HT_PRIME32,
  HT_PRIME33,
  HT_PRIME34,
  HT_PRIME35,
  HT_PRIME36,
  HT_PRIME37,
  HT_PRIME38,
  HT_PRIME39,
  HT_PRIME40,
  HT_PRIME41,
  HT_PRIME42,
  HT_PRIME43,
  HT_PRIME44,
  HT_PRIME45,
  0
};

/*
static unsigned int minTabSize = 1;
static unsigned int maxTabSize = 1073741824;
unsigned int dictHTSizes[] = {
  1,
  2,
  4,
  8,
  16,
  32,
  64,
  128,
  256,
  512,
  1024,
  2048,
  4096,
  8129,
  16384,
  32768,
  65536,
  131072,
  262144,
  524288,
  1048576,
  2097152,
  4194304,
  8388608,
  16777216,
  33554432,
  67108864,
  134217728,
  268435456,
  536870912,
  1073741824,
  0
};
*/

//
//
void DictHashTable::mkEmpty()
{
  int tableSize = dictHTSizes[sizeIndex];
  entries = /* slots = */ 0;
  maxEntries = (int) (GDT_MAXENTRIES * tableSize);
  // maxSlots = (int) (GDT_MAXSLOTS * tableSize);
  table = (DictNode *) oz_heapMalloc(tableSize * sizeof(DictNode));
  for (int i = tableSize; i--; )
    (void) new (&table[i]) DictNode;
}

//
void DictHashTable::init(int size)
{
  Assert(size <= maxTabSize);
  sizeIndex = 0;
  while (dictHTSizes[sizeIndex] < size)
    sizeIndex++;
  Assert(size <= dictHTSizes[sizeIndex]);
  mkEmpty();
}

//
void DictHashTable::resize()
{
  DictNode* old = table;
  DictNode* eold = table + dictHTSizes[sizeIndex];
  Assert(old != eold);
  int oldEntries = entries;

  //
  Assert(entries >= 0);
  sizeIndex++;
  mkEmpty();
  entries = oldEntries;

  //
  do {
    if (!old->isEmpty()) {
      if (!old->isPointer()) {
	htReAdd(old->getKey(), old->getValue());
      } else {
	DictNode *sptr = old->getDictNodeSPtr();
	DictNode *eptr = old->getDictNodeEPtr();
	do {
	  htReAdd(sptr->getKey(), sptr->getValue());
	  sptr++;
	} while (sptr < eptr);
      }
    }
    old++;
  } while (old < eold);
  Assert(entries == oldEntries);
}

//
void DictHashTable::compactify()
{
  int oldSize;
  DictNode* old;
  int oldEntries;

  //
  Assert(entries >= 0);
  oldSize = dictHTSizes[sizeIndex];
  if (entries >= (oldSize / GDT_MINFULL))
    return;

  //
  old = table;
  oldEntries = entries;

  // can be zero too:
  int tableSize = (int) ((double) entries * GDT_IDEALENTRIES);
  Assert(tableSize < oldSize);
  sizeIndex--;
  while (sizeIndex >= 0 && dictHTSizes[sizeIndex] >= tableSize)
    sizeIndex--;
  Assert(sizeIndex < 0 || dictHTSizes[sizeIndex] < tableSize);
  sizeIndex++;
  Assert(sizeIndex >= 0 && dictHTSizes[sizeIndex] >= tableSize);
  Assert(dictHTSizes[sizeIndex] < oldSize);	// must not oscillate;
  // construct the table anew;
  mkEmpty();
  entries = oldEntries;

  //
  for (int i = oldSize; i--; old++) {
    if (!old->isEmpty()) {
      if (!old->isPointer()) {
	htReAdd(old->getKey(), old->getValue());
      } else {
	DictNode *sptr = old->getDictNodeSPtr();
	DictNode *eptr = old->getDictNodeEPtr();
	do {
	  htReAdd(sptr->getKey(), sptr->getValue());
	  sptr++;
	} while (sptr < eptr);
      }
    }
  }
  Assert(entries == oldEntries);
}

//
DictHashTable* DictHashTable::copy()
{
  int tableSize = dictHTSizes[sizeIndex];
  DictNode *an = (DictNode *) oz_heapMalloc(tableSize * sizeof(DictNode));

  //
  for (int i = tableSize; i--; ) {
    DictNode *n = &table[i];
    if (n->isEmpty()) {
      (void) new (&an[i]) DictNode();
    } else {
      if (!n->isPointer()) {
	(void) new (&an[i]) DictNode(*n); // maybe empty;
      } else {
	DictNode *sptr = n->getDictNodeSPtr();
	DictNode *eptr = n->getDictNodeEPtr();
	DictNode *newA = (DictNode *)
	  oz_heapMalloc(((char *) eptr) - ((char *) sptr));
	DictNode *nn = &an[i];
	//
	nn->setSPtr(newA);
	do {
	  (void) new (newA++) DictNode(*sptr++);
	} while (sptr < eptr);
	nn->setEPtr(newA);
      }
    }
  }

  //      
  DictHashTable *dht = new DictHashTable(*this);
  dht->table = an;
  return (dht);
}

// Return _unsorted_ list containing all features:
OZ_Term DictHashTable::getKeys() 
{
  OZ_Term arity = AtomNil;

  //
  for (int i = dictHTSizes[sizeIndex]; i--; ) {
    DictNode* n = &table[i];
    if (!n->isEmpty()) {
      if (!n->isPointer()) {
	Assert(oz_isFeature(n->getKey()));
	arity = oz_cons(n->getKey(), arity);
      } else {
	DictNode *sptr = n->getDictNodeSPtr();
	DictNode *eptr = n->getDictNodeEPtr();
	do {
	  Assert(oz_isFeature(sptr->getKey()));
	  arity = oz_cons(sptr->getKey(), arity);
	  sptr++;
	} while (sptr < eptr);
      }
    }
  }
  return (arity);
}

OZ_Term DictHashTable::getPairs()
{
  OZ_Term arity = AtomNil;

  //
  for (int i = dictHTSizes[sizeIndex]; i--; ) {
    DictNode* n = &table[i];
    if (!n->isEmpty()) {
      if (!n->isPointer()) {
	Assert(oz_isFeature(n->getKey()));
	SRecord *sr = SRecord::newSRecord(AtomPair, 2);
	sr->setArg(0, n->getKey());
	sr->setArg(1, n->getValue());
	arity = oz_cons(makeTaggedSRecord(sr), arity);
      } else {
	DictNode *sptr = n->getDictNodeSPtr();
	DictNode *eptr = n->getDictNodeEPtr();
	do {
	  Assert(oz_isFeature(sptr->getKey()));
	  SRecord *sr = SRecord::newSRecord(AtomPair, 2);
	  sr->setArg(0, sptr->getKey());
	  sr->setArg(1, sptr->getValue());
	  arity = oz_cons(makeTaggedSRecord(sr), arity);
	  sptr++;
	} while (sptr < eptr);
      }
    }
  }
  return (arity);
}

DictNode* DictHashTable::getPairsInArray()
{
  Assert(entries > 0);
  DictNode* out = new DictNode[entries];
  DictNode* acc = out;

  //
  for (int i = dictHTSizes[sizeIndex], ai = 0; i--; ) {
    DictNode* n = &table[i];
    if (!n->isEmpty()) {
      if (!n->isPointer()) {
	Assert(oz_isFeature(n->getKey()));
	new (acc++) DictNode(*n);
      } else {
	DictNode *sptr = n->getDictNodeSPtr();
	DictNode *eptr = n->getDictNodeEPtr();
	do {
	  Assert(oz_isFeature(sptr->getKey()));
	  new (acc++) DictNode(*sptr);
	  sptr++;
	} while (sptr < eptr);
      }
    }
  }

  //
  return (out);
}

OZ_Term DictHashTable::getItems() 
{
  OZ_Term arity = AtomNil;

  //
  for (int i = dictHTSizes[sizeIndex]; i--; ) {
    DictNode* n = &table[i];
    if (!n->isEmpty()) {
      if (!n->isPointer()) {
	Assert(oz_isFeature(n->getKey()));
	arity = oz_cons(n->getValue(), arity);
      } else {
	DictNode *sptr = n->getDictNodeSPtr();
	DictNode *eptr = n->getDictNodeEPtr();
	do {
	  Assert(oz_isFeature(sptr->getKey()));
	  arity = oz_cons(sptr->getValue(), arity);
	  sptr++;
	} while (sptr < eptr);
      }
    }
  }
  return (arity);
}

class Order_TaggedRef_By_Feat {
public:
  Bool operator()(const TaggedRef& a, const TaggedRef& b) {
    return featureCmp(a, b) < 0;
  }
};

//
// Allocate & return sorted list containing all features:
// Takes optional tail as input argument.
OZ_Term DictHashTable::getArityList(OZ_Term tail)
{
  Assert(entries >= 0);
  if (entries > 0) {
    int i, ai;

    //
    // put elements into a temporary array;
    NEW_TEMP_ARRAY(OZ_Term, arr, entries);
    //
    for (i = dictHTSizes[sizeIndex], ai = 0; i--; ) {
      DictNode* n = &table[i];
      if (!n->isEmpty()) {
	if (!n->isPointer()) {
	  Assert(oz_isFeature(n->getKey()));
	  arr[ai++] = n->getKey();
	} else {
	  DictNode *sptr = n->getDictNodeSPtr();
	  DictNode *eptr = n->getDictNodeEPtr();
	  do {
	    Assert(oz_isFeature(sptr->getKey()));
	    arr[ai++] = sptr->getKey();
	    sptr++;
	  } while (sptr < eptr);
	}
      }
    }

    //
    Order_TaggedRef_By_Feat lt;
    fastsort(arr, entries, lt);
    //
    OZ_Term arity = tail;
    for (i = entries; i--; )
      arity = oz_cons(arr[i], arity);
    //
    return (arity);
  } else {
    return (tail);
  }
}

// Convert dynamic table to Literal, SRecord, or LTuple:
OZ_Term DictHashTable::toRecord(OZ_Term lbl)
{
  Assert(entries >= 0);
  if (entries == 0) {
    return (lbl);
  } else {
    OZ_Term alist = getArityList(oz_nil());
    Arity *arity = aritytable.find(alist);
    SRecord *newrec = SRecord::newSRecord(lbl, arity);
    int i;

    //
    for (i = dictHTSizes[sizeIndex]; i--; ) {
      DictNode* n = &table[i];
      if (!n->isEmpty()) {
	if (!n->isPointer()) {
	  Bool ok = newrec->setFeature(n->getKey(), n->getValue());
	  Assert(ok);
	} else {
	  DictNode *sptr = n->getDictNodeSPtr();
	  DictNode *eptr = n->getDictNodeEPtr();
	  do {
	    Bool ok = newrec->setFeature(sptr->getKey(), sptr->getValue());
	    Assert(ok);
	    sptr++;
	  } while (sptr < eptr);
	}
      }
    }

    //
    return (newrec->normalize());
  }
}

//  OZ_Term DictHashTable::htFindOutline(OZ_Term key)
//  {
//    return (htFind(key));
//  }

/*
 * Builtins
 */

OZ_BI_define(BIdictionaryNew,0,1)
{
  OZ_RETURN(makeTaggedConst(new OzDictionary(oz_currentBoard())));
} OZ_BI_end

OZ_BI_define(BIdictionaryKeys,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->keys());
} OZ_BI_end


OZ_BI_define(BIdictionaryMarkSafe,1,0)
{
  oz_declareDictionaryIN(0,dict);
  dict->markSafe();
  return PROCEED;
} OZ_BI_end


OZ_BI_define(BIdictionaryEntries,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->pairs());
} OZ_BI_end


OZ_BI_define(BIdictionaryItems,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->items());
} OZ_BI_end


OZ_BI_define(BIdictionaryClone,1,1)
{
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->clone(oz_currentBoard()));
} OZ_BI_end


OZ_BI_define(BIdictionaryToRecord,2,1) {
  oz_declareNonvarIN(0, label);

  if (!oz_isLiteral(label)) 
    oz_typeError(0, "Literal");

  oz_declareDictionaryIN(1,dict);

  OZ_RETURN(dict->toRecord(label));
} OZ_BI_end


OZ_BI_define(BIdictionaryIsEmpty,1,1) {
  oz_declareDictionaryIN(0,dict);

  OZ_RETURN(dict->isEmpty() ? oz_true() : oz_false());
} OZ_BI_end


OZ_Return isDictionaryInline(OZ_Term t, OZ_Term &out)
{
  NONVAR( t, term);
  out = oz_bool(oz_isDictionary(term));
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN1(BIisDictionary,isDictionaryInline)


#define GetDictAndKey(d,k,dict,key,checkboard)				\
  NONVAR(k,key);							\
  if (!oz_isFeature(key))        { oz_typeError(1,"feature"); }		\
  NONVAR(d,dictaux);							\
  if (!oz_isDictionary(dictaux)) { oz_typeError(0,"Dictionary"); }	\
  OzDictionary *dict = tagged2Dictionary(dictaux);			\
  if (checkboard)		 { CheckLocalBoard(dict,"dict"); }

OZ_Return dictionaryMemberInline(OZ_Term d, OZ_Term k, OZ_Term &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  out = dict->member(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryMember,dictionaryMemberInline)

OZ_Return dictionaryGetInline(OZ_Term d, OZ_Term k, OZ_Term &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  out = dict->getArg(key);
  if (out) {
    return (PROCEED);
  } else {
    return oz_raise(E_SYSTEM,E_KERNEL,"dict",2,d,k);
  }
  Assert(0);
}

OZ_DECLAREBI_USEINLINEFUN2(BIdictionaryGet,dictionaryGetInline)


OZ_Return dictionaryCondGetInline(OZ_Term d, OZ_Term k, OZ_Term deflt, OZ_Term &out)
{
  GetDictAndKey(d,k,dict,key,NO);
  out = dict->getArg(key);
  if (!out)
    out = deflt;
  return (PROCEED);
}
OZ_DECLAREBI_USEINLINEFUN3(BIdictionaryCondGet,dictionaryCondGetInline)

OZ_Return dictionaryPutInline(OZ_Term d, OZ_Term k, OZ_Term value)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->setArg(key,value);
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEREL3(BIdictionaryPut,dictionaryPutInline)

OZ_Return dictionaryExchangeInline(OZ_Term d, OZ_Term k, OZ_Term value, OZ_Term& old)
{
  GetDictAndKey(d,k,dict,key,OK);
  // do not create entry if it does not already exist
  if (dict->exchange(key,value,old,false)) return PROCEED;
  else return oz_raise(E_SYSTEM,E_KERNEL,"dict",2,d,k);
}

OZ_DECLAREBI_USEINLINEFUN3(BIdictionaryExchange,dictionaryExchangeInline)

OZ_Return dictionaryCondExchangeInline(OZ_Term d, OZ_Term k, OZ_Term deflt, OZ_Term value, OZ_Term& old)
{
  GetDictAndKey(d,k,dict,key,OK);
  // create entry if it does not already exist
  if (! dict->exchange(key,value,old,true)) {
    // if feature was non-existent, return default
    old = deflt;
  }
  return PROCEED;
}

OZ_DECLAREBI_USEINLINEFUN4(BIdictionaryCondExchange,dictionaryCondExchangeInline)

OZ_Return dictionaryRemoveInline(OZ_Term d, OZ_Term k)
{
  GetDictAndKey(d,k,dict,key,OK);
  dict->remove(key);
  return PROCEED;
}
OZ_DECLAREBI_USEINLINEREL2(BIdictionaryRemove,dictionaryRemoveInline)


OZ_BI_define(BIdictionaryRemoveAll,1,0)
{
  oz_declareNonvarIN(0,dict);
  if (!oz_isDictionary(dict)) {
    oz_typeError(0,"Dictionary");
  }

  tagged2Dictionary(dict)->removeAll();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIdictionaryWaitOr,1,1)
{
  oz_declareNonvarIN(0,td);
  if (!oz_isDictionary(td)) {
    oz_typeError(0,"Dictionary");
  }

  OzDictionary * dict = tagged2Dictionary(td);

  OZ_Term arity = oz_deref(dict->keys());

  while (!oz_isNil(arity)) {
    OZ_Term f = oz_deref(oz_head(arity)); 
    OZ_Term v;
    v = dict->getArg(f);

    DEREF(v,vPtr);
    Assert(!oz_isRef(v));
    if (!oz_isVarOrRef(v)) {
      am.emptySuspendVarList();
      OZ_RETURN(f);
    }
    (void) am.addSuspendVarListInline(vPtr);
    arity=oz_deref(oz_tail(arity));
  }
  
  return SUSPEND;
  
} OZ_BI_end

//
OZ_Term registry_get(OZ_Term k)
{
  return (tagged2Dictionary(system_registry)->getArg(k));
}

void registry_put(OZ_Term k,OZ_Term v)
{
  tagged2Dictionary(system_registry)->setArg(k,v);
}
